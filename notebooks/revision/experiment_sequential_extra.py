#!/usr/bin/env python3

import subprocess
import time
import logging
from typing import List, Dict
import re
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import sys
import json
from dataclasses import dataclass

# Base paths setup
BASE_DIR = Path(__file__).parent.parent.parent
# BINARY_DIR = BASE_DIR / "build/release/bin/release"
BINARY_DIR = BASE_DIR / "build/release/bin/release"
RESULTS_DIR = BASE_DIR / "experiments/sequential/"
MATERIAL_COLORS_FILE = BASE_DIR / "notebooks/material-colors.json"

# Create necessary directories
for directory in [RESULTS_DIR]:
    directory.mkdir(parents=True, exist_ok=True)

@dataclass
class ExperimentConfig:
    dataset: str  # Changed from dist_param
    base_unit: int
    theta: float
    line_read: int = 10000000
    num_runs: int = 30

    def get_base_flags(self) -> str:
        return (f"--app.line_read={self.line_read} "
                f"--app.tuples_no={int(self.line_read*1.2)} "
                f"--app.theta={self.theta} "
                f"--app.dataset={self.dataset} "  # Changed from dist_param
                f"--app.num_runs={self.num_runs}")

    def get_exp_dir(self, base_dir: Path) -> Path:
        return base_dir / f"DATASET={self.dataset}/BASE_UNIT={self.base_unit}/THETA={self.theta}"

    def get_log_dir(self, base_dir: Path) -> Path:
        return self.get_exp_dir(base_dir) / "logs"

    def get_figure_dir(self, base_dir: Path) -> Path:
        return self.get_exp_dir(base_dir) / "figures"
    
class Logger:
    def __init__(self, log_dir: Path):
        self.log_dir = log_dir
        self.console_handler = logging.StreamHandler()
        self.console_handler.setFormatter(
            logging.Formatter('%(asctime)s - %(name)s - %(message)s',
                            datefmt='%Y-%m-%d %H:%M:%S')
        )

    def get_logger(self, name: str) -> logging.Logger:
        logger = logging.getLogger(name)
        logger.setLevel(logging.INFO)
        
        if logger.hasHandlers():
            logger.handlers.clear()
            
        file_handler = logging.FileHandler(self.log_dir / f'{name}.log', mode='w')
        file_handler.setFormatter(
            logging.Formatter('%(asctime)s - %(message)s',
                            datefmt='%Y-%m-%d %H:%M:%S')
        )
        
        logger.addHandler(file_handler)
        logger.addHandler(self.console_handler)
        logger.propagate = False
        
        return logger

