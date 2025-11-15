#!/usr/bin/env python3
"""
Example: Streaming Morpheus Benchmark Results to Dashboard

This script demonstrates how to:
1. Run Morpheus benchmarks
2. Stream results to the dashboard in real-time
3. Update charts and gauges live
"""

import asyncio
import aiohttp
import json
import time
import random
from pathlib import Path
from typing import Tuple
import subprocess

# Try to import the benchmark parser
try:
    from benchmark_parser import BenchmarkParser, BenchmarkSuite
except ImportError:
    print("Note: benchmark_parser not imported (optional for this demo)")

class BenchmarkStreamer:
    """Stream benchmark results to dashboard"""
    
    def __init__(self, server_url: str = 'http://localhost:8080'):
        self.server_url = server_url
        self.session = None
    
    async def __aenter__(self):
        self.session = aiohttp.ClientSession()
        return self
    
    async def __aexit__(self, *args):
        if self.session:
            await self.session.close()
    
    async def report_result(self, algorithm: str, baseline_ms: float, 
                          morpheus_ms: float, baseline_std: float = 15.0,
                          morpheus_std: float = 12.0, **kwargs) -> bool:
        """
        Report a benchmark result to the dashboard
        
        Args:
            algorithm: Algorithm name (BFS, PageRank, Betweenness)
            baseline_ms: Baseline execution time
            morpheus_ms: Morpheus execution time
            baseline_std: Baseline standard deviation
            morpheus_std: Morpheus standard deviation
            **kwargs: Additional metrics (L1/L2/L3 hit rates, IPC, etc.)
        
        Returns:
            True if successful, False otherwise
        """
        
        try:
            data = {
                'algorithm': algorithm,
                'baseline_ms': baseline_ms,
                'morpheus_ms': morpheus_ms,
                'baseline_std': baseline_std,
                'morpheus_std': morpheus_std,
                **kwargs
            }
            
            async with self.session.post(
                f'{self.server_url}/api/update',
                json=data
            ) as resp:
                if resp.status == 200:
                    result = await resp.json()
                    print(f"✓ {algorithm}: {result['message']}")
                    return True
                else:
                    error = await resp.text()
                    print(f"✗ {algorithm}: {error}")
                    return False
        
        except aiohttp.ClientConnectorError:
            print(f"✗ Cannot connect to dashboard at {self.server_url}")
            print("  Make sure dashboard_server.py is running:")
            print("  python dashboard_server.py")
            return False
        except Exception as e:
            print(f"✗ Error reporting {algorithm}: {e}")
            return False
    
    async def get_current_data(self) -> dict:
        """Get current data from dashboard"""
        try:
            async with self.session.get(f'{self.server_url}/api/data') as resp:
                if resp.status == 200:
                    return await resp.json()
        except Exception as e:
            print(f"✗ Error getting data: {e}")
        return {}

async def example_1_simple_update():
    """Example 1: Simple benchmark result update"""
    
    print("\n" + "="*70)
    print("EXAMPLE 1: Simple Benchmark Result Update")
    print("="*70)
    
    async with BenchmarkStreamer() as streamer:
        # Report a single result
        await streamer.report_result(
            algorithm='BFS',
            baseline_ms=250,
            morpheus_ms=198,
            baseline_std=15,
            morpheus_std=12
        )

async def example_2_multiple_algorithms():
    """Example 2: Report results for multiple algorithms"""
    
    print("\n" + "="*70)
    print("EXAMPLE 2: Multiple Algorithm Results")
    print("="*70)
    
    algorithms = [
        ('BFS', 250, 198),
        ('PageRank', 350, 248),
        ('Betweenness', 420, 365),
    ]
    
    async with BenchmarkStreamer() as streamer:
        for algo, baseline, morpheus in algorithms:
            await streamer.report_result(
                algorithm=algo,
                baseline_ms=baseline,
                morpheus_ms=morpheus
            )
            # Small delay to see updates appear
            await asyncio.sleep(1)

async def example_3_with_cache_metrics():
    """Example 3: Report with cache hit rate metrics"""
    
    print("\n" + "="*70)
    print("EXAMPLE 3: Results with Cache Metrics")
    print("="*70)
    
    cache_metrics = {
        'BFS': {
            'baseline_ms': 250,
            'morpheus_ms': 198,
            'l1_hit_rate': 0.88,
            'l2_hit_rate': 0.92,
            'l3_hit_rate': 0.96,
            'ipc': 2.4,
            'branch_accuracy': 0.98
        },
        'PageRank': {
            'baseline_ms': 350,
            'morpheus_ms': 248,
            'l1_hit_rate': 0.85,
            'l2_hit_rate': 0.90,
            'l3_hit_rate': 0.95,
            'ipc': 2.8,
            'branch_accuracy': 0.97
        }
    }
    
    async with BenchmarkStreamer() as streamer:
        for algo, metrics in cache_metrics.items():
            baseline = metrics.pop('baseline_ms')
            morpheus = metrics.pop('morpheus_ms')
            
            await streamer.report_result(
                algorithm=algo,
                baseline_ms=baseline,
                morpheus_ms=morpheus,
                **metrics
            )
            await asyncio.sleep(1)

