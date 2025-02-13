import os
import re
import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots
from typing import List, Dict, Set, Any
import json

import plotly.io as pio   
pio.kaleido.scope.mathjax = None

def load_material_colors(filepath='material-colors.json'):
    """Load material design colors from JSON file."""
    with open(filepath, 'r') as f:
        colors = json.load(f)
    return colors


class ThroughputExperimentAnalyzer:
    def __init__(self, base_path: str, fixed_params: Dict[str, Set[str]], subplot_params: List[str]):
        self.base_path = base_path
        self.fixed_params = fixed_params
        self.subplot_params = subplot_params
        self.varying_params = [
            'PARALLEL_DESIGN',
            'HEAVY_QUERY_RATE'
        ]
        
        # Load colors once at initialization
        with open('./notebooks/material-colors.json') as f:
            self.material_colors = json.load(f)

    def get_design_style(self, design: str, query_rate: str) -> dict:
        """Returns color by query rate and line pattern by design"""
        query_colors = {
            '0.000000': 'red',
            '1.000000': 'blue', 
            '5.000000': 'yellow',
            '10.000000': 'green'
        }
        
        # Use different symbols in the line pattern
        design_patterns = {
            'GLOBAL_HASHMAP': 'solid',
            'QPOPSS': 'dot'  # will create a dotted line pattern
        }
        
        return {
            'color': self.material_colors[query_colors[query_rate]]['500'],
            'dash': design_patterns[design]
        }
        
    def parse_experiment_path(self, path: str) -> Dict[str, Any]:
        """Extract experiment parameters from path."""
        params = {}
        parts = path.split(os.sep)
        for part in parts:
            if '=' in part:
                key, value = part.split('=')
                params[key] = value
        return params

    def extract_throughput(self, file_content: str) -> float:
        """Extract throughput value from file content."""
        pattern = r"Throughput: (\d+\.\d+)"
        match = re.search(pattern, file_content)
        if match:
            return float(match.group(1))
        return 0.0

    def read_throughput_file(self, folder_path: str) -> float:
        """Read delegation file and extract throughput value."""
        for filename in os.listdir(folder_path):
            if filename.endswith('_delegation.json'):
                file_path = os.path.join(folder_path, filename)
                with open(file_path, 'r') as f:
                    content = f.read()
                    return self.extract_throughput(content)
        return 0.0

    def matches_fixed_params(self, params: Dict[str, str]) -> bool:
        """Check if experiment parameters match the fixed parameters."""
        for key, values in self.fixed_params.items():
            if key in params and params[key] not in values:
                return False
        return True

    def analyze_throughput_experiments(self) -> List[Dict[str, Any]]:
        """Analyze all experiments and prepare visualization data."""
        results = []
        
        for root, dirs, files in os.walk(self.base_path):
            if any(f.endswith('_delegation.json') for f in files):
                params = self.parse_experiment_path(root)
                
                if not self.matches_fixed_params(params):
                    continue
                throughput = self.read_throughput_file(root)
                if throughput > 0:
                    params['throughput'] = throughput
                    results.append(params)
        
        return results

    def create_throughput_visualization(self, results: List[Dict[str, Any]]) -> go.Figure:
        """Create line chart visualization."""
        # Group results by subplot parameters
        grouped_results = {}
        for result in results:
            key_params = tuple(
                (k, v) for k, v in result.items()
                if k in self.subplot_params
            )
            group_key = frozenset(key_params)
            if group_key not in grouped_results:
                grouped_results[group_key] = []
            grouped_results[group_key].append(result)

        # Create subplot for each group
        fig = make_subplots(
            rows=len(grouped_results),
            cols=1,
            subplot_titles=[', '.join(f"{k}={v}" for k, v in dict(key).items())
                          for key in grouped_results.keys()],
            vertical_spacing=0.1
        )

        # Generate distinct colors for combinations of varying parameters
        colors = ['blue', 'red', 'green', 'purple', 'orange', 'brown', 'pink', 'gray']
        param_combinations = {}
        color_idx = 0

        row = 1
        for group_key, group_results in grouped_results.items():
            # Group by PARALLEL_DESIGN and HEAVY_QUERY_RATE
            design_rate_groups = {}
            
            for result in group_results:
                combo = (result['PARARLLEL_DESIGN'], result['HEAVY_QUERY_RATE'])
                if combo not in design_rate_groups:
                    design_rate_groups[combo] = []
                design_rate_groups[combo].append(result)

            for combo, combo_results in design_rate_groups.items():
                if combo not in param_combinations:
                    param_combinations[combo] = colors[color_idx % len(colors)]
                    color_idx += 1

                # Sort by number of threads
                sorted_results = sorted(combo_results, key=lambda x: int(x['NUM_THREADS']))
                
                x_values = [int(r['NUM_THREADS']) for r in sorted_results]
                y_values = [r['throughput'] for r in sorted_results]
                
                name = f"Design={combo[0]}, Query Rate={combo[1]}"
                
                style = self.get_design_style(
                    design=combo[0],     # PARALLEL_DESIGN
                    query_rate=combo[1]  # HEAVY_QUERY_RATE
                )
                
                fig.add_trace(
                    go.Scatter(
                        x=x_values,
                        y=y_values,
                        name=name,
                        mode='lines+markers',
                        # line=dict(color=param_combinations[combo]),
                        line=dict(
                        color=style['color'],
                        dash=style['dash']
                        ),
                        marker=dict(
                            size=10,
                            color=style['color']
                        ),
                        legendgroup=name,
                        showlegend=(row == 1),
                        hovertemplate="Threads: %{x}<br>Throughput: %{y:.2f} Mops<extra></extra>"
                    ),
                    row=row,
                    col=1
                )
            
            row += 1

        # Update layout
        fig.update_layout(
            height=400 * len(grouped_results),
            title_text="Throughput Analysis",
            showlegend=True,
            template="plotly_white",
            legend=dict(
                yanchor="top",
                y=0.99,
                xanchor="left",
                x=1.01
            )
        )
        
        # Update the axes formatting:
        for i in range(len(grouped_results)):
            fig.update_xaxes(
                title_text="Number of Threads",
                row=i+1,
                col=1,
                tickmode='array',
                tickvals=sorted(list(fixed_params['NUM_THREADS']))
            )
            fig.update_yaxes(
                title_text="Throughput (Mops)",
                row=i+1,
                col=1,
                rangemode='tozero',
                tickformat='.0f'
            )

        # Update axes labels
        for i in range(len(grouped_results)):
            fig.update_xaxes(title_text="Number of Threads", row=i+1, col=1)
            fig.update_yaxes(title_text="Throughput (Mops)", row=i+1, col=1)

        return fig

    def create_throughput_visualization_2(self, results: List[Dict[str, Any]]) -> go.Figure:
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
                sorted_results = sorted(matching_results, key=lambda x: x['DIST_PARAM'])
                x_values = [r['DIST_PARAM'] for r in sorted_results]
                y_values = [r['throughput'] for r in sorted_results]
                
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
                sorted_results = sorted(matching_results, key=lambda x: x['DIST_PARAM'])
                x_values = [r['DIST_PARAM'] for r in sorted_results]
                y_values = [r['throughput'] for r in sorted_results]
                
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

        thread_values = sorted(x for x in self.fixed_params['DIST_PARAM'])

        # Configure three legend positions
        legend_positions = [0.2, 0.5, 0.8]  # Left, Center, Right
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
        thread_values = sorted([x for x in self.fixed_params['DIST_PARAM'] if x not in {'2', '5'}])
        fig.update_layout(
            height=300,
            width=640,
            xaxis_title=dict(
                text="Number of Threads",
                font=dict(family='serif', color='black')
            ),
            yaxis_title=dict(
                text="Throughput (Mops)",
                font=dict(family='serif', color='black')
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
                showgrid=False,
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
                zeroline=False,
                showgrid=True  # No vertical grid
            ),
            **legend_configs  # Unpack all legend configurations
        )

        return fig
