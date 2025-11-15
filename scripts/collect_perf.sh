#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_DIR="$PROJECT_ROOT/perf_results"
DURATION=30
INTERVAL=100  # ms

usage() {
    echo "Usage: $0 [OPTIONS] -- command [args...]"
    echo "Collect performance counters for a command"
    echo ""
    echo "OPTIONS:"
    echo "  -o, --output DIR    Output directory (default: $OUTPUT_DIR)"
    echo "  -d, --duration SEC  Duration in seconds (default: $DURATION)"
    echo "  -i, --interval MS   Sampling interval in ms (default: $INTERVAL)"
    echo "  -h, --help          Show this help message"
    exit 1
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -o|--output)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        -d|--duration)
            DURATION="$2"
            shift 2
            ;;
        -i|--interval)
            INTERVAL="$2"
            shift 2
            ;;
        -h|--help)
            usage
            ;;
        --)
            shift
            break
            ;;
        *)
            echo "Unknown option: $1"
            usage
            ;;
    esac
done

if [[ $# -eq 0 ]]; then
    echo "Error: Command is required"
    usage
fi

mkdir -p "$OUTPUT_DIR"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
PERF_DATA="$OUTPUT_DIR/perf_data_$TIMESTAMP.data"
PERF_SCRIPT="$OUTPUT_DIR/perf_script_$TIMESTAMP.txt"

# Create perf script
cat > "$PERF_SCRIPT" << 'EOF'
#!/bin/bash

# Performance events to monitor
EVENTS="
cpu-cycles,
instructions,
cache-references,
cache-misses,
branch-instructions,
branch-misses,
L1-dcache-loads,
L1-dcache-load-misses,
L1-dcache-stores,
llc-loads,
llc-load-misses,
llc-stores,
llc-store-misses
"

# Convert comma-separated events to perf format
PERF_EVENTS=$(echo $EVENTS | tr -d '[:space:]' | tr -d '\n')

# Run perf stat
perf stat -e "$PERF_EVENTS" -o "$1" --timeout $2 $3
EOF

chmod +x "$PERF_SCRIPT"

echo "Collecting performance data for $DURATION seconds..."
echo "Command: $@"

# Run perf collection
"$PERF_SCRIPT" "$PERF_DATA" "$DURATION" "$@"

if [[ $? -eq 0 ]]; then
    echo "Performance data written to: $PERF_DATA"
    
    # Parse and format results
    echo ""
    echo "=== PERFORMANCE SUMMARY ==="
    grep -E "(cpu-cycles|instructions|cache-misses|branch-misses|llc-load-misses)" "$PERF_DATA" | \
        head -10
else
    echo "Error: perf collection failed"
    exit 1
fi