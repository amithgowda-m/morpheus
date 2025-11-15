# Morpheus Benchmark Analysis Toolkit

Complete analysis pipeline for Morpheus adaptive prefetching benchmark results. Automatically extracts metrics, computes statistical speedup analysis, generates publication-quality visualizations, and creates an interactive dashboard.

## Components

### 1. **BenchmarkParser** (`benchmark_parser.py`)
Parses JSON benchmark files and organizes results into queryable data structures.

**Key Classes:**
- `BenchmarkResult`: Dataclass representing a single benchmark run
  - Fields: algorithm, graph_size, time_ms, phase, num_iterations, convergence_value, name
  - Methods: __post_init__ (converts phase strings to enums)

- `BenchmarkSuite`: Collection of benchmark results with filtering and statistics
  - Methods:
    - `filter_by_algorithm(algo)`: Get results for specific algorithm
    - `filter_by_phase(phase)`: Get results for execution phase
    - `summary_stats()`: Compute mean/median/std across results
    - `count_by_algorithm()`: Get count per algorithm

- `BenchmarkParser`: Main parser interface
  - Methods:
    - `load_from_directory(path)`: Recursively load all *.json files
    - `load_from_files(files)`: Load specific JSON files
    - `parse_benchmark_result(dict)`: Convert dict to BenchmarkResult
    - `generate_summary_report()`: Text report of loaded results
    - `export_csv(output_file)`: Export as CSV for external tools

**Usage:**
```python
from benchmark_parser import BenchmarkParser

parser = BenchmarkParser()
suite = parser.load_from_directory('results/')
print(f"Loaded {suite.count} benchmarks")

# Filter and analyze
bfs_results = suite.filter_by_algorithm('BFS')
stats = bfs_results.summary_stats()
print(f"BFS: {stats['mean']:.2f}ms ± {stats['std']:.2f}ms")

# Export
parser.export_csv('results/all_benchmarks.csv')
```

### 2. **SpeedupAnalyzer** (`speedup_analysis.py`)
Computes statistical speedup metrics with confidence intervals, significance tests, and effect sizes.

**Key Classes:**
- `SpeedupMetrics`: Dataclass storing speedup statistics
  - Fields: speedup, ci_lower, ci_upper, p_value, cohens_d, baseline_mean, optimized_mean
  - Methods: is_significant (p < 0.05)

- `SpeedupAnalyzer`: Static methods for speedup computation
  - Methods:
    - `compute_speedup(baseline_times, optimized_times)`: Compute speedup with CI, t-test, effect size
    - `_bootstrap_ci(data, percentiles=[2.5, 97.5])`: Bootstrap confidence intervals (10k resampling)
    - `_cohens_d(group1, group2)`: Cohen's d effect size (pooled std)
    - `speedup_summary(metrics)`: Text summary of speedup results
    - `generate_acm_table(metrics)`: LaTeX-ready table for publication

**Statistical Methods:**
- **Speedup**: baseline_mean / optimized_mean
- **Confidence Interval**: Bootstrap percentile method (10,000 resamples)
- **Significance Test**: Welch's t-test (unequal variances)
- **Effect Size**: Cohen's d = (μ₁ - μ₂) / σ_pooled

**Usage:**
```python
from speedup_analysis import SpeedupAnalyzer

baseline_times = [10.2, 10.5, 10.1, 9.8]
optimized_times = [8.1, 8.3, 8.0, 7.9]

metrics = SpeedupAnalyzer.compute_speedup(baseline_times, optimized_times)
print(f"Speedup: {metrics.speedup:.2f}×")
print(f"95% CI: [{metrics.ci_lower:.2f}, {metrics.ci_upper:.2f}]")
print(f"p-value: {metrics.p_value:.2e}")
print(f"Cohen's d: {metrics.cohens_d:.3f}")

# LaTeX table
acm_table = SpeedupAnalyzer.generate_acm_table({'BFS': metrics})
```

### 3. **ResultsVisualizer** (`results_visualizer.py`)
Generates publication-quality matplotlib plots with error bars, significance markers, and ACM formatting.

**Key Class:**
- `ResultsVisualizer`: Matplotlib wrapper with 5 plot types
  - Constructor: `ResultsVisualizer(output_dir='results/plots', dpi=300, style='seaborn')`
  - Methods:
    - `execution_time_trends(suite, output_file)`: Log-log plot of time vs graph size
    - `speedup_comparison(metrics, output_file)`: Bar chart with error bars and significance
    - `cache_behavior_heatmap(suite, output_file)`: L1/L2/L3 miss rate heatmap
    - `phase_classification_distribution(suite, output_file)`: Pie + bar chart of phase distribution
    - `throughput_comparison(suite, output_file)`: Operations/second comparison

