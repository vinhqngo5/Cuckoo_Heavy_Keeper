import os
import re
import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots
from typing import List, Dict, Set, Any
import json

import plotly.io as pio   
pio.kaleido.scope.mathjax = None

class LatencyExperimentAnalyzer:
    def __init__(self, base_path: str, fixed_params: Dict[str, Set[str]], subplot_params: List[str]):
        self.base_path = base_path
        self.fixed_params = fixed_params
        self.subplot_params = subplot_params
        self.varying_params = [
            'PARARLLEL_DESIGN',
            'HEAVY_QUERY_RATE'
        ]
        
        # Load colors
        with open('./notebooks/material-colors.json') as f:
            self.material_colors = json.load(f)

    def parse_experiment_path(self, path: str) -> Dict[str, Any]:
        """Extract experiment parameters from path."""
        params = {}
        parts = path.split(os.sep)
        for part in parts:
            if '=' in part:
                key, value = part.split('=')
                params[key] = value
        return params

    def extract_latency(self, file_content: str) -> float:
        """Extract average latency value from file content."""
        pattern = r"Raw latencies \(ns\): ([\d,]+)"
        match = re.search(pattern, file_content)
        if match:
            latencies = [int(x) for x in match.group(1).split(',') if x.strip() and x.strip().lstrip('-').isdigit() and int(x) > 0]
            return sum(latencies) / len(latencies) / 1000  # Convert to microseconds
        return 0.0


    def matches_fixed_params(self, params: Dict[str, str]) -> bool:
        """Check if experiment parameters match the fixed parameters."""
        for key, values in self.fixed_params.items():
            if key in params and params[key] not in values:
                return False
        return True

    # def read_latency_file(self, folder_path: str) -> float:
    #     """Read heavyhitter file and extract latency value."""
    #     for filename in os.listdir(folder_path):
    #         if filename.endswith('_heavyhitter.json'):
    #             file_path = os.path.join(folder_path, filename)
    #             with open(file_path, 'r') as f:
    #                 content = f.read()
    #                 return self.extract_latency(content)
    #     return 0.0
    
    # def analyze_latency_experiments(self) -> List[Dict[str, Any]]:
    #     """Analyze all experiments and prepare visualization data."""
    #     results = []
        
    #     for root, dirs, files in os.walk(self.base_path):
    #         if any(f.endswith('_heavyhitter.json') for f in files):
    #             params = self.parse_experiment_path(root)
                
    #             if not self.matches_fixed_params(params):
    #                 continue
    #             latency = self.read_latency_file(root)
    #             if latency > 0:
    #                 params['latency'] = latency
    #                 results.append(params)
        
    #     return results

    def read_latency_file(self, folder_path: str) -> List[float]:
        """Read all delegation files and extract latency values."""
        latencies = []
        for filename in os.listdir(folder_path):
            if filename.endswith('_heavyhitter.json'):
                file_path = os.path.join(folder_path, filename)
                with open(file_path, 'r') as f:
                    content = f.read()
                    latency = self.extract_latency(content)
                    if latency > 0:
                        latencies.append(latency)
        return latencies

    def analyze_latency_experiments(self) -> List[Dict[str, Any]]:
        """Analyze all experiments and prepare visualization data."""
        results = []
        
        for root, dirs, files in os.walk(self.base_path):
            delegation_files = [f for f in files if f.endswith('_heavyhitter.json')]
            if delegation_files:  # If folder contains delegation files
                params = self.parse_experiment_path(root)
                
                if not self.matches_fixed_params(params):
                    continue
                    
                # Get all latency values for this experiment
                latencies = self.read_latency_file(root)
                
                if latencies:  # Only include if we have valid latency values
                    # Calculate average latency
                    avg_latency = sum(latencies) / len(latencies)
                    params['latency'] = avg_latency
                    params['num_runs'] = len(latencies)  # Optional: track number of runs
                    results.append(params)
        
        return results


    def create_latency_visualization(self, results: List[Dict[str, Any]]) -> go.Figure:
        """Create line chart visualization with improved styling."""
        
        fig = go.Figure()
        
        query_rates_order = ['0.000000', '1.000000', '10.000000']
        query_colors = {
            '0.000000': self.material_colors['purple']['500'],
            '1.000000': self.material_colors['red']['500'],
            '10.000000': self.material_colors['blue']['500']
        }
        
        marker_styles = {
            '0.000000': 'circle',
            '1.000000': 'square',
            '10.000000': 'diamond'
        }
        
        # Group traces by query rate
        for i, query_rate in enumerate(query_rates_order):
            legend_group = f'legend{i+1}'
            
            # Add QPOPSS trace
            matching_results = [r for r in results 
                            if r['PARARLLEL_DESIGN'] == 'QPOPSS' 
                            and r['HEAVY_QUERY_RATE'] == query_rate]
            
            if matching_results:
                sorted_results = sorted(matching_results, key=lambda x: int(x['NUM_THREADS']))
                x_values = [int(r['NUM_THREADS']) for r in sorted_results]
                y_values = [r['latency'] for r in sorted_results]
                
                name = f"mCHK-i ({float(query_rate)/100:.2f}%)"
                
                fig.add_trace(
                    go.Scatter(
                        x=x_values,
                        y=y_values,
                        name=name,
                        mode='lines+markers',
                        line=dict(
                            color=query_colors[query_rate],
                            dash='dot',
                            width=2
                        ),
                        marker=dict(
                            size=10,
                            symbol=marker_styles[query_rate],
                            color=query_colors[query_rate]
                        ),
                        legendgroup=legend_group,
                        legend=legend_group,
                        showlegend=True
                    )
                )

            # Add GLOBAL_HASHMAP trace
            matching_results = [r for r in results 
                            if r['PARARLLEL_DESIGN'] == 'GLOBAL_HASHMAP' 
                            and r['HEAVY_QUERY_RATE'] == query_rate]
            
            if matching_results:
                sorted_results = sorted(matching_results, key=lambda x: int(x['NUM_THREADS']))
                x_values = [int(r['NUM_THREADS']) for r in sorted_results]
                y_values = [r['latency'] for r in sorted_results]
                
                name = f"mCHK-q ({float(query_rate)/100:.2f}%)"
                
                fig.add_trace(
                    go.Scatter(
                        x=x_values,
                        y=y_values,
                        name=name,
                        mode='lines+markers',
                        line=dict(
                            color=query_colors[query_rate],
                            dash='solid',
                            width=2
                        ),
                        marker=dict(
                            size=10,
                            symbol=marker_styles[query_rate],
                            color=query_colors[query_rate]
                        ),
                        legendgroup=legend_group,
                        legend=legend_group,
                        showlegend=True
                    )
                )

        thread_values = sorted([int(x) for x in self.fixed_params['NUM_THREADS']])

        # Configure three legend positions
        legend_positions = [0.2, 0.5, 0.8]
        legend_configs = {}
        
        for i in range(3):
            key = f'legend{i+1}'
            legend_configs[key] = dict(
                yanchor="top",
                y=1.35,
                xanchor="center",
                x=legend_positions[i],
                orientation="h",
                font=dict(family='serif', size=14, color='black'),
                bgcolor='rgba(255, 255, 255, 0)',
                groupclick="toggleitem"
            )

        thread_values = sorted([x for x in self.fixed_params['NUM_THREADS'] if x not in {'2', '5'}])
        fig.update_layout(
            height=300,
            width=640,
            xaxis_title=dict(
                text="Number of Threads",
                font=dict(family='serif', size=16, color='black')
            ),
            yaxis_title=dict(
                text="Latency (μs)",
                font=dict(family='serif', size=16, color='black')
            ),
            template="plotly_white",
            plot_bgcolor='white',
            paper_bgcolor='white',
            showlegend=True,
            margin=dict(
                l=5,
                r=5,
                t=40,
                b=5,
                pad=0
            ),
            font_family='serif',
            xaxis=dict(
                tickfont=dict(size=16, color='black'),
                showline=True,
                linewidth=0.1,
                linecolor='black',
                mirror=True,
                tickmode='array',
                tickvals=thread_values,
                title_standoff=5,
                ticktext=[str(x) for x in thread_values],
                showgrid=False,  # No vertical grid
                gridcolor='lightgray',
                zeroline=False,
                gridwidth=0.1
            ),
            yaxis=dict(
                tickfont=dict(size=16, color='black'),
                showline=True, 
                linewidth=0.1,
                linecolor='black',
                title_standoff=5,
                mirror=True,
                showgrid=True,  # Show horizontal grid
                gridcolor='lightgray',
                zeroline=False,
                gridwidth=0.1
            ),
            **legend_configs
        )

        return fig

