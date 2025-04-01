import os
import re
import numpy as np
import json
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
from typing import List, Dict, Set, Any
from matplotlib.ticker import MaxNLocator, LinearLocator, FixedLocator
from matplotlib.ticker import FormatStrFormatter, ScalarFormatter
import matplotlib.font_manager as fm


def load_material_colors(filepath='material-colors.json'):
    """Load material design colors from JSON file."""
    with open(filepath, 'r') as f:
        colors = json.load(f)
    return colors

class ThroughputExperimentAnalyzer:
    def __init__(self, base_paths: List[str], fixed_params: Dict[str, Set[str]], machine_names: List[str] = None):
        self.base_paths = base_paths
        self.machine_names = machine_names if machine_names else [f"Machine {i+1}" for i in range(len(base_paths))]
        self.fixed_params = fixed_params
        self.varying_params = ['PARARLLEL_DESIGN', 'HEAVY_QUERY_RATE', 'ALGORITHM']
        
        # Load material colors
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

    def extract_throughput(self, file_content: str) -> float:
        """Extract throughput value from file content."""
        pattern = r"Throughput: (\d+\.\d+)"
        match = re.search(pattern, file_content)
        if match:
            return float(match.group(1))
        return 0.0

    def matches_fixed_params(self, params: Dict[str, str]) -> bool:
        """Check if experiment parameters match the fixed parameters."""
        for key, values in self.fixed_params.items():
            if key in params and params[key] not in values:
                return False
        return True
    
    def read_throughput_file(self, folder_path: str) -> List[float]:
        """Read all delegation files and extract throughput values."""
        throughputs = []
        for filename in os.listdir(folder_path):
            if filename.endswith('_delegation.json'):
                file_path = os.path.join(folder_path, filename)
                with open(file_path, 'r') as f:
                    content = f.read()
                    throughput = self.extract_throughput(content)
                    if throughput > 0:
                        throughputs.append(throughput)
        return throughputs

    def analyze_throughput_experiments(self) -> Dict[str, List[Dict[str, Any]]]:
        """Analyze all experiments and prepare visualization data for multiple machines."""
        results_by_machine = {}
        
        for idx, base_path in enumerate(self.base_paths):
            machine_name = self.machine_names[idx]
            results = []
            
            for root, dirs, files in os.walk(base_path):
                delegation_files = [f for f in files if f.endswith('_delegation.json')]
                if delegation_files:  # If folder contains delegation files
                    params = self.parse_experiment_path(root)
                    
                    if not self.matches_fixed_params(params):
                        continue
                        
                    # Get all throughput values for this experiment
                    throughputs = self.read_throughput_file(root)
                    
                    if throughputs:  # Only include if we have valid throughput values
                        # Calculate average throughput
                        avg_throughput = sum(throughputs) / len(throughputs)
                        params['throughput'] = avg_throughput
                        params['num_runs'] = len(throughputs)
                        results.append(params)
            
            results_by_machine[machine_name] = results
            
            # Save cache for each machine
            cache_file = os.path.join(base_path, 'parallel_throughput_data.json')
            with open(cache_file, 'w') as f:
                json.dump(results, f)
        
        return results_by_machine
    
    def create_throughput_visualization_matplotlib(self, results_by_machine: Dict[str, List[Dict[str, Any]]]) -> None:
        """Create throughput visualization for multiple machines using Matplotlib."""
        material_colors = load_material_colors("./notebooks/material-colors.json")
        
        # Get the notebooks directory (one level up from the revision directory)
        notebooks_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

        # Reference fonts relative to the notebooks directory
        font_path = os.path.join(notebooks_dir, "fonts", "LinLibertine_R.ttf")
        fm.fontManager.addfont(font_path)

        # Also add the bold variant for titles
        bold_font_path = os.path.join(notebooks_dir, "fonts", "LinLibertine_RB.ttf")
        fm.fontManager.addfont(bold_font_path)
        
        # Font configuration dictionary
        font_config = {
            # 'family': 'serif',
            'family': 'Linux Libertine',
            'title_size': 12,
            'label_size': 12,
            'tick_size': 10, 
            'annotation_size': 8,
            'legend_size': 8,
            'machine_name_size': 12,
            'offset_size': 10
        }
        plt.rcParams['font.family'] = font_config['family']
        plt.rcParams['font.serif'] = ['Linux Libertine']


        # Define query rates with corrected labels
        query_rates = ['0.000000', '1.000000', '10.000000']
        query_rate_labels = {'0.000000': '0%', '1.000000': '0.01%', '10.000000': '0.1%'}
        
        # Algorithm display names and decorations
        algorithm_display_names = {
            'cuckoo_heavy_keeper': 'CHK',
            'augmented_sketch': 'AS',
            'count_min': 'CMS',
            'heavy_keeper': 'HK',
            'heap_hashmap_space_saving': 'SS'
        }
        
        # Define decorations: (color, linestyle, marker)
        algorithm_decorations = {
            'cuckoo_heavy_keeper': ('purple', '-', 'x'),
            'augmented_sketch': ('orange', '--', 's'),
            'count_min': ('green', '-', 'o'),
            'heavy_keeper': ('red', ':', 'D'),
            'heap_hashmap_space_saving': ('blue', '-.', '^')
        }
        
        # Design variations: Different line styles
        design_line_styles = {
            'GLOBAL_HASHMAP': '-',  # solid
            'QPOPSS': '--'          # dashed
        }
        
        # Set font to serif for all text elements
        plt.rcParams['font.family'] = font_config['family']
        
        # Create figure with 3x2 subplots (3 rows for query rates, 2 columns for machines)
        fig, axs = plt.subplots(3, 2, figsize=(5.6, 4.55), sharex=False)
        
        # Make sure to include all thread values including 2 and 5
        thread_values = sorted([int(x) for x in self.fixed_params['NUM_THREADS']])
        
        # Process each machine (column)
        for machine_idx, (machine_name, results) in enumerate(results_by_machine.items()):
            
            # Process each query rate (row)
            for row_idx, query_rate in enumerate(query_rates):
                ax = axs[row_idx, machine_idx]
                speedup_annotations = []
                
                # Process each algorithm
                for alg in sorted(set(r['ALGORITHM'] for r in results)):
                    color_name, _, marker = algorithm_decorations[alg]
                    color = material_colors[color_name]["500"]
                    
                    # Process each design type
                    for design in ['GLOBAL_HASHMAP', 'QPOPSS']:
                        design_suffix = 'q' if design == 'GLOBAL_HASHMAP' else 'i'
                        
                        # Filter results for this algorithm, design and query rate
                        matching_results = [r for r in results 
                                        if r['ALGORITHM'] == alg
                                        and r['PARARLLEL_DESIGN'] == design
                                        and r['HEAVY_QUERY_RATE'] == query_rate]
                        
                        if matching_results:
                            # Sort by number of threads
                            sorted_results = sorted(matching_results, key=lambda x: int(x['NUM_THREADS']))
                            
                            x_values = [int(r['NUM_THREADS']) for r in sorted_results]
                            # Values are already in Mops, no need to divide
                            y_values = [float(r['throughput']) *1e6 for r in sorted_results]
                            
                            # Create label with the format m{algoname}-{design}
                            # Only show labels in the legend for the first row, first column
                            label = None
                            if row_idx == 0 and machine_idx == 0:
                                label = f"m{algorithm_display_names[alg]}-{design_suffix}"
                            
                            # Use standard linestyle parameter
                            line = ax.plot(x_values, y_values, 
                                        linestyle=design_line_styles[design], marker=marker, 
                                        markerfacecolor='none', markersize=6,
                                        linewidth=1.5, color=color, 
                                        label=label)
                
                        if matching_results and len(y_values) > 1:
                            first_val = y_values[0]
                            last_val = y_values[-1]
                            if first_val > 0:  # Avoid division by zero
                                speedup = last_val / first_val
                                speedup_annotations.append({
                                    'x': x_values[-1],
                                    'y': y_values[-1],
                                    'speedup': speedup,
                                    'color': color,
                                    'text': f"{speedup:.1f}×",
                                    'algo': f"{algorithm_display_names[alg]}-{design_suffix}"
                                })

                # After processing all algorithms, add staggered speedup annotations
                if speedup_annotations:
                    # Sort annotations by y-value (highest to lowest) instead of speedup
                    speedup_annotations.sort(key=lambda x: x['y'], reverse=True)
                    
                    # Get maximum x value and y range
                    max_x = max(ann['x'] for ann in speedup_annotations)
                    y_range = ax.get_ylim()
                    y_max = y_range[1]
                    y_min = y_range[0]
                    available_height = y_max - y_min
                    
                    # Calculate fixed positions for annotations
                    # Position them in the right 15% of the plot, evenly spaced
                    x_pos = max_x * 1.04  # Horizontal position (slightly right of the plot)
                    
                    # Place annotations with fixed spacing in a neat column
                    annotation_count = len(speedup_annotations)
                    
                    # Calculate spacing to distribute evenly across 80% of the y-axis
                    spacing = (available_height * 0.8) / max(1, annotation_count - 1) if annotation_count > 1 else 0
                    start_y = y_max - (available_height * 0.1)  # Start at 10% from the top
                    
                    # Add all annotations
                    for i, ann in enumerate(speedup_annotations):
                        y_pos = start_y - (i * spacing)
                        
                        # Add annotation without algorithm name, just the speedup value
                        ax.annotate(f"{ann['text']}",  # Removed algorithm name
                                xy=(x_pos, y_pos),
                                xytext=(0, 0),
                                textcoords="offset points",
                                fontsize=font_config['annotation_size'], 
                                fontfamily=font_config['family'],
                                ha='left', va='center',
                                color=ann['color'])
                            
                # Adjust x-axis limits to make room for annotations
                # Find max thread value for this query rate
                max_thread_values = []
                for r in results:
                    if r['HEAVY_QUERY_RATE'] == query_rate:
                        max_thread_values.append(int(r['NUM_THREADS']))
                
                if max_thread_values:
                    x_max = max(max_thread_values)
                    ax.set_xlim(right=x_max * 1.2)
                
                # Set title with HH-Query rate for each subplot
                ax.set_title(f"HH-Query Rate = {query_rate_labels[query_rate]}", 
                        fontsize=font_config['title_size'], fontfamily=font_config['family'], pad=2)
                
                # Set y-axis label for all subplots
                ax.set_ylabel('Throughput', fontsize=font_config['label_size'], 
                            fontfamily=font_config['family'], labelpad=1)
                formatter = ScalarFormatter(useMathText=True)
                formatter.set_powerlimits((7,7))  # Force 10^7
                ax.yaxis.set_major_formatter(formatter)
                ax.yaxis.offsetText.set_fontsize(font_config['offset_size'])
                # ax.yaxis.offsetText.set_position((0, 1.05))
                
                # Style the subplot
                for spine in ax.spines.values():
                    spine.set_visible(True)
                    spine.set_linewidth(0.1)
                    spine.set_color('black')
                
                # Explicitly set x-ticks on all subplots
                ax.set_xticks(thread_values)
                ax.set_xticklabels([str(x) for x in thread_values], 
                                fontsize=font_config['tick_size'])
                ax.tick_params(axis='both', which='both', direction='in', 
                            pad=2, labelsize=font_config['tick_size'])
                ax.yaxis.set_major_locator(MaxNLocator(nbins=4, min_n_ticks=4))
                
                ax.grid(True, color='gray', alpha=0.2, linestyle='-', linewidth=0.1, axis='y', zorder=0)
                
                # Set x-axis label for all subplots
                ax.set_xlabel('Number of Threads', fontsize=font_config['label_size'], 
                            fontfamily=font_config['family'], labelpad=1)
        
        # Add machine names as a super title for each column
        for machine_idx, machine_name in enumerate(results_by_machine.keys()):
            fig.text(0.25 + 0.5*machine_idx, 0.883, machine_name, 
                    ha='center', fontsize=font_config['machine_name_size'], 
                    fontfamily=font_config['family'])
        
        # Organize legend by algorithm pairs (-q and -i versions together)
        handles, labels = axs[0, 0].get_legend_handles_labels()
        
        # Reorganize handles and labels by algorithm
        alg_pairs = {}
        for handle, label in zip(handles, labels):
            # Extract algorithm name (before the -q or -i)
            algo = label.split('-')[0]
            if algo not in alg_pairs:
                alg_pairs[algo] = []
            alg_pairs[algo].append((handle, label))
        
        # Create new handles and labels list organized by algorithm
        new_handles = []
        new_labels = []
        custom_order = ['mCMS', 'mAS', 'mSS', 'mCHK']
        for algo in custom_order:
        # for algo in sorted(alg_pairs.keys()):
            for handle, label in alg_pairs[algo]:
                new_handles.append(handle)
                new_labels.append(label)
                
        # Add legend with organized pairs, positioned higher to avoid overlap
        fig.legend(new_handles, new_labels,
                loc='upper center',
                bbox_to_anchor=(0.5, 1.03),  # Move legend higher
                ncol=len(alg_pairs),  # One column per algorithm
                fontsize=font_config['legend_size'],
                frameon=False,
                handlelength=1.5,
                handletextpad=0.5,
                columnspacing=0.6,
                prop={'family': font_config['family']})
        
        # Adjust layout
        plt.tight_layout(pad=0.0, h_pad=0.0, w_pad=0.0)
        plt.subplots_adjust(top=0.837, wspace=0.18)
        
        # Save figure
        figure_path = os.path.join(self.base_paths[0], 'figures')
        os.makedirs(figure_path, exist_ok=True)
        filename = f"par_throughput_comparison.pdf"
        plt.savefig(os.path.join(figure_path, filename), format='pdf', 
                bbox_inches='tight', pad_inches=0.03, dpi=5000)
        plt.close(fig)


