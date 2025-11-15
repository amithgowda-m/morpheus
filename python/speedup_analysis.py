#!/usr/bin/env python3
"""
Speedup Analysis for Morpheus ML-optimized vs Baseline Configurations
Computes effect sizes, confidence intervals, and statistical significance.
"""

import numpy as np
from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple
from scipy import stats
import json
from pathlib import Path


@dataclass
class SpeedupMetrics:
    """Speedup statistics between two configurations"""
    baseline_config: str
    optimized_config: str
    algorithm: str
    graph_size: Optional[int] = None
    
    # Raw metrics
    baseline_times_ms: List[float] = None
    optimized_times_ms: List[float] = None
    
    # Computed stats
    baseline_mean: float = 0.0
    optimized_mean: float = 0.0
    speedup_factor: float = 1.0
    
    # Statistical measures
    baseline_std: float = 0.0
    optimized_std: float = 0.0
    speedup_ci_lower: float = 1.0
    speedup_ci_upper: float = 1.0
    p_value: float = 1.0
    cohens_d: float = 0.0
    
    def __post_init__(self):
        if self.baseline_times_ms is None:
            self.baseline_times_ms = []
        if self.optimized_times_ms is None:
            self.optimized_times_ms = []
    
    def to_dict(self) -> Dict:
        """Convert to JSON-serializable dict"""
        return {
            'baseline_config': self.baseline_config,
            'optimized_config': self.optimized_config,
            'algorithm': self.algorithm,
            'graph_size': self.graph_size,
            'baseline_mean_ms': round(self.baseline_mean, 6),
            'optimized_mean_ms': round(self.optimized_mean, 6),
            'speedup_factor': round(self.speedup_factor, 4),
            'speedup_ci_lower': round(self.speedup_ci_lower, 4),
            'speedup_ci_upper': round(self.speedup_ci_upper, 4),
            'p_value': round(self.p_value, 6),
            'cohens_d': round(self.cohens_d, 4),
            'baseline_std_ms': round(self.baseline_std, 6),
            'optimized_std_ms': round(self.optimized_std, 6),
        }


class SpeedupAnalyzer:
    """Compute speedup ratios and statistical measures"""
    
    @staticmethod
    def compute_speedup(baseline_times: List[float], 
                       optimized_times: List[float]) -> SpeedupMetrics:
        """
        Compute speedup factor with 95% CI and significance testing.
        
        Speedup = baseline_time / optimized_time
        CI computed via bootstrap resampling
        Significance via paired t-test (if matched) or Welch's t-test
        Effect size via Cohen's d
        """
        if len(baseline_times) == 0 or len(optimized_times) == 0:
            raise ValueError("Empty time lists")
        
        baseline_arr = np.array(baseline_times)
        optimized_arr = np.array(optimized_times)
        
        baseline_mean = np.mean(baseline_arr)
        optimized_mean = np.mean(optimized_arr)
        speedup = baseline_mean / optimized_mean
        
        baseline_std = np.std(baseline_arr, ddof=1)
        optimized_std = np.std(optimized_arr, ddof=1)
        
        # Confidence interval on speedup via bootstrap
        speedup_ci = SpeedupAnalyzer._bootstrap_ci(
            baseline_arr, optimized_arr, statistic=lambda b, o: np.mean(b) / np.mean(o),
            n_bootstrap=10000, ci=0.95
        )
        
        # Significance test: Welch's t-test
        t_stat, p_value = stats.ttest_ind(baseline_arr, optimized_arr, equal_var=False)
        
        # Cohen's d effect size (pooled)
        cohens_d = SpeedupAnalyzer._cohens_d(baseline_arr, optimized_arr)
        
        metrics = SpeedupMetrics(
            baseline_config="baseline",
            optimized_config="optimized",
            algorithm="unknown",
            baseline_times_ms=baseline_times,
            optimized_times_ms=optimized_times,
            baseline_mean=baseline_mean,
            optimized_mean=optimized_mean,
            speedup_factor=speedup,
            baseline_std=baseline_std,
            optimized_std=optimized_std,
            speedup_ci_lower=speedup_ci[0],
            speedup_ci_upper=speedup_ci[1],
            p_value=p_value,
            cohens_d=cohens_d,
        )
        return metrics
    
    @staticmethod
    def _bootstrap_ci(baseline: np.ndarray, optimized: np.ndarray,
                     statistic, n_bootstrap: int = 10000, ci: float = 0.95) -> Tuple[float, float]:
        """Bootstrap confidence interval for speedup statistic"""
        bootstrap_stats = []
        n = len(baseline)
        
        for _ in range(n_bootstrap):
            # Resample with replacement
            idx_b = np.random.choice(len(baseline), size=len(baseline), replace=True)
            idx_o = np.random.choice(len(optimized), size=len(optimized), replace=True)
            
            b_sample = baseline[idx_b]
            o_sample = optimized[idx_o]
            
            stat = statistic(b_sample, o_sample)
            bootstrap_stats.append(stat)
        
        bootstrap_stats = np.array(bootstrap_stats)
        alpha = 1.0 - ci
        lower = np.percentile(bootstrap_stats, 100 * alpha / 2)
        upper = np.percentile(bootstrap_stats, 100 * (1 - alpha / 2))
        
        return (lower, upper)
    
    @staticmethod
    def _cohens_d(group1: np.ndarray, group2: np.ndarray) -> float:
        """Cohen's d effect size (pooled standard deviation)"""
        n1, n2 = len(group1), len(group2)
        var1, var2 = np.var(group1, ddof=1), np.var(group2, ddof=1)
        
        pooled_std = np.sqrt(((n1 - 1) * var1 + (n2 - 1) * var2) / (n1 + n2 - 2))
        
        if pooled_std == 0:
            return 0.0
        
        return (np.mean(group1) - np.mean(group2)) / pooled_std
    
    @staticmethod
    def speedup_summary(speedup_metrics: List[SpeedupMetrics]) -> Dict:
        """Summarize speedup across multiple runs"""
        if not speedup_metrics:
            return {}
        
        speedups = [m.speedup_factor for m in speedup_metrics]
        
        return {
            'count': len(speedup_metrics),
            'mean_speedup': round(np.mean(speedups), 4),
            'median_speedup': round(np.median(speedups), 4),
            'std_speedup': round(np.std(speedups), 4),
            'min_speedup': round(np.min(speedups), 4),
            'max_speedup': round(np.max(speedups), 4),
            'significant_count': sum(1 for m in speedup_metrics if m.p_value < 0.05),
            'avg_cohens_d': round(np.mean([m.cohens_d for m in speedup_metrics]), 4),
        }