class ExperimentRunner:
    RESULT_PATTERN = r'RESULT_SUMMARY: FrequencyEstimator=(\w+(?:\s*\w+)*) TotalHeavyHitters=(\d+) TotalHeavyHitterCandidates=(\d+) Precision=([\d.e-]+) Recall=([\d.e-]+) F1Score=([\d.e-]+) ARE=([\d.e-]+) AAE=([\d.e-]+) ExecutionTime=([\d.e-]+) Throughput=([\d.e-]+)'
    METRICS = ['Precision', 'Recall', 'ARE', 'AAE', 'ExecutionTime', 'Throughput', 'F1Score']

    def __init__(self, binary_dir: Path, results_dir: Path):
        self.binary_dir = binary_dir
        self.results_dir = results_dir
        
        with open(MATERIAL_COLORS_FILE, 'r') as f:
            self.colors = json.load(f)

    def _get_sketch_commands(self, config: ExperimentConfig) -> Dict[str, str]:
        """Generate command flags for different sketch algorithms."""
        base_unit = config.base_unit
        theta = config.theta
        return {
            "example_count_min": f"--countmin.width={32*base_unit} --countmin.depth={8}",
            "example_heavy_keeper": f"--heavykeeper.m2={85*base_unit}",
            "example_cuckoo_heavy_keeper": f"--cuckooheavykeeper.bucket_num={32*base_unit} --cuckooheavykeeper.theta={theta}",
            "example_augmented_sketch": f"--augmentedsketch.width={29 + (base_unit-1) * 32} --augmentedsketch.depth={8}",
            "example_heap_hashmap_space_saving": f"--spacesaving.k={43*base_unit}",
        }

    def _run_command(self, command: List[str], logger: logging.Logger) -> None:
        """Execute a command and log its output."""
        logger.info(f"Running command: {' '.join(command)}")
        start_time = time.time()
        
        try:
            process = subprocess.Popen(
                command,
                cwd=self.binary_dir,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                universal_newlines=True
            )
            
            for line in process.stdout:
                logger.info(line.strip())
            
            process.wait()
            if process.returncode != 0:
                logger.error(f"Command failed with return code {process.returncode}")
            
        except Exception as e:
            logger.error(f"Error: {str(e)}")
        
        logger.info(f"Duration: {time.time() - start_time:.2f} seconds")

    def _process_results(self, log_files: List[Path]) -> pd.DataFrame:
        """Process log files and return combined results as DataFrame."""
        data = []
        for log_file in log_files:
            with open(log_file, 'r') as file:
                for line in file:
                    if match := re.search(self.RESULT_PATTERN, line):
                        data.append({
                            'FrequencyEstimator': match.group(1),
                            'TotalHeavyHitters': int(match.group(2)),
                            'TotalHeavyHitterCandidates': int(match.group(3)),
                            'Precision': float(match.group(4)),
                            'Recall': float(match.group(5)),
                            'F1Score': float(match.group(6)),
                            'ARE': float(match.group(7)),
                            'AAE': float(match.group(8)),
                            'ExecutionTime': float(match.group(9)),
                            'Throughput': float(match.group(10))
                        })
        return pd.DataFrame(data)

    def _plot_metrics(self, df: pd.DataFrame, exp_dir: Path) -> None:
        """Generate plots for all metrics."""
        color_palette = [self.colors[c]['500'] for c in ['blue', 'red', 'green', 'orange', 'purple']]
        
        for metric in self.METRICS:
            plt.figure(figsize=(10, 6))
            sns.set_style("whitegrid")
            
            sns.boxplot(x='FrequencyEstimator', y=metric, data=df, 
                       palette=color_palette, width=0.6)
            
            plt.title(f'{metric} Comparison', fontsize=14, y=1.05)
            plt.xlabel('Frequency Estimator', fontsize=12)
            plt.ylabel(metric, fontsize=12)
            
            # Add statistics
            for i, estimator in enumerate(df['FrequencyEstimator'].unique()):
                stats = df[df['FrequencyEstimator'] == estimator][metric]
                label = f"{stats.mean():.2f}±{stats.std():.2f}"
                plt.text(i, plt.ylim()[1], label, ha='center', va='bottom', 
                        fontweight='bold', fontsize=10)
            
            plt.tight_layout()
            plt.savefig(exp_dir / f'{metric}.png', dpi=300, bbox_inches='tight')
            plt.close()

    def run(self, config: ExperimentConfig) -> None:
        """Run experiment with given configuration."""
        # Create experiment directories
        exp_dir = config.get_exp_dir(self.results_dir)
        log_dir = config.get_log_dir(self.results_dir)
        figure_dir = config.get_figure_dir(self.results_dir)
        
        for directory in [exp_dir, log_dir, figure_dir]:
            directory.mkdir(parents=True, exist_ok=True)
        
        # Initialize logger with the new log directory
        self.logger = Logger(log_dir)
        
        # Run each sketch algorithm
        for cmd_name, cmd_flags in self._get_sketch_commands(config).items():
            cmd_logger = self.logger.get_logger(cmd_name)
            command = [f"./{cmd_name}"] + config.get_base_flags().split() + cmd_flags.split()
            self._run_command(command, cmd_logger)
            print (command)
        
        # Process and plot results
        df = self._process_results(list(log_dir.glob('*.log')))
        self._plot_metrics(df, figure_dir)
        
        # Print summary statistics
        print(df.groupby('FrequencyEstimator').agg({
            metric: ['mean', 'std'] for metric in self.METRICS
        }))

def main():
    configs = [
        ExperimentConfig(dataset=ds, base_unit=bu, theta=t)
        # for ds in ["WebDocs", "AdTracking", "CAIDA"]  # Available datasets
        for ds in ["CAIDA_L", "CAIDA_H", "WebDocs", "AdTracking"]  # Available datasets
        for bu in [2, 4, 8, 16, 32, 64]
        for t in [0.0001, 0.0005, 0.001, 0.005, 0.01]
    ]
    
    runner = ExperimentRunner(BINARY_DIR, RESULTS_DIR)
    
    for config in configs:
        print(f"\nRunning experiment with:")
        print(f"Dataset: {config.dataset}")
        print(f"Base Unit: {config.base_unit}")
        print(f"Theta: {config.theta}")
        
        runner.run(config)
        sys.stdout.flush()

if __name__ == "__main__":
    main()
