#!/usr/bin/env python3

import json
import glob
import os
import sys
import statistics
from datetime import datetime
from pathlib import Path

def load_results(result_dir):
    """Load all result files from directory"""
    results = {}
    pattern = os.path.join(result_dir, "*.json")
    
    for file_path in glob.glob(pattern):
        try:
            with open(file_path, 'r') as f:
                data = json.load(f)
                
            algorithm = data.get('algorithm', 'unknown')
            if algorithm not in results:
                results[algorithm] = []
                
            results[algorithm].append(data)
            
        except (json.JSONDecodeError, KeyError) as e:
            print(f"Warning: Skipping invalid file {file_path}: {e}")
            
    return results

def analyze_algorithm(algorithm_results):
    """Analyze results for a specific algorithm"""
    if not algorithm_results:
        return None
        
    execution_times = [r['execution_time_ms'] for r in algorithm_results]
    cache_misses = [r.get('l3_cache_misses', 0) for r in algorithm_results]
    instructions = [r.get('instructions', 0) for r in algorithm_results]
    
    analysis = {
        'algorithm': algorithm_results[0]['algorithm'],
        'sample_count': len(algorithm_results),
        'execution_time': {
            'mean': statistics.mean(execution_times),
            'median': statistics.median(execution_times),
            'stdev': statistics.stdev(execution_times) if len(execution_times) > 1 else 0,
            'min': min(execution_times),
            'max': max(execution_times)
        },
        'cache_efficiency': {
            'avg_misses': statistics.mean(cache_misses),
            'avg_instructions': statistics.mean(instructions),
            'misses_per_instruction': statistics.mean(cache_misses) / statistics.mean(instructions) if statistics.mean(instructions) > 0 else 0
        }
    }
    
    return analysis

def print_analysis(analysis):
    """Print formatted analysis results"""
    for alg, data in analysis.items():
        if data is None:
            continue
            
        print(f"\n=== {data['algorithm'].upper()} ===")
        print(f"Samples: {data['sample_count']}")
        
        time = data['execution_time']
        print(f"Execution Time (ms):")
        print(f"  Mean: {time['mean']:.2f}")
        print(f"  Median: {time['median']:.2f}")
        print(f"  StdDev: {time['stdev']:.2f}")
        print(f"  Range: {time['min']:.2f} - {time['max']:.2f}")
        
        cache = data['cache_efficiency']
        print(f"Cache Efficiency:")
        print(f"  L3 Misses: {cache['avg_misses']:.0f}")
        print(f"  Instructions: {cache['avg_instructions']:.0f}")
        print(f"  MPKI: {cache['misses_per_instruction'] * 1000:.2f}")

def main():
    if len(sys.argv) != 2:
        print("Usage: analyze_results.py <results_directory>")
        sys.exit(1)
        
    result_dir = sys.argv[1]
    
    if not os.path.exists(result_dir):
        print(f"Error: Directory {result_dir} does not exist")
        sys.exit(1)
        
    print(f"Analyzing results in: {result_dir}")
    
    results = load_results(result_dir)
    
    if not results:
        print("No valid result files found")
        sys.exit(1)
        
    analysis = {}
    for algorithm, algorithm_results in results.items():
        analysis[algorithm] = analyze_algorithm(algorithm_results)
        
    print_analysis(analysis)
    
    # Save summary
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    summary_file = os.path.join(result_dir, f"summary_{timestamp}.json")
    
    with open(summary_file, 'w') as f:
        json.dump(analysis, f, indent=2)
        
    print(f"\nSummary saved to: {summary_file}")

if __name__ == "__main__":
    main()