async def example_4_simulated_live_stream():
    """Example 4: Simulate live streaming with random variations"""
    
    print("\n" + "="*70)
    print("EXAMPLE 4: Simulated Live Streaming (10 iterations)")
    print("="*70)
    print("Watch the dashboard gauge and timeline update in real-time!\n")
    
    base_values = {
        'BFS': (250, 198),
        'PageRank': (350, 248),
        'Betweenness': (420, 365),
    }
    
    async with BenchmarkStreamer() as streamer:
        for iteration in range(10):
            print(f"\n--- Iteration {iteration + 1}/10 ---")
            
            # Report results with slight variations
            for algo, (baseline, morpheus) in base_values.items():
                # Add random variation (±5%)
                varied_baseline = baseline * (0.95 + random.random() * 0.1)
                varied_morpheus = morpheus * (0.95 + random.random() * 0.1)
                
                await streamer.report_result(
                    algorithm=algo,
                    baseline_ms=varied_baseline,
                    morpheus_ms=varied_morpheus,
                    baseline_std=15 * (0.8 + random.random() * 0.4),
                    morpheus_std=12 * (0.8 + random.random() * 0.4)
                )
            
            # Wait between iterations
            if iteration < 9:
                print("Waiting 5 seconds before next iteration...")
                await asyncio.sleep(5)

async def example_5_load_from_file():
    """Example 5: Load benchmark results from JSON and stream"""
    
    print("\n" + "="*70)
    print("EXAMPLE 5: Load Results from JSON File")
    print("="*70)
    
    # Create sample JSON file
    sample_data = {
        'timestamp': '2024-11-15T12:00:00',
        'results': [
            {
                'algorithm': 'BFS',
                'baseline_ms': 248,
                'morpheus_ms': 196,
                'l1_hit_rate': 0.88,
                'l2_hit_rate': 0.92
            },
            {
                'algorithm': 'PageRank',
                'baseline_ms': 352,
                'morpheus_ms': 250,
                'l1_hit_rate': 0.85,
                'l2_hit_rate': 0.90
            }
        ]
    }
    
    json_file = Path('/tmp/benchmark_results.json')
    json_file.write_text(json.dumps(sample_data, indent=2))
    print(f"Created sample JSON: {json_file}\n")
    
    # Load and stream
    async with BenchmarkStreamer() as streamer:
        data = json.loads(json_file.read_text())
        
        for result in data['results']:
            algo = result.pop('algorithm')
            baseline = result.pop('baseline_ms')
            morpheus = result.pop('morpheus_ms')
            
            await streamer.report_result(
                algorithm=algo,
                baseline_ms=baseline,
                morpheus_ms=morpheus,
                **result
            )
            await asyncio.sleep(1)

async def example_6_continuous_monitoring():
    """Example 6: Continuous monitoring loop"""
    
    print("\n" + "="*70)
    print("EXAMPLE 6: Continuous Monitoring (Run until Ctrl+C)")
    print("="*70 + "\n")
    
    async with BenchmarkStreamer() as streamer:
        iteration = 0
        
        try:
            while True:
                iteration += 1
                
                # Simulate benchmark results
                results = {
                    'BFS': (250 + random.gauss(0, 10), 198 + random.gauss(0, 8)),
                    'PageRank': (350 + random.gauss(0, 15), 248 + random.gauss(0, 12)),
                    'Betweenness': (420 + random.gauss(0, 20), 365 + random.gauss(0, 16)),
                }
                
                print(f"Cycle {iteration}")
                for algo, (baseline, morpheus) in results.items():
                    await streamer.report_result(
                        algorithm=algo,
                        baseline_ms=max(150, baseline),
                        morpheus_ms=max(100, morpheus)
                    )
                
                # Wait before next cycle
                print("Waiting 10 seconds...\n")
                await asyncio.sleep(10)
        
        except KeyboardInterrupt:
            print("\n✓ Monitoring stopped")

async def main():
    """Run examples"""
    
    print("\n")
    print("╔" + "="*68 + "╗")
    print("║" + " "*68 + "║")
    print("║" + "  Morpheus Dashboard - Benchmark Streaming Examples".center(68) + "║")
    print("║" + " "*68 + "║")
    print("╚" + "="*68 + "╝")
    
    print("\n📝 These examples show how to stream benchmark results to the dashboard.")
    print("   Make sure dashboard_server.py is running first!\n")
    
    examples = [
        ("1", "Simple Update", example_1_simple_update),
        ("2", "Multiple Algorithms", example_2_multiple_algorithms),
        ("3", "With Cache Metrics", example_3_with_cache_metrics),
        ("4", "Live Streaming", example_4_simulated_live_stream),
        ("5", "Load from JSON", example_5_load_from_file),
        ("6", "Continuous Monitor", example_6_continuous_monitoring),
    ]
    
    print("Available Examples:")
    for num, desc, _ in examples:
        print(f"  {num}. {desc}")
    print("  0. Exit")
    
    print()
    
    while True:
        choice = input("Run example (0-6): ").strip()
        
        if choice == "0":
            print("Goodbye!")
            break
        
        try:
            idx = int(choice) - 1
            if 0 <= idx < len(examples):
                num, desc, func = examples[idx]
                print(f"\nRunning: {desc}")
                await func()
            else:
                print("Invalid choice")
        except ValueError:
            print("Please enter a number (0-6)")
        except Exception as e:
            print(f"Error: {e}")

if __name__ == '__main__':
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\n\n✓ Examples stopped")
