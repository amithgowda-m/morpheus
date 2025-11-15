#!/usr/bin/env python3
"""
MORPHEUS BENCHMARK ANALYSIS TOOLKIT - QUICKSTART GUIDE

Complete end-to-end example showing how to:
1. Run benchmarks and capture results
2. Parse benchmark JSON files
3. Compute statistical speedup metrics
4. Generate publication-quality visualizations
5. Create interactive dashboard
6. Export ACM-ready summary tables
"""

# ============================================================================
# STEP 1: RUN BENCHMARKS AND GENERATE JSON RESULTS
# ============================================================================
"""
First, run the Morpheus benchmarks and save results as JSON:

    cd /home/amithgowda/morpheus
    
    # Build the project (if not already built)
    mkdir -p build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j8
    
    # Run benchmarks and save results
    ./benchmark-runner > ../results/bfs_baseline.json
    ./benchmark-runner --with-ml-optimization > ../results/bfs_optimized.json
    
    # Repeat for different algorithms and configurations
    # Results should be in ../results/*.json directory

The JSON format should include:
{
    "algorithm": "BFS",
    "iterations": 100,
    "graph_vertices": 1000000,
    "graph_edges": 5000000,
    "min_time_ns": 1000000000,
    "max_time_ns": 1200000000,
    "avg_time_ns": 1100000000,
    "execution_time_ms": 1100.0,
    "timestamp": 1234567890,
    "performance_samples": 100,
    "final_phase": 0,
    "avg_convergence_iterations": 50
}
"""

# ============================================================================
# STEP 2: PARSE BENCHMARK RESULTS
# ============================================================================

from pathlib import Path
from benchmark_parser import BenchmarkParser

# Initialize parser
parser = BenchmarkParser(results_dir='results')

# Load all JSON files from results directory
suites = parser.load_from_directory('results')

print(f"\nLoaded {len(suites)} configuration(s)")
for config_name, suite in suites.items():
    print(f"  {config_name}: {len(suite.results)} results")
    print(f"    Algorithms: {set(r.algorithm for r in suite.results)}")
    stats = suite.summary_stats()
    print(f"    Mean time: {stats['mean_time_ms']:.2f}ms")


# ============================================================================
# STEP 3: COMPUTE SPEEDUP STATISTICS
# ============================================================================

from speedup_analysis import SpeedupAnalyzer, generate_acm_table

# Get a suite with both baseline and optimized results
baseline_suite = suites.get('baseline')
optimized_suite = suites.get('optimized')

if baseline_suite and optimized_suite:
    speedup_metrics = {}
    
    # Compute speedup for each algorithm
    for algo in set(r.algorithm for r in baseline_suite.results):
        baseline_results = baseline_suite.by_algorithm(algo)
        optimized_results = optimized_suite.by_algorithm(algo)
        
        baseline_times = [r.execution_time_ms for r in baseline_results]
        optimized_times = [r.execution_time_ms for r in optimized_results]
        
        if baseline_times and optimized_times:
            metrics = SpeedupAnalyzer.compute_speedup(baseline_times, optimized_times)
            metrics.algorithm = algo
            speedup_metrics[algo] = metrics
            
            # Print results
            print(f"\n{algo} Speedup Analysis:")
            print(f"  Baseline:  {metrics.baseline_mean:.2f}ms ± {metrics.baseline_std:.2f}ms")
            print(f"  Optimized: {metrics.optimized_mean:.2f}ms ± {metrics.optimized_std:.2f}ms")
            print(f"  Speedup:   {metrics.speedup_factor:.2f}× ")
            print(f"  95% CI:    [{metrics.speedup_ci_lower:.2f}, {metrics.speedup_ci_upper:.2f}]")
            print(f"  p-value:   {metrics.p_value:.2e} ", end="")
            
            if metrics.p_value < 0.001:
                print("***")
            elif metrics.p_value < 0.01:
                print("**")
            elif metrics.p_value < 0.05:
                print("*")
            else:
                print("(ns)")
            
            print(f"  Cohen's d: {metrics.cohens_d:.3f}")


# ============================================================================
# STEP 4: GENERATE VISUALIZATIONS
# ============================================================================

from results_visualizer import ResultsVisualizer

# Create visualizer with output directory
viz = ResultsVisualizer(output_dir='results/plots', dpi=300)

# Generate execution time trends (log-log plot)
if baseline_suite:
    viz.execution_time_trends(baseline_suite, 'results/plots/execution_time_trends.png')
    print("\n✓ Generated: execution_time_trends.png")

# Generate speedup comparison chart with error bars
if speedup_metrics:
    viz.speedup_comparison(speedup_metrics, 'results/plots/speedup_comparison.png')
    print("✓ Generated: speedup_comparison.png")

# Generate cache behavior heatmap
if baseline_suite:
    viz.cache_behavior_heatmap(baseline_suite, 'results/plots/cache_behavior.png')
    print("✓ Generated: cache_behavior.png")

# Generate phase classification distribution
if baseline_suite:
    viz.phase_classification_distribution(baseline_suite, 'results/plots/phase_distribution.png')
    print("✓ Generated: phase_distribution.png")

# Generate throughput comparison
if baseline_suite:
    viz.throughput_comparison(baseline_suite, 'results/plots/throughput_comparison.png')
    print("✓ Generated: throughput_comparison.png")

print("\nAll plots saved to results/plots/")


