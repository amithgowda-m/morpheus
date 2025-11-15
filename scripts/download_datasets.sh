#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DATA_DIR="$PROJECT_ROOT/data"
SNAP_BASE="https://snap.stanford.edu/data"
GAP_BASE="https://portal.nersc.gov/project/m1982/GAP"

mkdir -p "$DATA_DIR"

download_file() {
    local url=$1
    local output=$2
    
    if command -v wget &> /dev/null; then
        wget -O "$output" "$url"
    elif command -v curl &> /dev/null; then
        curl -L -o "$output" "$url"
    else
        echo "Error: Neither wget nor curl available"
        return 1
    fi
}

extract_if_needed() {
    local file=$1
    
    case "$file" in
        *.gz)
            if [[ ! -f "${file%.gz}" ]]; then
                echo "Extracting $file..."
                gunzip -c "$file" > "${file%.gz}"
            fi
            ;;
        *.zip)
            if [[ ! -d "${file%.zip}" ]]; then
                echo "Extracting $file..."
                unzip -q -d "${file%.zip}" "$file"
            fi
            ;;
    esac
}

echo "Downloading graph datasets..."

# Small test graph (already included in sample.csr)
echo "Skipping test graph (using included sample.csr)"

# SNAP datasets
SNAP_DATASETS=(
    "web-Google.txt.gz"
    "roadNet-CA.txt.gz"
    "com-Youtube.txt.gz"
)

for dataset in "${SNAP_DATASETS[@]}"; do
    output_file="$DATA_DIR/$dataset"
    
    if [[ ! -f "$output_file" ]] && [[ ! -f "${output_file%.gz}" ]]; then
        echo "Downloading $dataset..."
        download_file "$SNAP_BASE/$dataset" "$output_file"
        extract_if_needed "$output_file"
    else
        echo "Skipping $dataset (already exists)"
    fi
done

# GAP datasets (twitter graph)
TWITTER_FILE="$DATA_DIR/twitter-2010.txt.gz"
if [[ ! -f "$TWITTER_FILE" ]] && [[ ! -f "${TWITTER_FILE%.gz}" ]]; then
    echo "Downloading twitter-2010..."
    download_file "$GAP_BASE/twitter-2010.txt.gz" "$TWITTER_FILE"
    extract_if_needed "$TWITTER_FILE"
else
    echo "Skipping twitter-2010 (already exists)"
fi

echo "Dataset download completed"
echo "Available graphs:"
find "$DATA_DIR" -name "*.txt" -o -name "*.csr" | while read file; do
    if [[ -f "$file" ]]; then
        size=$(du -h "$file" | cut -f1)
        lines=$(wc -l < "$file" 2>/dev/null || echo "N/A")
        echo "  $file ($size, $lines lines)"
    fi
done