def generate_acm_table(speedup_list: List[SpeedupMetrics], 
                       output_file: Optional[Path] = None) -> str:
    """
    Generate ACM-ready table with speedups, CI, and significance markers.
    Format suitable for paper tables.
    """
    lines = [
        r"\begin{table}[h]",
        r"\centering",
        r"\caption{Speedup Analysis: Morpheus (ML-optimized) vs Baseline}",
        r"\label{tab:speedup}",
        r"\begin{tabular}{|l|c|c|c|c|c|}",
        r"\hline",
        r"Algorithm & Graph Size & Speedup & 95\% CI & p-value & Cohen's $d$ \\",
        r"\hline",
    ]
    
    for metric in speedup_list:
        significance = "***" if metric.p_value < 0.001 else "**" if metric.p_value < 0.01 else "*" if metric.p_value < 0.05 else ""
        
        line = (
            f"{metric.algorithm} & "
            f"{metric.graph_size if metric.graph_size else 'N/A'} & "
            f"{metric.speedup_factor:.2f}x{significance} & "
            f"[{metric.speedup_ci_lower:.2f}, {metric.speedup_ci_upper:.2f}] & "
            f"{metric.p_value:.4f} & "
            f"{metric.cohens_d:.2f} \\\\"
        )
        lines.append(line)
    
    lines.extend([
        r"\hline",
        r"\end{tabular}",
        r"\end{table}",
    ])
    
    table_text = "\n".join(lines)
    
    if output_file:
        Path(output_file).write_text(table_text)
    
    return table_text


# Example usage
if __name__ == "__main__":
    # Example: Compare two sets of timings
    baseline = [10.5, 10.3, 10.7, 10.2, 10.6]
    optimized = [8.1, 8.3, 8.0, 8.2, 8.4]
    
    metrics = SpeedupAnalyzer.compute_speedup(baseline, optimized)
    metrics.algorithm = "BFS"
    metrics.graph_size = 1000
    
    print(f"Speedup: {metrics.speedup_factor:.2f}x")
    print(f"95% CI: [{metrics.speedup_ci_lower:.2f}, {metrics.speedup_ci_upper:.2f}]")
    print(f"p-value: {metrics.p_value:.4f}")
    print(f"Cohen's d: {metrics.cohens_d:.4f}")
    
    # ACM table
    table = generate_acm_table([metrics])
    print("\nACM LaTeX Table:")
    print(table)
