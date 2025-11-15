#!/usr/bin/env python3

import json
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import argparse
import glob
import os
from datetime import datetime

class ResultsPlotter:
    def __init__(self):
        self.colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd']
        self.set_style()
    
    def set_style(self):
        """Set matplotlib style"""
        plt.style.use('seaborn-v0_8-whitegrid')
        sns.set_palette(self.colors)
    
    def load_results(self, result_dir):
        """Load all result files from directory"""
        results = []
        pattern = os.path.join(result_dir, "*.json")
        
        for file_path in glob.glob(pattern):
            try:
                with open(file_path, 'r') as f:
                    data = json.load(f)
                    data['file'] = os.path.basename(file_path)
                    results.append(data)
            except (json.JSONDecodeError, KeyError) as e:
                print(f"Warning: Skipping invalid file {file_path}: {e}")
        
        return pd.DataFrame(results)
    
    def plot_execution_times(self, df, output_file=None):
        """Plot execution times by algorithm"""
        if df.empty:
            print("No data to plot")
            return
        
        plt.figure(figsize=(10, 6))
        
        # Group by algorithm
        algorithms = df['algorithm'].unique()
        
        data_to_plot = []
        labels = []
        
        for alg in algorithms:
            alg_data = df[df['algorithm'] == alg]
            if not alg_data.empty:
                data_to_plot.append(alg_data['execution_time_ms'].values)
                labels.append(alg)
        
        plt.boxplot(data_to_plot, labels=labels)
        plt.ylabel('Execution Time (ms)')
        plt.title('Algorithm Execution Times')
        plt.xticks(rotation=45)
        plt.tight_layout()
        
        if output_file:
            plt.savefig(output_file, dpi=300, bbox_inches='tight')
            print(f"Execution times plot saved to: {output_file}")
        else:
            plt.show()
    
    def plot_cache_performance(self, df, output_file=None):
        """Plot cache performance metrics"""
        if df.empty:
            return
        
        fig, axes = plt.subplots(2, 2, figsize=(12, 10))
        axes = axes.flatten()
        
        metrics = [
            ('l3_cache_misses', 'L3 Cache Misses'),
            ('l2_cache_misses', 'L2 Cache Misses'), 
            ('l1_cache_misses', 'L1 Cache Misses'),
            ('instructions', 'Instructions')
        ]
        
        algorithms = df['algorithm'].unique()
        
        for idx, (metric, title) in enumerate(metrics):
            if idx >= len(axes):
                break
                
            ax = axes[idx]
            data_to_plot = []
            
            for alg in algorithms:
                alg_data = df[df['algorithm'] == alg]
                if not alg_data.empty and metric in alg_data.columns:
                    data_to_plot.append(alg_data[metric].values)
            
            if data_to_plot:
                ax.boxplot(data_to_plot, labels=algorithms)
                ax.set_title(title)
                ax.tick_params(axis='x', rotation=45)
        
        plt.tight_layout()
        
        if output_file:
            plt.savefig(output_file, dpi=300, bbox_inches='tight')
            print(f"Cache performance plot saved to: {output_file}")
        else:
            plt.show()
    
    def plot_speedup_comparison(self, df, output_file=None):
        """Plot speedup comparison between different configurations"""
        if df.empty:
            return
        
        # This would require data with different prefetching strategies
        if 'strategy' not in df.columns:
            print("No strategy information available for speedup comparison")
            return
        
        plt.figure(figsize=(10, 6))
        
        strategies = df['strategy'].unique()
        algorithms = df['algorithm'].unique()
        
        for alg in algorithms:
            alg_data = df[df['algorithm'] == alg]
            baseline = alg_data[alg_data['strategy'] == 'none']['execution_time_ms'].mean()
            
            speedups = []
            strategy_labels = []
            
            for strategy in strategies:
                if strategy != 'none':
                    strategy_data = alg_data[alg_data['strategy'] == strategy]
                    if not strategy_data.empty:
                        mean_time = strategy_data['execution_time_ms'].mean()
                        speedup = baseline / mean_time if mean_time > 0 else 1.0
                        speedups.append(speedup)
                        strategy_labels.append(strategy)
            
            if speedups:
                plt.bar(strategy_labels, speedups, alpha=0.7, label=alg)
        
        plt.axhline(y=1.0, color='red', linestyle='--', alpha=0.7, label='Baseline')
        plt.ylabel('Speedup (vs. no prefetching)')
        plt.title('Prefetching Strategy Speedup Comparison')
        plt.legend()
        plt.tight_layout()
        
        if output_file:
            plt.savefig(output_file, dpi=300, bbox_inches='tight')
            print(f"Speedup comparison plot saved to: {output_file}")
        else:
            plt.show()
    
    def create_summary_report(self, df, output_file):
        """Create a comprehensive summary report"""
        if df.empty:
            return
        
        summary = {
            'timestamp': datetime.now().isoformat(),
            'total_runs': len(df),
            'algorithms': {},
            'overall_metrics': {}
        }
        
        # Overall metrics
        numeric_columns = df.select_dtypes(include=['number']).columns
        for col in numeric_columns:
            summary['overall_metrics'][col] = {
                'mean': df[col].mean(),
                'median': df[col].median(),
                'std': df[col].std(),
                'min': df[col].min(),
                'max': df[col].max()
            }
        
        # Algorithm-specific metrics
        for algorithm in df['algorithm'].unique():
            alg_data = df[df['algorithm'] == algorithm]
            summary['algorithms'][algorithm] = {
                'count': len(alg_data),
                'execution_time_ms': {
                    'mean': alg_data['execution_time_ms'].mean(),
                    'std': alg_data['execution_time_ms'].std()
                }
            }
            
            # Add cache performance if available
            cache_metrics = ['l1_cache_misses', 'l2_cache_misses', 'l3_cache_misses']
            for metric in cache_metrics:
                if metric in alg_data.columns:
                    summary['algorithms'][algorithm][metric] = {
                        'mean': alg_data[metric].mean(),
                        'std': alg_data[metric].std()
                    }
        
        # Write summary to file
        with open(output_file, 'w') as f:
            json.dump(summary, f, indent=2)
        
        print(f"Summary report saved to: {output_file}")

def main():
    parser = argparse.ArgumentParser(description='Plot benchmark results')
    parser.add_argument('--input', '-i', required=True, help='Input directory with JSON results')
    parser.add_argument('--output-dir', '-o', required=True, help='Output directory for plots')
    parser.add_argument('--format', '-f', default='png', choices=['png', 'pdf', 'svg'], 
                       help='Output format for plots')
    
    args = parser.parse_args()
    
    # Create output directory
    os.makedirs(args.output_dir, exist_ok=True)
    
    # Initialize plotter
    plotter = ResultsPlotter()
    
    # Load results
    df = plotter.load_results(args.input)
    
    if df.empty:
        print("No results found to plot")
        return 1
    
    print(f"Loaded {len(df)} results")
    
    # Generate plots
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    
    plotter.plot_execution_times(
        df, 
        os.path.join(args.output_dir, f'execution_times_{timestamp}.{args.format}')
    )
    
    plotter.plot_cache_performance(
        df,
        os.path.join(args.output_dir, f'cache_performance_{timestamp}.{args.format}')
    )
    
    plotter.plot_speedup_comparison(
        df,
        os.path.join(args.output_dir, f'speedup_comparison_{timestamp}.{args.format}')
    )
    
    # Create summary report
    plotter.create_summary_report(
        df,
        os.path.join(args.output_dir, f'summary_{timestamp}.json')
    )
    
    print("All plots and reports generated successfully")
    return 0

if __name__ == '__main__':
    main()