# Define fixed parameters
fixed_params = {
    'DIST_PARAM': {'1.500000'},
    'NUM_THREADS': {'1', '5', '10', '20', '30', '40', '50', '60', '70'},
    # 'NUM_THREADS': {'1', '5', '10', '20', '30', '40', '50', '60', '70', '80', '90', '100', '110', '120'},
    'THETA': {'0.000050'},
    'PARARLLEL_DESIGN': {'GLOBAL_HASHMAP', 'QPOPSS'},
    'EVALUATE_MODE': {'throughput'},
    'HEAVY_QUERY_RATE': {'0.000000', '1.000000', '10.000000'},
    'ALGORITHM': {'cuckoo_heavy_keeper', 'augmented_sketch', 'count_min', 'heavy_keeper', 'heap_hashmap_space_saving'},
}

# Define the base paths to experiments
base_paths = [
    "./experiment_parallel_20250219/experiments_CAIDA_H_20250218",
    "./experiments_throughput_athena_20250303/experiments_throughput_20250303"
]

# machine_names = ["Machine 1", "Machine 2"]
machine_names = ["Platform A", "Platform B"]

# Create analyzer instance
analyzer = ThroughputExperimentAnalyzer(
    base_paths=base_paths,
    fixed_params=fixed_params,
    machine_names=machine_names
)

# Process both machines
results_by_machine = {}

# Try to load from cache first for each machine
for idx, base_path in enumerate(base_paths):
    machine_name = machine_names[idx]
    cache_file = os.path.join(base_path, 'parallel_throughput_data.json')
    
    if os.path.exists(cache_file):
        print(f"Loading cached results for {machine_name}...")
        with open(cache_file, 'r') as f:
            results_by_machine[machine_name] = [dict(r) for r in json.load(f)]
    else:
        print(f"No cached data for {machine_name}, will analyze")

# If any machine doesn't have cached results, analyze all
if len(results_by_machine) < len(base_paths):
    print("Analyzing experiments for all machines...")
    results_by_machine = analyzer.analyze_throughput_experiments()

# Create and save visualization
analyzer.create_throughput_visualization_matplotlib(results_by_machine)