# ============================================================================
# STEP 5: CREATE INTERACTIVE DASHBOARD
# ============================================================================

from dashboard_generator import DashboardGenerator
import json

dashboard = DashboardGenerator(output_dir='results/dashboard')

# Prepare metrics data for dashboard
metrics_data = []
if baseline_suite:
    for result in baseline_suite.results:
        metrics_data.append({
            'algorithm': result.algorithm,
            'graph_size': result.graph_vertices,
            'time_ms': result.execution_time_ms,
            'phase': result.final_phase if result.final_phase else 0,
            'iterations': result.iterations
        })

# Generate dashboard
if metrics_data:
    metrics_file = Path('results/dashboard/metrics.json')
    dashboard.generate_json_metrics(metrics_data, metrics_file)
    dashboard.generate_html_dashboard(metrics_file)
    print("\n✓ Dashboard created: results/dashboard/index.html")
    print("  Open in browser: file://$(pwd)/results/dashboard/index.html")


# ============================================================================
# STEP 6: EXPORT ACM PUBLICATION SUMMARY
# ============================================================================

if speedup_metrics:
    acm_table = generate_acm_table(list(speedup_metrics.values()))
    
    # Save to file
    summary_file = Path('results/acm_summary.txt')
    summary_file.write_text(acm_table)
    
    print("\n" + "="*70)
    print("ACM PUBLICATION TABLE")
    print("="*70)
    print(acm_table)
    print("="*70)
    print(f"\nTable saved to: {summary_file}")


# ============================================================================
# STEP 7: RUN COMPLETE ANALYSIS PIPELINE (AUTOMATED)
# ============================================================================

"""
Instead of running each step manually, use the complete pipeline:

    python benchmark_analysis_main.py \\
        --results-dir results/ \\
        --output-dir results/analysis/ \\
        --plots-dir results/plots/

This will:
1. Parse all JSON benchmarks
2. Compute speedup statistics
3. Generate all visualizations
4. Create interactive dashboard
5. Output ACM summary

The pipeline handles all steps automatically and provides a
complete analysis report in the output directory.
"""


# ============================================================================
# STEP 8: CUSTOM ANALYSIS SCRIPT
# ============================================================================

"""
For custom analysis beyond the standard pipeline, write your own script:

    from benchmark_parser import BenchmarkParser
    from speedup_analysis import SpeedupAnalyzer
    from results_visualizer import ResultsVisualizer
    
    # Load and analyze specific benchmarks
    parser = BenchmarkParser()
    suites = parser.load_from_directory('results')
    
    # Filter by graph size
    large_graphs = []
    for suite in suites.values():
        for result in suite.results:
            if result.graph_vertices >= 1000000:
                large_graphs.append(result)
    
    # Analyze specific algorithms
    bfs_results = [r for r in large_graphs if r.algorithm == 'BFS']
    
    # Generate custom plots
    viz = ResultsVisualizer()
    # ... create your visualizations
"""


# ============================================================================
# DIRECTORY STRUCTURE AFTER ANALYSIS
# ============================================================================

"""
results/
├── *.json                          # Raw benchmark results
├── export.csv                      # CSV export of all results
├── analysis/
│   ├── dashboard/
│   │   ├── index.html             # Interactive dashboard
│   │   ├── metrics.json           # Real-time metrics
│   │   └── style.css              # Dashboard styles
│   ├── acm_summary.txt            # Publication-ready summary
│   └── analysis_report.txt        # Detailed analysis report
├── plots/
│   ├── execution_time_trends.png  # Log-log time vs graph size
│   ├── speedup_comparison.png     # Speedup bars with error bars
│   ├── cache_behavior.png         # L1/L2/L3 miss rates heatmap
│   ├── phase_distribution.png     # Execution phase pie/bar chart
│   └── throughput_comparison.png  # Operations/second comparison
└── README.md                       # Analysis results summary
"""


# ============================================================================
# KEY METRICS & INTERPRETATION
# ============================================================================

"""
SPEEDUP INTERPRETATION:
  1.0×  = No improvement
  1.2×  = 20% speedup (modest)
  1.5×  = 50% speedup (good)
  2.0×+ = 2× or more (excellent)

STATISTICAL SIGNIFICANCE:
  *** = p < 0.001 (highly significant)
  **  = p < 0.01  (very significant)
  *   = p < 0.05  (significant)
  ns  = p ≥ 0.05  (not significant)

EFFECT SIZE (Cohen's d):
  0.2  = small effect
  0.5  = medium effect
  0.8  = large effect
  >1.0 = very large effect

CONFIDENCE INTERVALS:
  [a, b] where a is lower bound, b is upper bound
  Narrower CI = more precise estimate
  CI including 1.0 = result may not be significant
"""


# ============================================================================
# TROUBLESHOOTING
# ============================================================================

"""
Q: "No benchmark files found"
A: Ensure JSON files exist in results/ directory with correct format

Q: "Module import errors"
A: Activate venv: source /path/to/.venv/bin/activate
   Then: pip install -r requirements.txt

Q: "Plots not generated"
A: Install matplotlib/seaborn: pip install matplotlib seaborn

Q: "Dashboard not interactive"
A: Open with file:// URL in browser
   May need local HTTP server for auto-refresh:
   python -m http.server 8000 -d results/dashboard

Q: "ACM table has wrong values"
A: Check that benchmark_times are in milliseconds (float)
   p_values and CI are automatically computed
"""


if __name__ == '__main__':
    print(__doc__)
