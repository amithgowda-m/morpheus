#!/usr/bin/env python3
import pandas as pd
import numpy as np
import random

# Generate synthetic performance data for phase classification
np.random.seed(42)

def generate_phase_data(phase_name, samples=1000):
    if phase_name == "DenseSequential":
        l3_miss_rate = np.random.normal(0.005, 0.002, samples)
        ipc = np.random.normal(1.8, 0.3, samples)
        branch_miss_rate = np.random.normal(0.02, 0.01, samples)
    elif phase_name == "SparseRandom":
        l3_miss_rate = np.random.normal(0.03, 0.01, samples)
        ipc = np.random.normal(0.7, 0.2, samples)
        branch_miss_rate = np.random.normal(0.03, 0.01, samples)
    elif phase_name == "PointerChasing":
        l3_miss_rate = np.random.normal(0.015, 0.005, samples)
        ipc = np.random.normal(0.9, 0.2, samples)
        branch_miss_rate = np.random.normal(0.08, 0.02, samples)
    else:
        return None
    
    data = {
        'l3_miss_rate': np.clip(l3_miss_rate, 0.001, 0.1),
        'ipc': np.clip(ipc, 0.3, 2.5),
        'branch_miss_rate': np.clip(branch_miss_rate, 0.005, 0.15),
        'l1_misses': np.random.poisson(1000, samples),
        'l2_misses': np.random.poisson(500, samples),
        'instructions': np.random.normal(1000000, 200000, samples),
        'cycles': np.random.normal(800000, 150000, samples),
        'phase': phase_name
    }
    return pd.DataFrame(data)

# Generate data for all phases
df_dense = generate_phase_data("DenseSequential", 1000)
df_sparse = generate_phase_data("SparseRandom", 1000)
df_pointer = generate_phase_data("PointerChasing", 1000)

# Combine and shuffle
df = pd.concat([df_dense, df_sparse, df_pointer], ignore_index=True)
df = df.sample(frac=1, random_state=42).reset_index(drop=True)

# Save to CSV
df.to_csv('training_data.csv', index=False)
print(f"Generated {len(df)} training samples")
print("Phase distribution:")
print(df['phase'].value_counts())

# Show sample data
print("\nSample data:")
print(df.head())
