import os
import re
import numpy as np
import json
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
from typing import List, Dict, Set, Any
from mpl_toolkits.axes_grid1.inset_locator import inset_axes, mark_inset

def load_material_colors(filepath='material-colors.json'):
    """Load material design colors from JSON file."""
    with open(filepath, 'r') as f:
        colors = json.load(f)
    return colors

class LatencyExperimentAnalyzer:
    def __init__(self, base_paths: List[str], fixed_params: Dict[str, Set[str]], machine_names: List[str] = None):
        self.base_paths = base_paths
        self.machine_names = machine_names if machine_names else [f"Machine {i+1}" for i in range(len(base_paths))]
        self.fixed_params = fixed_params
        self.varying_params = ['PARARLLEL_DESIGN', 'HEAVY_QUERY_RATE']
        
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

    def analyze_latency_experiments(self) -> Dict[str, List[Dict[str, Any]]]:
        """Analyze all experiments and prepare visualization data for multiple machines."""
        results_by_machine = {}
        
        for idx, base_path in enumerate(self.base_paths):
            machine_name = self.machine_names[idx]
            results = []
            
            for root, dirs, files in os.walk(base_path):
                heavyhitter_files = [f for f in files if f.endswith('_heavyhitter.json')]
                if heavyhitter_files:  # If folder contains heavyhitter files
                    params = self.parse_experiment_path(root)
                    
                    if not self.matches_fixed_params(params):
                        continue
                        
                    # Get all latency values for this experiment
                    latencies = self.read_latency_file(root)
                    
                    if latencies:  # Only include if we have valid latency values
                        # Calculate average latency
                        avg_latency = sum(latencies) / len(latencies)
                        params['latency'] = avg_latency
                        params['num_runs'] = len(latencies)
                        results.append(params)
            
            results_by_machine[machine_name] = results
            
            # Save cache for each machine
            cache_file = os.path.join(base_path, 'parallel_latency_data.json')
            with open(cache_file, 'w') as f:
                json.dump(results, f)
        
        return results_by_machine
    
    def create_latency_visualization_matplotlib(self, results_by_machine: Dict[str, List[Dict[str, Any]]]) -> None:
        """Create latency visualization for multiple machines using Matplotlib."""
        material_colors = load_material_colors("./notebooks/material-colors.json")
        
        # Font configuration dictionary
        font_config = {
            'family': 'serif',
            'title_size': 10,
            'label_size': 10,
            'tick_size': 8,
            'tick_size_inset': 6,
            'tick_label_inset': 6,
            'annotation_size': 6,
            'legend_size': 8,
            'machine_name_size': 10,
            'inset_text_size': 6
        }
        plt.rcParams['font.family'] = font_config['family']
        
        # Define query rates with corrected labels
        query_rates = ['0.000000', '1.000000', '10.000000']
        query_rate_labels = {'0.000000': '0%', '1.000000': '0.01%', '10.000000': '0.1%'}
        
        # Algorithm display names and decorations (similar to throughput)
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
        fig, axs = plt.subplots(3, 2, figsize=(5.6, 4.8), sharex=False)
        
        # Make sure to include all thread values
        thread_values = sorted([int(x) for x in self.fixed_params['NUM_THREADS']])
        
        # Process each machine (column)
        for machine_idx, (machine_name, results) in enumerate(results_by_machine.items()):
            
            # Process each query rate (row)
            for row_idx, query_rate in enumerate(query_rates):
                ax = axs[row_idx, machine_idx]
                
                # Create list to store data for determining zoom range
                all_y_values = []
                high_thread_data = {}
                
                # Process each algorithm
                for alg in sorted(set(r['ALGORITHM'] for r in results)):
                    color_name, _, marker = algorithm_decorations[alg]
                    color = material_colors[color_name]["500"]
                    
                    # Process each design type
                    for design in ['GLOBAL_HASHMAP', 'QPOPSS']:
                        design_suffix = 'q' if design == 'GLOBAL_HASHMAP' else 'i'
                        key = f"{alg}_{design}"
                        high_thread_data[key] = {"x": [], "y": [], "color": color, "style": design_line_styles[design], "marker": marker}
                        
                        # Filter results for this algorithm, design and query rate
                        matching_results = [r for r in results 
                                        if r['ALGORITHM'] == alg
                                        and r['PARARLLEL_DESIGN'] == design
                                        and r['HEAVY_QUERY_RATE'] == query_rate]
                        
                        if matching_results:
                            # Sort by number of threads
                            sorted_results = sorted(matching_results, key=lambda x: int(x['NUM_THREADS']))
                            
                            x_values = [int(r['NUM_THREADS']) for r in sorted_results]
                            y_values = [float(r['latency']) for r in sorted_results]
                            all_y_values.extend(y_values)
                            
                            # Store high thread count data for zoom
                            for x, y in zip(x_values, y_values):
                                if x >= 60:  # Consider threads >= 60 for zoom
                                    high_thread_data[key]["x"].append(x)
                                    high_thread_data[key]["y"].append(y)
                            
                            # Create label with the format m{algoname}-{design}
                            label = None
                            if row_idx == 0 and machine_idx == 0:
                                label = f"m{algorithm_display_names[alg]}-{design_suffix}"
                            
                            # Plot main line
                            line = ax.plot(x_values, y_values, 
                                        linestyle=design_line_styles[design], marker=marker, 
                                        markerfacecolor='none', markersize=6,
                                        linewidth=1.5, color=color, 
                                        label=label)
                
                # Add zoom inset if we have high thread data
                if all_y_values and any(len(data["x"]) > 0 for data in high_thread_data.values()):
                    # Create inset axes in top right corner
                    axins = inset_axes(ax, width="25%", height="35%", 
                                    loc='upper right', borderpad=0.3)
                    
                    # Plot data in the inset
                    for key, data in high_thread_data.items():
                        if data["x"] and data["y"]:
                            # For inset plot, cap values at 300 for better visibility of low latencies
                            capped_y = [min(y, 300) for y in data["y"]]
                            
                            axins.plot(data["x"], capped_y, 
                                    linestyle=data["style"], marker=data["marker"],
                                    markerfacecolor='none', markersize=4,
                                    linewidth=1.2, color=data["color"])
                            
                            # # Add markers for points that were capped
                            # for i, (x, y) in enumerate(zip(data["x"], data["y"])):
                            #     if y > 300:
                            #         axins.scatter(x, 300, marker='^', s=20, color=data["color"], 
                            #                     alpha=0.7, zorder=10)
                    
                    # Set fixed limits for the inset - focus on threads 60-70 and latency 0-300
                    axins.set_xlim(60, 70)
                    axins.set_ylim(0, 300)
                    
                    # Add more yticks to the inset
                    axins.set_yticks([0, 100, 200, 300])
                    axins.set_yticklabels(['0', '100', '200', '300'], fontsize=font_config['tick_label_inset'])

                    # Format the inset
                    axins.tick_params(axis='both', which='both', labelsize=font_config['tick_size_inset'])
                    axins.grid(True, alpha=0.2, linestyle='-', linewidth=0.1)
                    
                    # # Add y-axis label indicating cap
                    # axins.text(0.02, 0.98, "≤300μs", transform=axins.transAxes,
                    #         fontsize=font_config['inset_text_size'], fontfamily=font_config['family'], va='top')
                    
                    # Draw connecting lines between inset and main plot with dotted style
                    mark_inset(ax, axins, loc1=3, loc2=4, fc="none", ec="0.5", lw=0.1, ls=":")
                
                # Set title and other styling (as before)
                ax.set_title(f"HH-Query Rate = {query_rate_labels[query_rate]}", 
                        fontsize=font_config['title_size'], fontfamily=font_config['family'], pad=2)
                
                # Set y-axis label for all subplots
                ax.set_ylabel('Latency (μs)', fontsize=font_config['label_size'], 
                            fontfamily=font_config['family'], labelpad=1)
                
                # Style the subplot
                for spine in ax.spines.values():
                    spine.set_visible(True)
                    spine.set_linewidth(0.1)
                    spine.set_color('black')
                
                # Explicitly set x-ticks on all subplots
                ax.set_xticks(thread_values)
                ax.set_xticklabels([str(x) for x in thread_values], fontsize=font_config['tick_size'])
                ax.tick_params(axis='both', which='both', direction='in', 
                            pad=2, labelsize=font_config['tick_size'])
                ax.yaxis.set_major_locator(MaxNLocator(nbins=4, min_n_ticks=4))
                
                ax.grid(True, color='gray', alpha=0.2, linestyle='-', linewidth=0.1, axis='y', zorder=0)
                
                # Set x-axis label for all subplots
                ax.set_xlabel('Number of Threads', fontsize=font_config['label_size'], 
                            fontfamily=font_config['family'], labelpad=1)
        
        # Add machine names as a super title for each column
        for machine_idx, machine_name in enumerate(results_by_machine.keys()):
            fig.text(0.25 + 0.5*machine_idx, 0.89, machine_name, 
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
        for algo in sorted(alg_pairs.keys()):
            for handle, label in alg_pairs[algo]:
                new_handles.append(handle)
                new_labels.append(label)
                
        # Add legend with organized pairs, positioned higher to avoid overlap
        fig.legend(new_handles, new_labels,
                loc='upper center',
                bbox_to_anchor=(0.5, 1.03),
                ncol=len(alg_pairs),  # One column per algorithm
                fontsize=font_config['legend_size'],
                frameon=False,
                handlelength=1.5,
                handletextpad=0.5,
                columnspacing=0.6,
                prop={'family': font_config['family']})
        
        # Adjust layout
        plt.tight_layout(pad=0.0, h_pad=0.9, w_pad=0.0)
        plt.subplots_adjust(top=0.85, wspace=0.21, hspace=0.56)  # Added wspace for column spacing consistency
        
        # Save figure
        figure_path = os.path.join(self.base_paths[0], 'figures')
        os.makedirs(figure_path, exist_ok=True)
        filename = f"par_latency_comparison_all.pdf"
        plt.savefig(os.path.join(figure_path, filename), format='pdf', 
                bbox_inches='tight', pad_inches=0.03, dpi=2000)
        plt.close(fig)


# Define fixed parameters
fixed_params = {
    'DIST_PARAM': {'1.500000'},  # Filter to just one value for this comparison
    'NUM_THREADS': {'1', '5', '10', '20', '30', '40', '50', '60', '70'},
    'THETA': {'0.000050'},
    'PARARLLEL_DESIGN': {'GLOBAL_HASHMAP', 'QPOPSS'},
    'HEAVY_QUERY_RATE': {'0.000000', '1.000000', '10.000000'},
    'ALGORITHM': {'cuckoo_heavy_keeper', 'augmented_sketch', 'count_min', 'heavy_keeper', 'heap_hashmap_space_saving'}
}

# Define the base paths to experiments
base_paths = [
    "./experiments_latency_20250303",
    "./experiment_latency_athena_20250228/experiments_latency_20250228",
]

machine_names = ["Machine 1", "Machine 2"]

# Create analyzer instance
analyzer = LatencyExperimentAnalyzer(
    base_paths=base_paths,
    fixed_params=fixed_params,
    machine_names=machine_names
)

# Process both machines
results_by_machine = {}

# Try to load from cache first for each machine
for idx, base_path in enumerate(base_paths):
    machine_name = machine_names[idx]
    cache_file = os.path.join(base_path, 'parallel_latency_data.json')
    
    if os.path.exists(cache_file):
    # if False:
        print(f"Loading cached results for {machine_name}...")
        with open(cache_file, 'r') as f:
            results_by_machine[machine_name] = [dict(r) for r in json.load(f)]
    else:
        print(f"No cached data for {machine_name}, will analyze")

# If any machine doesn't have cached results, analyze all
if len(results_by_machine) < len(base_paths):
    print("Analyzing experiments for all machines...")
    results_by_machine = analyzer.analyze_latency_experiments()

# Create and save visualization
analyzer.create_latency_visualization_matplotlib(results_by_machine)