#!/bin/bash

################################################################################
# Real Graph Dataset Download and Conversion Script
# 
# Downloads real-world graph datasets from SNAP and other sources
# Converts them to Morpheus CSR format for benchmarking
################################################################################

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DATA_DIR="${SCRIPT_DIR}/../data"
CONVERTER_DIR="${SCRIPT_DIR}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}"
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║         MORPHEUS REAL GRAPH DATASET DOWNLOADER                ║"
echo "║      Downloads SNAP datasets and converts to CSR format       ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo -e "${NC}\n"

# Create data directory
mkdir -p "$DATA_DIR"
cd "$DATA_DIR"

###############################################################################
# 1. Web Graph (Google web crawl) - ~880K vertices, ~5.1M edges
###############################################################################

echo -e "${YELLOW}[1/5] Downloading web-Google graph...${NC}"

if [ ! -f "web-Google.txt" ]; then
    if [ ! -f "web-Google.txt.gz" ]; then
        echo "Fetching from Stanford SNAP repository..."
        wget -q https://snap.stanford.edu/data/web-Google.txt.gz \
            -O web-Google.txt.gz || {
            echo -e "${RED}Failed to download web-Google${NC}"
            echo "Try manual download from: https://snap.stanford.edu/data/web-Google.html"
        }
    fi
    
    if [ -f "web-Google.txt.gz" ]; then
        echo "Extracting..."
        gunzip -v web-Google.txt.gz
    fi
fi

if [ -f "web-Google.txt" ]; then
    echo -e "${GREEN}✓ web-Google graph ready${NC}"
    wc -l web-Google.txt
else
    echo -e "${YELLOW}⊘ Skipped: web-Google${NC}"
fi

###############################################################################
# 2. LiveJournal Social Network - ~4.8M vertices, ~69M edges
###############################################################################

echo -e "${YELLOW}[2/5] Downloading soc-LiveJournal1 graph...${NC}"

if [ ! -f "soc-LiveJournal1.txt" ]; then
    if [ ! -f "soc-LiveJournal1.txt.gz" ]; then
        echo "Fetching from Stanford SNAP repository..."
        wget -q https://snap.stanford.edu/data/soc-LiveJournal1.txt.gz \
            -O soc-LiveJournal1.txt.gz || {
            echo -e "${RED}Failed to download soc-LiveJournal1${NC}"
            echo "Try manual download from: https://snap.stanford.edu/data/soc-LiveJournal1.html"
        }
    fi
    
    if [ -f "soc-LiveJournal1.txt.gz" ]; then
        echo "Extracting (this may take a minute)..."
        gunzip -v soc-LiveJournal1.txt.gz
    fi
fi

if [ -f "soc-LiveJournal1.txt" ]; then
    echo -e "${GREEN}✓ soc-LiveJournal1 graph ready${NC}"
    wc -l soc-LiveJournal1.txt
else
    echo -e "${YELLOW}⊘ Skipped: soc-LiveJournal1${NC}"
fi

###############################################################################
# 3. Wiki Graph - ~2.4M vertices, ~68M edges
###############################################################################

echo -e "${YELLOW}[3/5] Downloading wiki graph...${NC}"

if [ ! -f "wiki-topcats.txt" ]; then
    if [ ! -f "wiki-topcats.txt.gz" ]; then
        echo "Fetching from Stanford SNAP repository..."
        wget -q https://snap.stanford.edu/data/wiki-topcats.txt.gz \
            -O wiki-topcats.txt.gz || {
            echo -e "${RED}Failed to download wiki graph${NC}"
        }
    fi
    
    if [ -f "wiki-topcats.txt.gz" ]; then
        echo "Extracting..."
        gunzip -v wiki-topcats.txt.gz
    fi
fi

if [ -f "wiki-topcats.txt" ]; then
    echo -e "${GREEN}✓ wiki graph ready${NC}"
    wc -l wiki-topcats.txt
else
    echo -e "${YELLOW}⊘ Skipped: wiki graph${NC}"
fi

###############################################################################
# 4. Twitter Graph - Large, preprocessed
###############################################################################

echo -e "${YELLOW}[4/5] Preparing Twitter graph...${NC}"

if [ ! -f "twitter.txt" ]; then
    echo "Note: Twitter graph is large (~1.5GB). Download separately if needed:"
    echo "  https://snap.stanford.edu/data/twitter7.html"
    echo -e "${YELLOW}⊘ Skipped: Twitter (too large for auto-download)${NC}"
fi

###############################################################################
# 5. Create sample synthetic graphs for testing
###############################################################################

echo -e "${YELLOW}[5/5] Creating synthetic test graphs...${NC}"

# Create small test graph (1000 vertices, random)
python3 << 'PYTHON_SCRIPT'
import random

# Small test graph
with open('test-small.txt', 'w') as f:
    for u in range(100):
        for _ in range(random.randint(2, 10)):
            v = random.randint(0, 99)
            if u != v:
                f.write(f"{u}\t{v}\n")

# Medium test graph
with open('test-medium.txt', 'w') as f:
    for u in range(1000):
        for _ in range(random.randint(2, 5)):
            v = random.randint(0, 999)
            if u != v:
                f.write(f"{u}\t{v}\n")

print("✓ Test graphs created")
PYTHON_SCRIPT

###############################################################################
# Summary
###############################################################################

echo -e "\n${BLUE}"
echo "═══════════════════════════════════════════════════════════════"
echo "DOWNLOAD SUMMARY"
echo "═══════════════════════════════════════════════════════════════"
echo -e "${NC}"

echo -e "\n${GREEN}Downloaded Graphs:${NC}"
for file in *.txt; do
    if [ -f "$file" ]; then
        lines=$(wc -l < "$file")
        size=$(du -h "$file" | cut -f1)
        printf "  • %-30s %8d edges  %6s\n" "$file" "$lines" "$size"
    fi
done

echo -e "\n${YELLOW}Next Steps:${NC}"
echo "1. Convert graphs to CSR format:"
echo "   cd $(dirname "$SCRIPT_DIR")/build"
echo ""
echo "   For each .txt graph, create CSR:"
echo "   ../scripts/convert_snap_to_csr.py ../data/web-Google.txt ../data/web-Google.csr"
echo "   ../scripts/convert_snap_to_csr.py ../data/soc-LiveJournal1.txt ../data/soc-LiveJournal1.csr"
echo ""
echo "2. Run benchmarks:"
echo "   ./benchmark-runner --graph ../data/web-Google.csr --algorithm bfs --iterations 10"
echo ""
echo "3. Analyze results:"
echo "   cd ../python"
echo "   python benchmark_analysis_main.py --results-dir ../results/"
echo ""

echo -e "${GREEN}✓ Graph download phase complete!${NC}\n"
