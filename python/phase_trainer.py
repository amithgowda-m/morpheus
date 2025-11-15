#!/usr/bin/env python3

try:
    import pandas as pd  # type: ignore[import]
except ImportError:
    import sys
    print("Error: missing required dependency 'pandas'. Install it with: pip install pandas")
    sys.exit(1)
import numpy as np
import json
import argparse
from sklearn.tree import DecisionTreeClassifier, export_text
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score, classification_report
from sklearn.preprocessing import LabelEncoder
import matplotlib.pyplot as plt
import seaborn as sns

class PhaseTrainer:
    def __init__(self):
        self.model = DecisionTreeClassifier(
            max_depth=5,
            min_samples_split=10,
            min_samples_leaf=5,
            random_state=42
        )
        self.label_encoder = LabelEncoder()
        self.feature_names = [
            'l3_miss_rate',
            'ipc',
            'branch_miss_rate', 
            'l1_misses',
            'l2_misses',
            'instructions',
            'cycles'
        ]
        self.phase_names = ['DenseSequential', 'SparseRandom', 'PointerChasing', 'Unknown']
        
    def load_data(self, csv_file):
        """Load training data from CSV file"""
        try:
            df = pd.read_csv(csv_file)
            print(f"Loaded data with {len(df)} samples")
            print(f"Columns: {df.columns.tolist()}")
            return df
        except Exception as e:
            print(f"Error loading data: {e}")
            return None
    
    def preprocess_data(self, df):
        """Preprocess the training data"""
        # Select features and target
        X = df[self.feature_names].copy()
        
        # Handle missing values
        X = X.fillna(X.mean())
        
        # Encode target variable
        y = self.label_encoder.fit_transform(df['phase'])
        
        return X, y
    
    def train(self, X, y):
        """Train the decision tree model"""
        # Split data
        X_train, X_test, y_train, y_test = train_test_split(
            X, y, test_size=0.2, random_state=42, stratify=y
        )
        
        # Train model
        self.model.fit(X_train, y_train)
        
        # Evaluate
        y_pred = self.model.predict(X_test)
        accuracy = accuracy_score(y_test, y_pred)
        
        print(f"Model accuracy: {accuracy:.3f}")
        print("\nClassification Report:")
        print(classification_report(y_test, y_pred, 
                                  target_names=self.label_encoder.classes_))
        
        return accuracy
    
    def export_model(self, output_file):
        """Export model as C++ decision tree rules"""
        tree_rules = export_text(self.model, 
                               feature_names=self.feature_names,
                               decimals=3)
        
        # Generate C++ code
        cpp_code = self._generate_cpp_code(tree_rules)
        
        with open(output_file, 'w') as f:
            f.write(cpp_code)
        
        print(f"Model exported to: {output_file}")
        
        # Also export feature importance
        self._export_feature_importance(output_file + '.importance.json')
    
    def _generate_cpp_code(self, tree_rules):
        """Generate C++ code from decision tree rules"""
        
        cpp_code = f"""// Auto-generated decision tree classifier
// Features: {', '.join(self.feature_names)}
// Classes: {', '.join(self.label_encoder.classes_)}

#include <vector>
#include <string>
#include <cmath>

namespace morpheus {{

enum class ExecutionPhase {{
    {', '.join(self.phase_names)}
}};

class TrainedPhaseClassifier {{
public:
    static ExecutionPhase classify(const std::vector<double>& features) {{
        if (features.size() < {len(self.feature_names)}) {{
            return ExecutionPhase::Unknown;
        }}
        
        // Extract features
        double {', '.join([f'{name} = features[{i}]' for i, name in enumerate(self.feature_names)])};
        
        // Decision tree rules
{self._convert_rules_to_cpp(tree_rules)}
    }}
    
private:
    static constexpr double L3_MISS_THRESHOLD = 0.01;
    static constexpr double IPC_THRESHOLD = 1.0;
    static constexpr double BRANCH_MISS_THRESHOLD = 0.05;
}};

}} // namespace morpheus
"""
        return cpp_code
    
    def _convert_rules_to_cpp(self, tree_rules):
        """Convert trained sklearn DecisionTree to nested C++ if/else statements
        by walking the tree_ structure (more robust than parsing export_text).
        """
        tree = self.model.tree_
        feature = tree.feature
        threshold = tree.threshold
        children_left = tree.children_left
        children_right = tree.children_right
        value = tree.value

        def recurse(node, depth=2):
            indent = '    ' * depth
            # Leaf node
            if children_left[node] == children_right[node]:
                # value[node] is shape (1, n_classes)
                class_id = int(np.argmax(value[node][0]))
                phase_name = self.label_encoder.classes_[class_id]
                return indent + f'return ExecutionPhase::{phase_name};\n'

            feat_idx = int(feature[node])
            thresh = float(threshold[node])

            # Left branch: feature <= thresh
            left_code = recurse(children_left[node], depth + 1)
            # Right branch: feature > thresh
            right_code = recurse(children_right[node], depth + 1)

            cond = f'features[{feat_idx}] <= {thresh:.6f}'
            code = indent + f'if ({cond}) {{\n'
            code += left_code
            code += indent + '} else {\n'
            code += right_code
            code += indent + '}\n'
            return code

        # Start recursion from root (node 0) with depth 2 (to match surrounding indentation)
        return recurse(0, depth=2)
    
    def _export_feature_importance(self, output_file):
        """Export feature importance as JSON"""
        importance_dict = {
            'feature_importance': dict(zip(self.feature_names, self.model.feature_importances_))
        }
        
        with open(output_file, 'w') as f:
            json.dump(importance_dict, f, indent=2)
    
    def plot_feature_importance(self, output_file=None):
        """Plot feature importance"""
        importance_df = pd.DataFrame({
            'feature': self.feature_names,
            'importance': self.model.feature_importances_
        }).sort_values('importance', ascending=True)
        
        plt.figure(figsize=(10, 6))
        plt.barh(importance_df['feature'], importance_df['importance'])
        plt.xlabel('Feature Importance')
        plt.title('Decision Tree Feature Importance')
        plt.tight_layout()
        
        if output_file:
            plt.savefig(output_file, dpi=300, bbox_inches='tight')
            print(f"Feature importance plot saved to: {output_file}")
        else:
            plt.show()

def main():
    parser = argparse.ArgumentParser(description='Train phase classification model')
    parser.add_argument('--input', '-i', required=True, help='Input CSV file with training data')
    parser.add_argument('--output', '-o', required=True, help='Output C++ header file')
    parser.add_argument('--plot', '-p', help='Output plot file (optional)')
    
    args = parser.parse_args()
    
    # Initialize trainer
    trainer = PhaseTrainer()
    
    # Load data
    df = trainer.load_data(args.input)
    if df is None:
        return 1
    
    # Preprocess data
    X, y = trainer.preprocess_data(df)
    
    # Train model
    accuracy = trainer.train(X, y)
    
    # Export model
    trainer.export_model(args.output)
    
    # Plot feature importance
    if args.plot:
        trainer.plot_feature_importance(args.plot)
    
    print(f"Training completed with accuracy: {accuracy:.3f}")
    return 0

if __name__ == '__main__':
    main()