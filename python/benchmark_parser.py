#!/usr/bin/env python3
"""
Benchmark Results Parser for Morpheus Adaptive Prefetching System
Extracts and organizes performance metrics from JSON benchmark outputs.
"""

import json
import os
from pathlib import Path
from dataclasses import dataclass, asdict
from typing import List, Dict, Optional, Tuple
import numpy as np
from datetime import datetime


@dataclass
class BenchmarkResult:
    """Single benchmark run result"""
    algorithm: str
    iterations: int
    graph_vertices: int
    graph_edges: int
    min_time_ns: int
    max_time_ns: int
    avg_time_ns: int
    execution_time_ms: float
    timestamp: int
    performance_samples: int = 0
    final_phase: Optional[int] = None
    avg_convergence_iterations: Optional[int] = None  # PageRank specific
    sample_size: Optional[int] = None  # Betweenness specific
    
    @property
    def std_dev_ns(self) -> float:
        """Estimate std dev from min/max"""
        return (self.max_time_ns - self.min_time_ns) / 4.0
    
    @property
    def throughput_ops_per_sec(self) -> float:
        """Operations per second (1 / avg_time_ns)"""
        return 1e9 / self.avg_time_ns if self.avg_time_ns > 0 else 0.0
    
    def to_dict(self) -> Dict:
        return asdict(self)


@dataclass
class BenchmarkSuite:
    """Collection of benchmark runs"""
    name: str
    configuration: str
    results: List[BenchmarkResult]
    metadata: Dict = None
    
    def __post_init__(self):
        if self.metadata is None:
            self.metadata = {}
    
    def by_algorithm(self, algorithm: str) -> List[BenchmarkResult]:
        """Filter results by algorithm"""
        return [r for r in self.results if r.algorithm == algorithm]
    
    def by_graph_size(self, vertices: int) -> List[BenchmarkResult]:
        """Filter results by graph vertex count"""
        return [r for r in self.results if r.graph_vertices == vertices]
    
    def summary_stats(self) -> Dict:
        """Compute summary statistics"""
        if not self.results:
            return {}
        
        times_ms = [r.execution_time_ms for r in self.results]
        return {
            'count': len(self.results),
            'mean_time_ms': np.mean(times_ms),
            'median_time_ms': np.median(times_ms),
            'std_dev_ms': np.std(times_ms),
            'min_time_ms': np.min(times_ms),
            'max_time_ms': np.max(times_ms),
        }


