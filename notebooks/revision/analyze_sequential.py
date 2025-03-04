import os
import re
import plotly.graph_objects as go
from plotly.subplots import make_subplots
from typing import List, Dict, Set, Any
import numpy as np
import copy
import json
import plotly.io as pio   
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from matplotlib.ticker import MaxNLocator, LinearLocator, FixedLocator
from matplotlib.ticker import FormatStrFormatter, ScalarFormatter



pio.kaleido.scope.mathjax = None

def load_material_colors(filepath='material-colors.json'):
    """Load material design colors from JSON file."""
    with open(filepath, 'r') as f:
        colors = json.load(f)
    return colors



class SequentialExperimentAnalyzer:
    # Regular expression pattern from SequentialExperiment
    RESULT_PATTERN = r'RESULT_SUMMARY: FrequencyEstimator=(\w+(?:\s*\w+)*) TotalHeavyHitters=(\d+) TotalHeavyHitterCandidates=(\d+) Precision=([\d.e-]+) Recall=([\d.e-]+) F1Score=([\d.e-]+) ARE=([\d.e-]+) AAE=([\d.e-]+) ExecutionTime=([\d.e-]+) Throughput=([\d.e-]+)'
    
    def __init__(self, base_path: str, fixed_params: Dict[str, Set[str]], 
                 default_params: Dict[str, str], varying_params: List[str]):
        """
        Initialize the analyzer with configuration.
        
        Args:
            base_path: Path to experiments folder
            fixed_params: Dict of parameters to filter by
            default_params: Dict of default parameter values
            varying_params: List of parameters that should vary
        """
        self.base_path = base_path
        self.fixed_params = fixed_params
        self.default_params = default_params
        self.varying_params = varying_params
        # self.metrics = ['Precision', 'Recall', 'F1Score', 'ARE', 'AAE', 
        #                'ExecutionTime', 'Throughput']
      
        self.metrics = ['Precision', 'Recall', 'ARE', 'Throughput']

    def parse_experiment_path(self, path: str) -> Dict[str, str]:
        """Extract experiment parameters from path."""
        params = {}
        parts = path.split(os.sep)
        for part in parts:
            if '=' in part:
                key, value = part.split('=')
                params[key] = value
        return params

    def read_log_file(self, file_path: str) -> List[Dict[str, Any]]:
        """Read and parse a log file."""
        results = []
        with open(file_path, 'r') as f:
            for line in f:
                if match := re.search(self.RESULT_PATTERN, line):
                    results.append({
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
        return results

    def matches_params(self, params: Dict[str, str], target_params: Dict[str, Set[str]]) -> bool:
        """Check if experiment parameters match the target parameters."""
        for key, values in target_params.items():
            if key in params and params[key] not in values:
                return False
        return True

    def analyze_experiments(self) -> Dict[str, List[Dict[str, Any]]]:
        """Analyze all experiments and prepare visualization data."""
        results_by_param = {param: [] for param in self.varying_params}
        
        for param in self.varying_params:
            # For each varying parameter, fix all others to their default values
            current_fixed_params = self.fixed_params.copy()
            for other_param in self.varying_params:
                if other_param != param:
                    current_fixed_params[other_param] = {self.default_params[other_param]}
            
            # Walk through the experiment directory
            for root, _, files in os.walk(self.base_path):
                if any(f.endswith('.log') for f in files):
                    params = self.parse_experiment_path(root)
                    
                    if not self.matches_params(params, current_fixed_params):
                        continue
                    
                    # Read all log files in the directory
                    for file in files:
                        if file.endswith('.log'):
                            file_results = self.read_log_file(os.path.join(root, file))
                            for result in file_results:
                                result.update(params)  # Add path parameters to results
                                results_by_param[param].append(result)
        
        return results_by_param

    def save_metric_figures(self, results_by_param: Dict[str, List[Dict[str, Any]]]):
        """Save individual figures for each metric to the figures subfolder."""
        figure_path = os.path.join(self.base_path, 'figures')
        os.makedirs(figure_path, exist_ok=True)
        
        # Process each metric separately
        for metric in self.metrics:
            # Create temporary metric list with just this metric
            temp_metrics = [metric]
            
            # Use existing create_visualization but with single metric
            temp_analyzer = copy.deepcopy(self)
            temp_analyzer.metrics = temp_metrics
            
            # Get the figure for this metric
            metric_fig = temp_analyzer.create_visualization(results_by_param)
            
            # Update layout for single row
            metric_fig.update_layout(
                height=400,  # Single row height
                width=500 * len(self.varying_params),
                title_text=f"{metric} Analysis"
            )
            
            # Save figures
            # metric_fig.write_html(os.path.join(figure_path, f'{metric}_plot.html'))
            # metric_fig.write_image(os.path.join(figure_path, f'{metric}_plot.png'))
            metric_fig.write_image(os.path.join(figure_path, f'{metric}_plot.pdf'))
         
    def create_visualization(self, results_by_param: Dict[str, List[Dict[str, Any]]]) -> go.Figure:
        """Create interactive plotly visualization with clustered bar plots."""
        
        # Define parameter display names
        param_display_names = {
            'DIST_PARAM': 'Skewness',
            'BASE_UNIT': 'Memory(KB)',
            'THETA': 'φ'
        }
        
        # Create subplots for each metric and varying parameter combination
        fig = make_subplots(
            rows=len(self.metrics),
            cols=len(self.varying_params),
            subplot_titles=[f"{metric} vs {param_display_names[param]}" 
                            for metric in self.metrics 
                            for param in self.varying_params],
            vertical_spacing=0.05,
            horizontal_spacing=0.12
        )
        
        # Define colors for algorithms
        material_colors = load_material_colors("/home/vinh/Q32024/CuckooHeavyKeeper/notebooks/material-colors.json")
        colors = {
            'CountMinSketch': material_colors['green']["500"], 
            'AugmentedSketch': material_colors['orange']["500"],  
            'HeapHashMapSpaceSavingV2': material_colors['blue']["500"],  
            'HeavyKeeper': material_colors['red']["500"],  
            'CuckooHeavyKeeper': material_colors['purple']["700"]  
        }
        
        algorithm_display_names = {
            'CountMinSketch': 'CountMinSketch', 
            'HeapHashMapSpaceSavingV2': 'SpaceSaving',  
            'HeavyKeeper': 'HeavyKeeper',  
            'AugmentedSketch': 'AugmentedSketch',  
            'CuckooHeavyKeeper': 'CuckooHeavyKeeper'
        }
        
        # ensure CHK is always the last algorithm
        algorithms = ['CountMinSketch', 'AugmentedSketch', 'HeapHashMapSpaceSavingV2', 'HeavyKeeper', 'CuckooHeavyKeeper']
        
        metric_transforms = {
            'ARE': lambda x: np.log10(x),
            'AAE': lambda x: np.log10(x),
            # 'ARE': lambda x: x,
            # 'AAE': lambda x: x,
            'ExecutionTime': lambda x: x,
            'Throughput': lambda x: x,
            'Precision': lambda x: x,
            'Recall': lambda x: x,
            'F1Score': lambda x: x
        }
    
        metric_display_names = {
            'ARE': 'log10(ARE)',
            'AAE': 'log10(AAE)',
            # 'ARE': 'ARE',
            # 'AAE': 'AAE',
            'ExecutionTime': 'ExecutionTime',
            'Throughput': 'Throughput',
            'Precision': 'Precision',
            'Recall': 'Recall',
            'F1Score': 'F1Score'
        }
        
        
        # Process each metric and parameter combination
        for metric_idx, metric in enumerate(self.metrics, 1):
            for param_idx, (param, results) in enumerate(results_by_param.items(), 1):
                # Group results by parameter value
                param_values = sorted(set(r[param] for r in results), key=float)
                # algorithms = sorted(set(r['FrequencyEstimator'] for r in results))
                
                for alg_idx, algorithm in enumerate(algorithms):
                    # Collect data for each parameter value
                    x_vals = []
                    y_vals = []
                    err_vals = []
                    
                    for val in param_values:
                        matching_results = [r[metric] for r in results 
                                        if r[param] == val 
                                        and r['FrequencyEstimator'] == algorithm]
                        
                        if matching_results:
                            x_vals.append(val)
                            # y_vals.append(np.mean(matching_results))
                            # err_vals.append(np.std(matching_results))
                            y_vals.append(metric_transforms[metric](np.mean(matching_results)))
                            err_vals.append(metric_transforms[metric](np.std(matching_results)))
                    
                    # Add trace for this algorithm with legendgroup
                    fig.add_trace(
                        go.Bar(
                            name=algorithm_display_names[algorithm],
                            x=x_vals,
                            y=y_vals,
                            # error_y=dict(
                            #     type='data',
                            #     array=err_vals,
                            #     visible=True,
                            #     color='rgba(0,0,0,0.5)',
                            #     thickness=1,
                            #     width=3
                            # ),
                            error_y=None,
                            marker_color=colors.get(algorithm, '#808080'),
                            legendgroup=algorithm_display_names[algorithm],  # Add this line
                            showlegend=(metric_idx == 1 and param_idx == 1),  # Only show in legend once
                            # legendgrouptitle_text=algorithm_display_names[algorithm], if (metric_idx == 1 and param_idx == 1) else None
                        ),
                        row=metric_idx,
                        col=param_idx
                    )
                
                # Update axes
                fig.update_xaxes(
                    title_text=param_display_names[param],
                    tickmode='array',
                    ticktext=[str(val) for val in param_values],
                    tickvals=param_values,
                    gridcolor='lightgray',
                    showgrid=False,
                    row=metric_idx,
                    col=param_idx
                )
                fig.update_yaxes(
                    title_text=metric_display_names[metric] if param_idx == 1 else None,
                    gridcolor='lightgray',
                    showgrid=False,
                    row=metric_idx,
                    col=param_idx
                )
        
        # Update layout
        fig.update_layout(
            height=250 * len(self.metrics),
            width=500 * len(self.varying_params),
            title_text="Sequential Experiment Analysis",
            template="plotly_white",
            plot_bgcolor='white',
            paper_bgcolor='white',
            barmode='group',  # This enables proper bar clustering
            bargap=0.15,      # Gap between bars in the same cluster
            bargroupgap=0,  # Gap between different clusters
            showlegend=True,
            legend=dict(
                yanchor="top",
                y=0.99,
                xanchor="left",
                x=1.01
            ),
            font=dict(size=12)
        )
        
        return fig

    def save_metric_figures_2(self, results_by_param: Dict[str, List[Dict[str, Any]]]):
        """Save individual figures for each metric-parameter combination."""
        figure_path = os.path.join(self.base_path, 'figures')
        os.makedirs(figure_path, exist_ok=True)
        
        param_display_names = {
            'DIST_PARAM': 'Skewness',
            'BASE_UNIT': 'Memory size (KB)',
            'THETA': 'phi'
        }
        
        material_colors = load_material_colors("/home/vinh/Q32024/CuckooHeavyKeeper/notebooks/material-colors.json")
        colors = {
            'CountMinSketch': material_colors['green']["500"], 
            'AugmentedSketch': material_colors['orange']["500"],
            'HeapHashMapSpaceSavingV2': material_colors['blue']["500"],
            'HeavyKeeper': material_colors['red']["500"],
            'CuckooHeavyKeeper': material_colors['purple']["700"]
        }
        
        algorithm_display_names = {
            'CountMinSketch': 'CMS',
            'HeapHashMapSpaceSavingV2': 'SS',
            'HeavyKeeper': 'HK',
            'AugmentedSketch': 'AS',
            'CuckooHeavyKeeper': 'CHK'
        }
        
        pattern_styles = {
            'CountMinSketch': '/',      # Forward slash pattern
            'AugmentedSketch': '\\',    # Backward slash pattern  
            'HeapHashMapSpaceSavingV2': '-',  # Cross pattern
            'HeavyKeeper': '+',         # Plus pattern
            'CuckooHeavyKeeper': 'x'    # Dot pattern
        }
        
        algorithms = ['CountMinSketch', 'AugmentedSketch', 'HeapHashMapSpaceSavingV2', 'HeavyKeeper', 'CuckooHeavyKeeper']
        
        metric_transforms = {
            'ARE': lambda x: np.log10(x) if x > 0 else -6,
            'AAE': lambda x: np.log10(x) if x > 0 else -6,
            'ExecutionTime': lambda x: x,
            'Throughput': lambda x: x/1000000,
            'Precision': lambda x: x if x > 0.03 else 0.03 + x,
            'Recall': lambda x: x if x > 0.03 else 0.03 + x,
            'F1Score': lambda x: x if x > 0.03 else 0.03 + x
        }
        
        metric_formats = {
            'ARE': '.2e',
            'AAE': '.2e',
            'ExecutionTime': '.1f',
            'Throughput': '.1f',
            'Precision': '.3f',
            'Recall': '.3f',
            'F1Score': '.3f'
        }
        
        metric_display_names = {
            'ARE': 'log<sub>10</sub>(ARE)',
            'AAE': 'log<sub>10</sub>(AAE)',
            'ExecutionTime': 'Execution Time (s)',
            'Throughput': 'Throughput',
            'Precision': 'Precision',
            'Recall': 'Recall',
            'F1Score': 'F1 Score'
        }

        for metric in self.metrics:
            for param in self.varying_params:
                fig = go.Figure()
                
                param_results = results_by_param[param]
                param_values = sorted(set(r[param] for r in param_results), key=float)
                
                for algorithm in algorithms:
                    x_vals = []
                    y_vals = []
                    
                    for val in param_values:
                        matching_results = [r[metric] for r in param_results 
                                        if r[param] == val 
                                        and r['FrequencyEstimator'] == algorithm]
                        
                        if matching_results:
                            x_vals.append(val)
                            y_vals.append(metric_transforms[metric](np.mean(matching_results)))
                    
                    fig.add_trace(
                        go.Bar(
                            name=algorithm_display_names[algorithm],
                            x=x_vals,
                            y=y_vals,
                            marker=dict(
                                color=colors[algorithm],
                                pattern=dict(
                                    shape=pattern_styles[algorithm],
                                    size=8,  # Smaller size for thinner lines
                                    solidity=0.15,  # Lower solidity for lighter pattern
                                    fgopacity=1,
                                    fillmode='overlay',
                                    fgcolor='rgb(0,0,0)'  # Make pattern lines black

                                )
                            )
                            # text=[f'{y:{metric_formats[metric]}}' for y in y_vals],
                            # textposition='outside',  # Place text above bars
                            # width=0.18,  # Adjust bar width
                            # textfont=dict(
                            #     family='Times New Roman',
                            #     size=16
                            # )
                        )
                    )
                
                    fig.update_layout(
                      height=300,
                      width=540,
                      xaxis_title=dict(
                      text=param_display_names[param],
                      font=dict(size=16, color='black')
                      ),
                      yaxis_title=dict(
                      text=metric_display_names[metric],
                      font=dict(size=16, color='black')
                      ),
                      template="plotly_white",
                      plot_bgcolor='white',
                      paper_bgcolor='white',
                      barmode='group',
                      showlegend=True,
                      legend=dict(
                      orientation="h",
                      yanchor="top",    
                      y=1.2,           
                      xanchor="center",
                      x=0.5,
                      font=dict(
                      family='serif',
                      color='black',
                      size=16
                      ),
                      bgcolor='rgba(255, 255, 255, 0)',
                      ),
                      margin=dict(
                      l=5,    
                      r=5,    
                      t=25,    
                      b=5,    
                      pad=0   
                      ),
                      font_family='serif',
                      xaxis=dict(tickfont=dict(size=16, color='black')),  # Add x-axis tick font size
                      yaxis=dict(tickfont=dict(size=16, color='black'))   # Add y-axis tick font size
                    )
                    
                
                
                # Add border by updating axes
                fig.update_xaxes(showline=True, linewidth=0.1, linecolor='black', mirror=True)
                fig.update_yaxes(showline=True, linewidth=0.1, linecolor='black', mirror=True)
          
                filename = f"{metric.lower()}_vs_{param.lower()}"
                fig.write_image(os.path.join(figure_path, f'seq_{filename}.pdf'))

    def save_metric_figures_3(self, results_by_param: Dict[str, List[Dict[str, Any]]]):
      """
      Save individual line chart figures for each metric-parameter combination using Matplotlib.
      Each algorithm gets its own decoration, axes have a solid border without extra dashes,
      tick labels are placed close to the border, and a horizontal legend with abbreviations is
      placed at the top of the figure.
      """
      # Font configuration dictionary
      font_config = {
          'family': 'serif',
          'title_size': 10,
          'label_size': 8,
          'label_size_combined': 10,
          'tick_size': 8,
          'tick_size_combined': 8,
          'legend_size': 8,
          'annotation_size': 8,
          'offset_size': 8
      }
      
      material_colors = load_material_colors("/home/vinh/Q32024/CuckooHeavyKeeper/notebooks/material-colors.json")
      figure_path = os.path.join(self.base_path, 'figures')
      os.makedirs(figure_path, exist_ok=True)
      
      # Set font to serif for all text elements
      plt.rcParams['font.family'] = font_config['family']
      
      # Define display names for parameters and metrics.
      param_display_names = {
          'DIST_PARAM': 'Skewness',
          'BASE_UNIT': 'Memory(KB)',
          'THETA': 'φ'
      }
      metric_display_names = {
          'ARE': 'log10(ARE)',
          'AAE': 'log10(AAE)',
          'ExecutionTime': 'Execution Time (s)',
          'Throughput': 'Throughput',
          'Precision': 'Precision',
          'Recall': 'Recall',
          'F1Score': 'F1 Score'
      }
      
      # Define decorations per algorithm: (color, linestyle, marker)
      decorations = {
          'CountMinSketch': ('green', '-', 'o'),
          'AugmentedSketch': ('orange', '--', 's'),
          'HeapHashMapSpaceSavingV2': ('blue', '-.', '^'),
          'HeavyKeeper': ('red', ':', 'D'),
          'CuckooHeavyKeeper': ('purple', '-', 'x')
      }
      # Abbreviated names for the legend.
      algorithm_display_names = {
          'CountMinSketch': 'CMS',
          'AugmentedSketch': 'AS',
          'HeapHashMapSpaceSavingV2': 'SS',
          'HeavyKeeper': 'HK',
          'CuckooHeavyKeeper': 'CHK'
      }
      algorithms = ['CountMinSketch', 'AugmentedSketch', 'HeapHashMapSpaceSavingV2', 'HeavyKeeper', 'CuckooHeavyKeeper']
      
      # Define metric transform functions.
      metric_transforms = {
          'ARE': lambda x: np.log10(x) if x > 0 else -6,
          'AAE': lambda x: np.log10(x) if x > 0 else -6,
          'ExecutionTime': lambda x: x,
          'Throughput': lambda x: x,
          'Precision': lambda x: x,
          'Recall': lambda x: x,
          'F1Score': lambda x: x
      }
      
      # Loop over each metric and each varying parameter.
      for metric in self.metrics:
          for param in self.varying_params:
              # Increase figure height to accommodate legend
              fig, ax = plt.subplots(figsize=(3, 1.7))
              
              # Get results for the current varying parameter.
              param_results = results_by_param[param]
              # Get sorted unique parameter values (as floats for proper ordering).
              param_values = sorted({float(r[param]) for r in param_results})
              
              for alg in algorithms:
                  x_vals = []
                  y_vals = []
                  # For each parameter value, collect the corresponding metric value for this algorithm.
                  for val in param_values:
                      matching_results = [r[metric] for r in param_results 
                                          if float(r[param]) == val and r['FrequencyEstimator'] == alg]
                      if matching_results:
                          x_vals.append(val)
                          y_mean = np.mean(matching_results)
                          y_vals.append(metric_transforms[metric](y_mean))
                  
                  if x_vals and y_vals:
                      color, linestyle, marker = decorations.get(alg, ('gray', '-', 'o'))
                      ax.plot(x_vals, y_vals,
                              label=algorithm_display_names.get(alg, alg),
                              color=material_colors[color]["500"], linestyle=linestyle, marker=marker,
                              markerfacecolor='none',
                              linewidth=2, markersize=6)
                      
              
              # Customize axes: add a solid border and adjust tick parameters.
              for spine in ax.spines.values():
                  spine.set_visible(True)
                  spine.set_linewidth(1.5)
                  spine.set_color('black')
              
              # Set axis labels and title.
              ax.set_xlabel(param_display_names.get(param, param), 
                          fontsize=font_config['label_size'], labelpad=0.1, 
                          fontfamily=font_config['family'])
              ax.set_ylabel(metric_display_names.get(metric, metric), 
                          fontsize=font_config['label_size'], labelpad=0.1, 
                          fontfamily=font_config['family'])
              ax.set_title(f"{metric_display_names.get(metric, metric)} vs {param_display_names.get(param, param)}", 
                          fontsize=font_config['title_size'], pad=18, 
                          fontfamily=font_config['family'])
              ax.tick_params(axis='both', which='both', direction='in', pad=2, 
                          labelsize=font_config['tick_size'])
              
              # Place a horizontal legend on top with more space
              ax.legend(loc='upper center',
                        bbox_to_anchor=(0.5, 1.3),
                        ncol=len(algorithms),
                        fontsize=font_config['legend_size'],
                        prop={'family': font_config['family']},
                        frameon=False)
              
              # Add more top margin to prevent overlap
              fig.tight_layout()
              filename = f"line_{metric.lower()}_vs_{param.lower()}.pdf"
              fig.savefig(os.path.join(figure_path, filename), format='pdf')
              plt.close(fig)

      # Create combined figure
      n_metrics = len(self.metrics)
      n_params = len(self.varying_params)
      n_plots = n_metrics * n_params
      n_cols = 2
      n_rows = (n_plots + n_cols - 1) // n_cols

      fig_combined = plt.figure(figsize=(5.6, 1.34 * n_rows))
      gs = fig_combined.add_gridspec(n_rows, n_cols)
      plot_idx = 0

       
      for metric in self.metrics:
          for param in self.varying_params:
              row = plot_idx // n_cols
              col = plot_idx % n_cols
              ax = fig_combined.add_subplot(gs[row, col])
              
              param_results = results_by_param[param]
              param_values = sorted({float(r[param]) for r in param_results})
              
              for alg in algorithms:
                  x_vals = []
                  y_vals = []
                  for val in param_values:
                      matching_results = [r[metric] for r in param_results 
                                      if float(r[param]) == val and r['FrequencyEstimator'] == alg]
                      if matching_results:
                          x_vals.append(val)
                          y_mean = np.mean(matching_results)
                          y_vals.append(metric_transforms[metric](y_mean))
                  
                  if x_vals and y_vals:
                      color, linestyle, marker = decorations.get(alg, ('gray', '-', 'o'))
                      ax.plot(x_vals, y_vals,
                              label=algorithm_display_names.get(alg, alg),
                              markerfacecolor='none',
                              color=material_colors[color]["500"], linestyle=linestyle, marker=marker,
                              linewidth=2, markersize=4)
              
              # Style the subplot
              for spine in ax.spines.values():
                  spine.set_visible(True)
                  spine.set_linewidth(0.1)
                  spine.set_color('black')
              
              ax.set_xlabel(param_display_names.get(param, param), 
                          fontsize=font_config['label_size_combined'], labelpad=0.1, 
                          fontfamily=font_config['family'])
              ax.set_ylabel(metric_display_names.get(metric, metric), 
                          fontsize=font_config['label_size_combined'], labelpad=0.1, 
                          fontfamily=font_config['family'])
              ax.tick_params(axis='both', which='both', direction='in', pad=2, 
                          labelsize=font_config['tick_size_combined'])
              ax.yaxis.set_major_locator(MaxNLocator(nbins=4, min_n_ticks=4))
              
              if param == 'THETA' and min(x_vals) < 0.001:  # Check if we need scientific notation
                # Force display of all theta values with correct scaling
                ax.xaxis.set_major_locator(FixedLocator([0.0001, 0.0005, 0.001, 0.005]))
                
                # Hide the default offset text
                ax.xaxis.offsetText.set_visible(False)
                
                # Add the multiplier at the bottom right
                ax.text(1, -0.24, '×$10^{-4}$', 
                        transform=ax.transAxes,
                        ha='right', va='center',
                        fontsize=font_config['annotation_size'], 
                        fontfamily=font_config['family'])
                
                # Custom formatter to show values properly scaled
                def custom_formatter(x, pos):
                    if x == 0.0001:
                        return '1'
                    elif x == 0.0005:
                        return '5'
                    elif x == 0.001:
                        return '10'
                    elif x == 0.005:
                        return '50'
                    else:
                        return f'{x*1000:.1f}'  # For any other values
                
                ax.xaxis.set_major_formatter(ticker.FuncFormatter(custom_formatter))
              else:
                ax.xaxis.set_major_locator(FixedLocator(x_vals))  # Show all x values
                  
              ax.yaxis.set_major_locator(MaxNLocator(nbins=4, min_n_ticks=4))
              if max(y_vals) > 1e6:  # Check if we need scientific notation for y-axis
                  ax.yaxis.set_major_formatter(ScalarFormatter(useMathText=True))
                  ax.ticklabel_format(style='sci', axis='y', scilimits=(0,0))
                  ax.yaxis.offsetText.set_fontsize(font_config['offset_size'])
                
              ax.grid(True, 
                color='gray',
                alpha=0.2,  # Transparency
                linestyle='-',
                linewidth=0.1,
                axis='y',
                zorder=0)  # Place grid behind plots
              
              plot_idx += 1

      # Add single legend at the top
      handles, labels = ax.get_legend_handles_labels()
      fig_combined.legend(handles, labels,
                        loc='upper center',
                        bbox_to_anchor=(0.5, 0.95),
                        prop={'family': font_config['family']},
                        ncol=5,
                        fontsize=font_config['legend_size'],
                        frameon=False,
                        handlelength=1.5,
                        handletextpad=0,
                        columnspacing=0.5)

      plt.tight_layout(pad=0.0, h_pad=0.1, w_pad=0)
      fig_combined.subplots_adjust(top=0.89, wspace=0.18)  # Added wspace for consistent column spacing
      combined_filename = "combined_metrics.pdf"
      fig_combined.savefig(os.path.join(figure_path, combined_filename), format='pdf', 
                          bbox_inches='tight', pad_inches=0.03, dpi=800)
      plt.close(fig_combined)
# Example usage:

# base_path = "../experiments/sequential"
base_path = "./experiments_20250218/sequential/DATASET=CAIDA_L"
fixed_params = {
    # 'DIST_PARAM': {'0.8', '1', '1.2', '1.4', '1.6'},
    # 'DATASET': {'CAIDA_L'},
    'BASE_UNIT': {'2', '4', '8', '16', '32'},
    'THETA': {'0.001', '0.005', '0.0005', '0.0001'}
}
default_params = {
    # 'DIST_PARAM': '1.2',
    # 'DATASET': 'CAIDA_H',
    'BASE_UNIT': '4',
    'THETA': '0.0005'
}
# varying_params = ['DIST_PARAM', 'BASE_UNIT', 'THETA']
# varying_params = ['THETA', 'BASE_UNIT', 'DIST_PARAM']
varying_params = ['THETA', 'BASE_UNIT']

# Create analyzer instance
analyzer = SequentialExperimentAnalyzer(
    base_path=base_path,
    fixed_params=fixed_params,
    default_params=default_params,
    varying_params=varying_params
)

# Analyze experiments
# Path for cached results
cache_file = os.path.join(base_path, 'processed_data.json')

# Check if cached results exist
if os.path.exists(cache_file):
  print("Loading cached results...")
  with open(cache_file, 'r') as f:
    # Convert string keys back to proper format when loading
    results = {k: [dict(r) for r in v] for k, v in json.load(f).items()}
else:
  print("Analyzing experiments...")
  results = analyzer.analyze_experiments()
  
  # Save results to cache
  with open(cache_file, 'w') as f:
    json.dump(results, f)

# Save metric figures
analyzer.save_metric_figures_3(results)