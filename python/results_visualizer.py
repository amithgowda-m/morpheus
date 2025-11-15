#!/usr/bin/env python3
"""
Dynamic Visualization Suite for Morpheus Benchmark Results
Generates publication-quality plots for execution time, speedup, and cache behavior.
"""

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import seaborn as sns
import numpy as np
from pathlib import Path
from typing import List, Dict, Optional, Tuple
from benchmark_parser import BenchmarkSuite, BenchmarkResult
from speedup_analysis import SpeedupMetrics


class ResultsVisualizer:
    """Generate publication-quality plots from benchmark results"""
    
    def __init__(self, output_dir: str = 'results/plots', dpi: int = 300, style: str = 'seaborn-v0_8'):
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self.dpi = dpi
        
        # Set style
        try:
            plt.style.use(style)
        except:
            plt.style.use('seaborn')
        
        sns.set_palette("husl")
    
    def execution_time_trends(self, suites: Dict[str, BenchmarkSuite],
                             algorithm: str = 'bfs',
                             output_file: Optional[str] = None) -> None:
        """
        Plot execution time trends across graph sizes.
        Shows baseline vs optimized with error bars.
        """
        fig, ax = plt.subplots(figsize=(12, 7))
        
        colors = plt.cm.Set1(np.linspace(0, 1, len(suites)))
        
        for color, (config, suite) in zip(colors, suites.items()):
            algo_results = suite.by_algorithm(algorithm)
            
            if not algo_results:
                continue
            
            # Group by graph size
            graph_sizes = sorted(set(r.graph_vertices for r in algo_results))
            means = []
            stds = []
            
            for size in graph_sizes:
                size_results = [r for r in algo_results if r.graph_vertices == size]
                times = [r.execution_time_ms for r in size_results]
                means.append(np.mean(times))
                stds.append(np.std(times))
            
            ax.errorbar(graph_sizes, means, yerr=stds, marker='o', linewidth=2.5,
                       label=f'{config.upper()}', color=color, markersize=8, capsize=5)
        
        ax.set_xlabel('Graph Vertices', fontsize=13, fontweight='bold')
        ax.set_ylabel('Execution Time (ms)', fontsize=13, fontweight='bold')
        ax.set_title(f'{algorithm.upper()} Execution Time Trends', fontsize=14, fontweight='bold')
        ax.set_xscale('log')
        ax.set_yscale('log')
        ax.grid(True, alpha=0.3)
        ax.legend(fontsize=11, loc='best')
        
        plt.tight_layout()
        
        output_path = output_file or self.output_dir / f'{algorithm}_time_trends.pdf'
        plt.savefig(output_path, dpi=self.dpi, format='pdf', bbox_inches='tight')
        print(f"Saved: {output_path}")
        plt.close()
    
    def speedup_comparison(self, speedup_metrics: List[SpeedupMetrics],
                          output_file: Optional[str] = None) -> None:
        """
        Bar plot comparing speedups across algorithms with 95% CI shading.
        """
        fig, ax = plt.subplots(figsize=(14, 8))
        
        algorithms = [m.algorithm for m in speedup_metrics]
        speedups = [m.speedup_factor for m in speedup_metrics]
        ci_lower = [m.speedup_ci_lower for m in speedup_metrics]
        ci_upper = [m.speedup_ci_upper for m in speedup_metrics]
        
        # Error bars (CI)
        errors = [np.array(speedups) - np.array(ci_lower),
                 np.array(ci_upper) - np.array(speedups)]
        
        colors = ['#2ecc71' if m.p_value < 0.05 else '#95a5a6' for m in speedup_metrics]
        
        bars = ax.bar(range(len(algorithms)), speedups, yerr=errors,
                     color=colors, alpha=0.8, capsize=10, edgecolor='black', linewidth=1.5)
        
        # Add 1.0x reference line
        ax.axhline(y=1.0, color='red', linestyle='--', linewidth=2, label='Baseline (1.0x)', alpha=0.7)
        
        # Add significance markers
        for i, m in enumerate(speedup_metrics):
            sig_marker = "***" if m.p_value < 0.001 else "**" if m.p_value < 0.01 else "*" if m.p_value < 0.05 else ""
            if sig_marker:
                ax.text(i, speedups[i] + 0.1, sig_marker, ha='center', fontsize=14, fontweight='bold')
        
        ax.set_xticks(range(len(algorithms)))
        ax.set_xticklabels(algorithms, fontsize=12)
        ax.set_ylabel('Speedup Factor (×)', fontsize=13, fontweight='bold')
        ax.set_title('Morpheus Speedup with ML-Based Adaptive Prefetching', fontsize=14, fontweight='bold')
        ax.set_ylim(0.9, max(speedups) * 1.15)
        ax.grid(axis='y', alpha=0.3)
        
        plt.tight_layout()
        
        output_path = output_file or self.output_dir / 'speedup_comparison.pdf'
        plt.savefig(output_path, dpi=self.dpi, format='pdf', bbox_inches='tight')
        print(f"Saved: {output_path}")
        plt.close()
    
    def cache_behavior_heatmap(self, results: Dict[str, List[Tuple]],
                              output_file: Optional[str] = None) -> None:
        """
        Heatmap showing cache miss rates/behavior across algorithms and configurations.
        
        results: Dict mapping 'algorithm' -> List of (graph_size, miss_rate, config)
        """
        fig, ax = plt.subplots(figsize=(10, 8))
        
        # Create matrix data
        algorithms = list(results.keys())
        configs = set()
        for algo_data in results.values():
            for _, _, config in algo_data:
                configs.add(config)
        
        configs = sorted(list(configs))
        data_matrix = np.zeros((len(algorithms), len(configs)))
        
        for i, algo in enumerate(algorithms):
            for j, config in enumerate(configs):
                algo_results = results[algo]
                matching = [mr for gs, mr, c in algo_results if c == config]
                data_matrix[i, j] = np.mean(matching) if matching else 0
        
        sns.heatmap(data_matrix, annot=True, fmt='.2%', cmap='RdYlGn_r',
                   xticklabels=configs, yticklabels=algorithms,
                   ax=ax, cbar_kws={'label': 'Cache Miss Rate'})
        
        ax.set_title('Cache Miss Rate: Baseline vs ML-Optimized', fontsize=14, fontweight='bold')
        ax.set_xlabel('Configuration', fontsize=12, fontweight='bold')
        ax.set_ylabel('Algorithm', fontsize=12, fontweight='bold')
        
        plt.tight_layout()
        
        output_path = output_file or self.output_dir / 'cache_behavior_heatmap.pdf'
        plt.savefig(output_path, dpi=self.dpi, format='pdf', bbox_inches='tight')
        print(f"Saved: {output_path}")
        plt.close()
    
    def phase_classification_distribution(self, results: List[BenchmarkResult],
                                         output_file: Optional[str] = None) -> None:
        """
        Pie/bar chart showing distribution of detected execution phases.
        """
        if not any(r.final_phase is not None for r in results):
            print("No phase data available")
            return
        
        phases = {}
        phase_names = {0: 'DenseSequential', 1: 'SparseRandom', 2: 'PointerChasing', 3: 'Unknown'}
        
        for r in results:
            if r.final_phase is not None:
                phase_name = phase_names.get(r.final_phase, f'Phase{r.final_phase}')
                phases[phase_name] = phases.get(phase_name, 0) + 1
        
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
        
        # Pie chart
        colors = ['#2ecc71', '#e74c3c', '#3498db', '#95a5a6']
        ax1.pie(phases.values(), labels=phases.keys(), autopct='%1.1f%%',
               colors=colors[:len(phases)], startangle=90)
        ax1.set_title('Detected Execution Phases Distribution', fontsize=12, fontweight='bold')
        
        # Bar chart
        ax2.bar(phases.keys(), phases.values(), color=colors[:len(phases)], edgecolor='black', linewidth=1.5)
        ax2.set_ylabel('Frequency', fontsize=12, fontweight='bold')
        ax2.set_title('Phase Detection Frequency', fontsize=12, fontweight='bold')
        ax2.tick_params(axis='x', rotation=45)
        ax2.grid(axis='y', alpha=0.3)
        
        plt.tight_layout()
        
        output_path = output_file or self.output_dir / 'phase_distribution.pdf'
        plt.savefig(output_path, dpi=self.dpi, format='pdf', bbox_inches='tight')
        print(f"Saved: {output_path}")
        plt.close()
    
    def throughput_comparison(self, suites: Dict[str, BenchmarkSuite],
                             algorithm: str = 'bfs',
                             output_file: Optional[str] = None) -> None:
        """
        Throughput (ops/sec) comparison across configurations.
        """
        fig, ax = plt.subplots(figsize=(12, 7))
        
        colors = plt.cm.Set2(np.linspace(0, 1, len(suites)))
        
        for color, (config, suite) in zip(colors, suites.items()):
            algo_results = suite.by_algorithm(algorithm)
            
            if not algo_results:
                continue
            
            graph_sizes = sorted(set(r.graph_vertices for r in algo_results))
            throughputs = []
            stds = []
            
            for size in graph_sizes:
                size_results = [r for r in algo_results if r.graph_vertices == size]
                tputs = [r.throughput_ops_per_sec for r in size_results]
                throughputs.append(np.mean(tputs))
                stds.append(np.std(tputs))
            
            ax.errorbar(graph_sizes, throughputs, yerr=stds, marker='s', linewidth=2.5,
                       label=f'{config.upper()}', color=color, markersize=8, capsize=5)
        
        ax.set_xlabel('Graph Vertices', fontsize=13, fontweight='bold')
        ax.set_ylabel('Throughput (operations/sec)', fontsize=13, fontweight='bold')
        ax.set_title(f'{algorithm.upper()} Throughput Comparison', fontsize=14, fontweight='bold')
        ax.set_xscale('log')
        ax.set_yscale('log')
        ax.grid(True, alpha=0.3)
        ax.legend(fontsize=11, loc='best')
        
        plt.tight_layout()
        
        output_path = output_file or self.output_dir / f'{algorithm}_throughput.pdf'
        plt.savefig(output_path, dpi=self.dpi, format='pdf', bbox_inches='tight')
        print(f"Saved: {output_path}")
        plt.close()


# Example usage
if __name__ == "__main__":
    from benchmark_parser import BenchmarkParser
    
    parser = BenchmarkParser(results_dir='results')
    suites = parser.load_from_directory()
    
    if suites:
        viz = ResultsVisualizer(output_dir='results/plots')
        
        # Generate plots
        for algo in ['bfs', 'pagerank']:
            viz.execution_time_trends(suites, algorithm=algo)
            viz.throughput_comparison(suites, algorithm=algo)
        
        print("Plots generated successfully!")
    else:
        print("No results found to visualize")