class BenchmarkParser:
    """Parse and organize benchmark JSON files"""
    
    def __init__(self, results_dir: str = 'results'):
        self.results_dir = Path(results_dir)
        self.results_dir.mkdir(exist_ok=True)
        self.suites: Dict[str, BenchmarkSuite] = {}
    
    def load_json_file(self, filepath: Path) -> Optional[Dict]:
        """Load single JSON benchmark file"""
        try:
            with open(filepath, 'r') as f:
                return json.load(f)
        except (json.JSONDecodeError, IOError) as e:
            print(f"Error loading {filepath}: {e}")
            return None
    
    def parse_benchmark_result(self, data: Dict) -> Optional[BenchmarkResult]:
        """Convert JSON dict to BenchmarkResult"""
        try:
            return BenchmarkResult(
                algorithm=data.get('algorithm', 'unknown'),
                iterations=int(data.get('iterations', 1)),
                graph_vertices=int(data.get('graph_vertices', 0)),
                graph_edges=int(data.get('graph_edges', 0)),
                min_time_ns=int(data.get('min_time_ns', 0)),
                max_time_ns=int(data.get('max_time_ns', 0)),
                avg_time_ns=int(data.get('avg_time_ns', 0)),
                execution_time_ms=float(data.get('execution_time_ms', 0.0)),
                timestamp=int(data.get('timestamp', 0)),
                performance_samples=int(data.get('performance_samples', 0)),
                final_phase=data.get('final_phase'),
                avg_convergence_iterations=data.get('avg_convergence_iterations'),
                sample_size=data.get('sample_size'),
            )
        except (ValueError, TypeError) as e:
            print(f"Error parsing result: {e}")
            return None
    
    def load_from_directory(self, dir_path: Optional[Path] = None,
                           pattern: str = '*.json') -> Dict[str, BenchmarkSuite]:
        """Load all JSON files from directory"""
        if dir_path is None:
            dir_path = self.results_dir
        
        dir_path = Path(dir_path)
        if not dir_path.exists():
            print(f"Directory {dir_path} does not exist")
            return self.suites
        
        # Group files by configuration (parent dir or filename prefix)
        config_groups: Dict[str, List[Path]] = {}
        for json_file in sorted(dir_path.glob(pattern)):
            config = json_file.stem.split('_')[0]  # e.g., 'baseline' from 'baseline_bfs.json'
            if config not in config_groups:
                config_groups[config] = []
            config_groups[config].append(json_file)
        
        # Parse each group
        for config, files in config_groups.items():
            results = []
            for json_file in files:
                data = self.load_json_file(json_file)
                if data:
                    result = self.parse_benchmark_result(data)
                    if result:
                        results.append(result)
            
            if results:
                suite = BenchmarkSuite(
                    name=config.capitalize(),
                    configuration=config,
                    results=results,
                    metadata={'source_dir': str(dir_path), 'file_count': len(files)}
                )
                self.suites[config] = suite
        
        return self.suites
    
    def load_from_files(self, filepaths: List[Path]) -> BenchmarkSuite:
        """Load results from explicit file list"""
        results = []
        config_name = "custom"
        
        for filepath in filepaths:
            data = self.load_json_file(Path(filepath))
            if data:
                result = self.parse_benchmark_result(data)
                if result:
                    results.append(result)
        
        suite = BenchmarkSuite(
            name=config_name.capitalize(),
            configuration=config_name,
            results=results,
            metadata={'file_count': len(filepaths)}
        )
        return suite
    
    def generate_summary_report(self, output_file: Optional[Path] = None) -> str:
        """Generate text summary of loaded benchmarks"""
        report_lines = [
            "=" * 80,
            f"MORPHEUS BENCHMARK SUMMARY REPORT",
            f"Generated: {datetime.now().isoformat()}",
            "=" * 80,
            ""
        ]
        
        for config, suite in self.suites.items():
            report_lines.append(f"\nConfiguration: {suite.name} ({config})")
            report_lines.append(f"Total runs: {len(suite.results)}")
            
            # Group by algorithm
            for algo in set(r.algorithm for r in suite.results):
                algo_results = suite.by_algorithm(algo)
                report_lines.append(f"\n  {algo.upper()}:")
                report_lines.append(f"    Runs: {len(algo_results)}")
                
                times = [r.execution_time_ms for r in algo_results]
                report_lines.append(f"    Mean time: {np.mean(times):.4f} ms")
                report_lines.append(f"    Std dev: {np.std(times):.4f} ms")
                report_lines.append(f"    Range: [{np.min(times):.4f}, {np.max(times):.4f}] ms")
        
        report_text = "\n".join(report_lines)
        
        if output_file:
            Path(output_file).write_text(report_text)
        
        return report_text
    
    def export_csv(self, output_file: Path, suite_name: Optional[str] = None) -> None:
        """Export results to CSV"""
        import csv
        
        suite = self.suites.get(suite_name) if suite_name else list(self.suites.values())[0]
        if not suite:
            print("No suite found to export")
            return
        
        with open(output_file, 'w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=[
                'algorithm', 'iterations', 'graph_vertices', 'graph_edges',
                'min_time_ns', 'max_time_ns', 'avg_time_ns', 'execution_time_ms',
                'timestamp', 'performance_samples'
            ])
            writer.writeheader()
            for result in suite.results:
                writer.writerow({k: v for k, v in result.to_dict().items() 
                               if k in writer.fieldnames})


# Example usage
if __name__ == "__main__":
    import sys
    
    parser = BenchmarkParser(results_dir='results')
    
    # Try to load from results directory
    suites = parser.load_from_directory()
    
    if suites:
        # Print summary
        report = parser.generate_summary_report()
        print(report)
        
        # Export first suite to CSV
        if suites:
            first_suite = list(suites.values())[0]
            csv_out = Path('results') / f'{first_suite.configuration}_export.csv'
            parser.export_csv(csv_out)
            print(f"\nExported to {csv_out}")
    else:
        print("No benchmark results found. Create results/ directory with .json files.")
        sys.exit(1)