# Example usage
fixed_params = {
    'DIST_PARAM': { '0.800000', '1.000000', '1.200000', '1.500000', '1.400000', '1.600000'},
    # 'NUM_THREADS': {'2', '5', '10', '20', '30', '40', '50', '60', '70'},
    'NUM_THREADS': {'50'},
    'THETA': {'0.000050'},
    # 'PARARLLEL_DESIGN': {'GLOBAL_HASHMAP', 'QPOPSS'},
    'PARARLLEL_DESIGN': {'QPOPSS'},
    # 'HEAVY_QUERY_RATE': {'0.000000', '1.000000', '10.000000'},
    'HEAVY_QUERY_RATE': {'10.000000'},
    # 'ALGORITHM': {'cuckoo_heavy_keeper', 'augmented_sketch', 'count_min', 'heavy_keeper', 'heap_hashmap_space_saving'},
    'ALGORITHM': {'cuckoo_heavy_keeper'},
}

subplot_params = ['DIST_PARAM', 'THETA', 'DIST_PARAM']

base_path = "./experiments_latency_20250226"
analyzer = LatencyExperimentAnalyzer(
    base_path=base_path,
    fixed_params=fixed_params,
    subplot_params=subplot_params
)

# Cache results
cache_file = os.path.join(base_path, 'parallel_latency_data.json')

# if os.path.exists(cache_file):
if False:
    print("Loading cached results...")
    with open(cache_file, 'r') as f:
        results = [dict(r) for r in json.load(f)]
else:
    print("Analyzing experiments...")
    results = analyzer.analyze_latency_experiments()
    
    # Save results to cache
    with open(cache_file, 'w') as f:
        json.dump(results, f)

# Create and save visualization
fig = analyzer.create_latency_visualization(results)
figure_path = os.path.join(base_path, 'figures')
os.makedirs(figure_path, exist_ok=True)
fig.write_image(os.path.join(figure_path, 'par_latency_plot.pdf'))