**Plot Features:**
- DPI=300 for publication
- Error bars: standard deviation or 95% CI
- Significance markers: \*\*\* (p<0.001), \*\* (p<0.01), \* (p<0.05)
- Grid lines for readability
- Color scheme: blue/green/orange (colorblind-friendly)
- Legend positioning optimized per plot type

**Usage:**
```python
from results_visualizer import ResultsVisualizer

viz = ResultsVisualizer(output_dir='results/plots')

# Single plots
viz.execution_time_trends(suite, 'time_trends.png')
viz.speedup_comparison(speedup_metrics, 'speedup.png')
viz.cache_behavior_heatmap(suite, 'cache.png')
viz.phase_classification_distribution(suite, 'phases.png')
viz.throughput_comparison(suite, 'throughput.png')
```

### 4. **DashboardGenerator** (`dashboard_generator.py`)
Creates interactive HTML dashboard with real-time updates and live charts.

**Key Classes:**
- `DashboardGenerator`: Main dashboard interface
  - Constructor: `DashboardGenerator(output_dir='results/dashboard')`
  - Methods:
    - `generate_html_dashboard(metrics_file, output_file)`: Create index.html with embedded Chart.js
    - `generate_json_metrics(benchmark_results, output_file)`: Export metrics as JSON

- `DashboardServer`: Simple HTTP server for local viewing (optional)
  - Methods: `start()`: Serve dashboard on localhost:8000

**Dashboard Features:**
- Real-time update indicator (pulsing green dot)
- 4 metric cards: Average Speedup, Benchmarks Run, Significant Improvements, ML Accuracy
- 4 interactive Chart.js charts:
  - Execution Time Trends (line chart)
  - Speedup by Algorithm (bar chart)
  - Phase Distribution (doughnut chart)
  - Throughput Comparison (line chart)
- Results table with sortable columns
- Auto-refresh every 5 seconds
- Responsive design (mobile-friendly)
- ACM SIGMOD branding

**Usage:**
```python
from dashboard_generator import DashboardGenerator

dashboard = DashboardGenerator()

# Generate metrics JSON
metrics_data = [
    {'algorithm': 'BFS', 'time_ms': 10.2, 'speedup': 1.26},
    {'algorithm': 'PageRank', 'time_ms': 45.3, 'speedup': 1.41},
]
dashboard.generate_json_metrics(metrics_data)

# Generate HTML dashboard
dashboard.generate_html_dashboard(Path('results/metrics.json'))

# Open in browser: file:///results/dashboard/index.html
```

### 5. **BenchmarkAnalysisPipeline** (`benchmark_analysis_main.py`)
Orchestration script that ties all components together into a single analysis workflow.

**Key Class:**
- `BenchmarkAnalysisPipeline`: Main orchestrator
  - Constructor: `BenchmarkAnalysisPipeline(results_dir, output_dir, plots_dir)`
  - Methods:
    - `run_full_analysis()`: Execute 5-step analysis pipeline
      1. Parse all benchmark JSON files
      2. Compute speedup statistics
      3. Generate visualizations (5 plot types)
      4. Create interactive dashboard
      5. Output ACM publication summary

**Usage:**
```bash
# Run complete analysis
python benchmark_analysis_main.py \
    --results-dir results/ \
    --output-dir results/analysis/ \
    --plots-dir results/plots/

# With custom patterns
python benchmark_analysis_main.py \
    --baseline-pattern "*baseline*" \
    --optimized-pattern "*ml-optimized*"
```

**Output Structure:**
```
results/
├── analysis/
│   ├── dashboard/
│   │   ├── index.html          # Interactive dashboard
│   │   └── metrics.json        # Real-time metrics
│   ├── acm_summary.txt         # Publication-ready summary
│   └── ...
├── plots/
│   ├── execution_time_trends.png
│   ├── speedup_comparison.png
│   ├── cache_behavior.png
│   ├── phase_distribution.png
│   └── throughput_comparison.png
└── *.json                       # Raw benchmark results
```

## Installation

The analysis toolkit requires the following Python packages (already installed in venv):

```bash
pip install numpy scipy pandas scikit-learn matplotlib seaborn
```

Verify installation:
```bash
python -c "from benchmark_parser import BenchmarkParser; print('✓ Ready')"
```

## Quick Start

