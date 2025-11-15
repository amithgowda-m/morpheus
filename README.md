# Morpheus: Adaptive Memory Prefetching for Graph Algorithms

Morpheus is a dynamic instrumentation framework that adaptively prefetches memory for irregular graph algorithms like BFS, PageRank, and Betweenness Centrality.

## Quick Start

```bash
# Build the project
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run benchmarks
./benchmarks/benchmark-runner --graph ../data/sample.csr --algorithm bfs

# Train phase classifier
cd ../python
pip install -r requirements.txt
python phase_trainer.py

# Run with instrumentation (requires Intel PIN)
./scripts/run_benchmarks.sh --with-pin