# Example usage
fixed_params = {
    # 'DIST_PARAM': {'1.200000', '1.500000'},
    # 'DIST_PARAM': { '1.500000'},
    'DIST_PARAM': { '0.800000', '1.000000', '1.200000', '1.400000', '1.600000'},
    'NUM_THREADS': {'2', '5', '10', '20', '30', '40', '50', '60', '70'},
    'THETA': {'0.000050'},
    'PARALLEL_DESIGN': {'GLOBAL_HASHMAP', 'QPOPSS'},
    'EVALUATE_MODE': {'throughput'},
    # 'HEAVY_QUERY_RATE': {'0.000000', '1.000000', '5.000000', '10.000000'}
    'HEAVY_QUERY_RATE': {'0.000000', '1.000000', '10.000000'},
    # 'ALGORITHM': {'cuckoo_heavy_keeper', 'augmented_sketch', 'count_min', 'heavy_keeper', 'heap_hashmap_space_saving'},
    'ALGORITHM': {'heap_hashmap_space_saving'},
    
}

subplot_params = ['DIST_PARAM', 'THETA', 'DIST_PARAM']
base_path="./experiments/parallel_varying_skewness_20250212"
analyzer = ThroughputExperimentAnalyzer(
    base_path=base_path,
    fixed_params=fixed_params,
    subplot_params=subplot_params
)

# results = analyzer.analyze_throughput_experiments()
# fig = analyzer.create_throughput_visualization(results)
# fig.show()

# figure_path = '/home/vinh/Q32024/CuckooHeavyKeeper/throughput_2024_11_10/figures'
# os.makedirs(figure_path, exist_ok=True)
# fig.update_layout(
#                 height=400,  # Single row height
#                 width=1000 ,
#             )
# fig.write_image(os.path.join(figure_path, 'par_throughput_plot.pdf'))

# Cache results
cache_file = os.path.join(base_path, 'parallel_throughput_data.json')

if os.path.exists(cache_file):
    print("Loading cached results...")
    with open(cache_file, 'r') as f:
        results = [dict(r) for r in json.load(f)]
else:
    print("Analyzing experiments...")
    results = analyzer.analyze_throughput_experiments()
    
    # Save results to cache
    with open(cache_file, 'w') as f:
        json.dump(results, f)

# Create and save visualization
fig = analyzer.create_throughput_visualization_2(results)
figure_path = os.path.join(base_path, 'figures')
os.makedirs(figure_path, exist_ok=True)
fig.write_image(os.path.join(figure_path, 'par_throughput_plot.pdf'))