### 1. Generate Sample Data (for testing)
```python
import json
from pathlib import Path

# Create sample benchmark results
Path('results').mkdir(exist_ok=True)

sample_benchmarks = [
    {
        'algorithm': 'BFS',
        'graph_size': 100000,
        'time_ms': 10.2,
        'phase': 'DenseSequential',
        'num_iterations': 100,
        'convergence_value': 0.95,
        'name': 'bfs_100k_baseline'
    },
    {
        'algorithm': 'BFS',
        'graph_size': 100000,
        'time_ms': 8.1,
        'phase': 'DenseSequential',
        'num_iterations': 100,
        'convergence_value': 0.95,
        'name': 'bfs_100k_ml_optimized'
    },
]

with open('results/sample_benchmarks.json', 'w') as f:
    json.dump(sample_benchmarks, f)
```

### 2. Run Full Analysis Pipeline
```bash
cd python
python benchmark_analysis_main.py --results-dir ../results/
```

### 3. View Results
- **Dashboard**: Open `results/analysis/dashboard/index.html` in browser
- **Plots**: View PNG files in `results/plots/`
- **Summary**: Read `results/analysis/acm_summary.txt`

## Advanced Usage

### Custom Analysis Script
```python
from benchmark_parser import BenchmarkParser
from speedup_analysis import SpeedupAnalyzer
from results_visualizer import ResultsVisualizer

# Parse
parser = BenchmarkParser()
suite = parser.load_from_directory('custom_results/')

# Analyze specific algorithm
bfs_results = suite.filter_by_algorithm('BFS')
baseline = [r.time_ms for r in bfs_results if 'baseline' in r.name]
optimized = [r.time_ms for r in bfs_results if 'optimized' in r.name]

# Compute speedup
metrics = SpeedupAnalyzer.compute_speedup(baseline, optimized)
print(f"BFS: {metrics.speedup:.2f}× (p={metrics.p_value:.2e})")

# Visualize
viz = ResultsVisualizer()
viz.speedup_comparison({'BFS': metrics}, 'my_speedup.png')
```

### Generate LaTeX Table
```python
from speedup_analysis import SpeedupAnalyzer

metrics = {...}  # computed speedup metrics
latex_table = SpeedupAnalyzer.generate_acm_table(metrics)
print(latex_table)

# Paste into paper:
# \begin{table}
#   \centering
#   {latex_table}
#   \caption{Speedup with 95\% CI}
# \end{table}
```

## API Reference

### BenchmarkResult
```python
@dataclass
class BenchmarkResult:
    algorithm: str
    graph_size: int
    time_ms: float
    phase: str
    num_iterations: int
    convergence_value: float
    name: str
```

### SpeedupMetrics
```python
@dataclass
class SpeedupMetrics:
    speedup: float
    ci_lower: float
    ci_upper: float
    p_value: float
    cohens_d: float
    baseline_mean: float
    optimized_mean: float
    
    def is_significant(self) -> bool:
        return self.p_value < 0.05
```

### Key Constants
- Bootstrap iterations: 10,000
- Confidence level: 95% (α=0.05)
- Plot DPI: 300 (publication quality)
- Chart.js version: 3.9.1

## Troubleshooting

**"No benchmark files found"**
- Ensure JSON files exist in `results/` directory
- Check file format: `{"algorithm": "BFS", "time_ms": 10.2, ...}`

**"Import error: No module named 'numpy'"**
- Activate venv: `source python/.venv/bin/activate`
- Install: `pip install numpy scipy pandas scikit-learn matplotlib seaborn`

**"Permission denied: /tmp/..."**
- Dashboard may need write access to temp directory
- Use `--output-dir ./local_results/` to write to current directory

**"Module not found: benchmark_parser"**
- Ensure working directory is `/morpheus/python/`
- Or add to PYTHONPATH: `export PYTHONPATH=$PYTHONPATH:/home/user/morpheus/python`

## Citation

If using this analysis toolkit in research, cite as:

```bibtex
@misc{morpheus-analysis,
  title={Morpheus Benchmark Analysis Toolkit},
  author={Morpheus Project},
  year={2024},
  url={https://github.com/morpheus/analysis}
}
```

## Output Interpretation

### Speedup Factor
- **1.0×**: No improvement
- **1.2-1.5×**: Modest improvement (typical for ML-based optimization)
- **>2.0×**: Significant improvement (investigate methodology)

### Cohen's d (Effect Size)
- **0.2**: Small effect
- **0.5**: Medium effect
- **0.8**: Large effect

### p-value (Statistical Significance)
- **p < 0.001** (\*\*\*): Highly significant
- **p < 0.01** (\*\*): Very significant
- **p < 0.05** (\*): Significant
- **p ≥ 0.05** (ns): Not significant

## License

See parent project LICENSE file.
