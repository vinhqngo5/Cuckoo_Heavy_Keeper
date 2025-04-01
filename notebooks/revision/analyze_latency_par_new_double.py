import os
import re
import numpy as np
import json
import matplotlib
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
from matplotlib.patches import ConnectionPatch, Rectangle
from matplotlib.ticker import MaxNLocator
from typing import List, Dict, Set, Any
from mpl_toolkits.axes_grid1.inset_locator import inset_axes, mark_inset
import matplotlib.font_manager as fm

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
            'tick_size_inset': 10,
            'tick_label_inset': 10,
            'annotation_size': 8,
            'legend_size': 10,
            'machine_name_size': 12,
            'inset_text_size': 10
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
        
        # Create figure
        fig = plt.figure(figsize=(6.45, 7))
        
        # Create a grid with 3 rows (each row contains a main plot and an inset)
        outer_grid = fig.add_gridspec(3, 2, height_ratios=[1, 1, 1], hspace=0.28, wspace=0.24)
        
        # Store axes for later reference
        main_axes = []
        inset_axes = []
        
        # Setup each cell with main plot and inset
        for row in range(3):
            row_axes = []
            row_insets = []
            
            for col in range(2):
                # Create nested gridspec inside each cell
                inner_grid = outer_grid[row, col].subgridspec(2, 1, height_ratios=[4.32, 1.55], hspace=0.9)
                
                # Create main plot
                main_ax = fig.add_subplot(inner_grid[0])
                row_axes.append(main_ax)
                
                # Create inset (separate x-axis, not shared)
                inset_ax = fig.add_subplot(inner_grid[1])
                row_insets.append(inset_ax)
                
            main_axes.append(row_axes)
            inset_axes.append(row_insets)
        
        # Make sure to include all thread values
        thread_values = sorted([int(x) for x in self.fixed_params['NUM_THREADS']])
        
        # Process each machine (column)
        for machine_idx, (machine_name, results) in enumerate(results_by_machine.items()):
            
            # Process each query rate (row)
            for row_idx, query_rate in enumerate(query_rates):
                ax = main_axes[row_idx][machine_idx]
                ax_inset = inset_axes[row_idx][machine_idx]
                
                # Dictionary to store all data and CHK-specific data
                all_data = {}
                chk_data = {}  # To track CHK ranges for y-axis limits
                
                # Process each algorithm
                for alg in sorted(set(r['ALGORITHM'] for r in results)):
                    color_name, _, marker = algorithm_decorations[alg]
                    color = material_colors[color_name]["500"]
                    
                    # Process each design type
                    for design in ['GLOBAL_HASHMAP', 'QPOPSS']:
                        design_suffix = 'q' if design == 'GLOBAL_HASHMAP' else 'i'
                        key = f"{alg}_{design}"
                        all_data[key] = {"x": [], "y": [], "color": color, "style": design_line_styles[design], 
                                        "marker": marker, "alg": alg, "design": design}
                        
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
                            
                            all_data[key]["x"] = x_values
                            all_data[key]["y"] = y_values
                            
                            # Track CHK data for setting y-axis limits
                            if alg == 'cuckoo_heavy_keeper':
                                chk_data[key] = {"x": x_values, "y": y_values}
                            
                            # Create label for main plot
                            label = None
                            if row_idx == 0 and machine_idx == 0:
                                label = f"m{algorithm_display_names[alg]}-{design_suffix}"
                            
                            # Plot in main axes
                            ax.plot(x_values, y_values, 
                                linestyle=design_line_styles[design], marker=marker, 
                                markerfacecolor='none', markersize=6,
                                linewidth=1.5, color=color, 
                                label=label)
                            
                            # Plot in inset axes (all algorithms)
                            ax_inset.plot(x_values, y_values, 
                                    linestyle=design_line_styles[design], marker=marker, 
                                    markerfacecolor='none', markersize=4,
                                    linewidth=1.2, color=color)
                
                # Style the main subplot
                ax.set_title(f"HH-Query Rate = {query_rate_labels[query_rate]}", 
                        fontsize=font_config['title_size'], fontfamily=font_config['family'], pad=2)
                ax.set_ylabel('Latency (μs)', fontsize=font_config['label_size'], 
                            fontfamily=font_config['family'], labelpad=1)
                
                for spine in ax.spines.values():
                    spine.set_visible(True)
                    spine.set_linewidth(0.1)
                    spine.set_color('black')
                
                # Set x-ticks for main plot
                ax.set_xticks(thread_values)
                ax.set_xticklabels([str(x) for x in thread_values], 
                                fontsize=font_config['tick_size'])
                ax.tick_params(axis='both', which='both', direction='in', 
                            pad=2, labelsize=font_config['tick_size'])
                ax.yaxis.set_major_locator(MaxNLocator(nbins=4, min_n_ticks=4))
                
                ax.grid(True, color='gray', alpha=0.2, linestyle='-', linewidth=0.1, axis='y', zorder=0)
                
                # Add x-axis label to main plot
                ax.set_xlabel('Number of Threads', fontsize=font_config['label_size'], 
                            fontfamily=font_config['family'], labelpad=1)
                
                # Style the inset
                # Set y-axis limits based on CHK data
                if chk_data:
                    all_chk_y = []
                    for data in chk_data.values():
                        all_chk_y.extend(data["y"])
                    
                    if all_chk_y:
                        y_max = max(all_chk_y) * 1.1  # Add 10% margin
                        ax_inset.set_ylim(0, y_max)
                        
                        # Add exactly 3 y-ticks with nice round numbers
                        # Round max value to a nice number
                        rounded_max = np.ceil(y_max / 100) * 100  # Round up to nearest 100
                        middle_value = rounded_max / 2  # Middle value
                        
                        # Set exactly 3 ticks: 0, middle, max
                        y_ticks = [0, middle_value, rounded_max]
                        ax_inset.set_yticks(y_ticks)
                        ax_inset.set_yticklabels([f"{int(y)}" for y in y_ticks], 
                                                fontsize=font_config['tick_size_inset'])
                
                # Set x-axis limits to show all threads
                ax_inset.set_xlim(1, 70)
                
                # Format the inset
                ax_inset.grid(True, alpha=0.2, linestyle='-', linewidth=0.1)
                
                # Only show subset of x-ticks to avoid crowding
                ax_inset.set_xticks([1, 10, 20, 30, 40, 50, 60, 70])
                ax_inset.set_xticklabels(['1', '10', '20', '30', '40', '50', '60', '70'], 
                                    fontsize=font_config['tick_size_inset'])
                ax_inset.tick_params(axis='both', which='both', labelsize=font_config['tick_size_inset'])
                
                # Add inset title to indicate zoom
                ax_inset.set_title("Zoom (All threads, y-axis scaled to CHK)", 
                                fontsize=font_config['inset_text_size'], fontfamily=font_config['family'], pad=1)

                mark_inset(ax, ax_inset, loc1=1, loc2=2, fc="none", ec="0.5", lw=1, ls=":")
                # mark_inset(ax, axins, loc1=3, loc2=4, fc="none", ec="0.5", lw=0.1, ls=":")

                
        # Add machine names as a super title for each column
        for machine_idx, machine_name in enumerate(results_by_machine.keys()):
            fig.text(0.25 + 0.5*machine_idx, 0.893, machine_name, 
                    ha='center', fontsize=font_config['machine_name_size'], 
                    fontfamily=font_config['family'])
        
        # Organize legend by algorithm pairs (-q and -i versions together)
        handles, labels = main_axes[0][0].get_legend_handles_labels()
        
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
                
        # Add legend with organized pairs
        fig.legend(new_handles, new_labels,
                loc='upper center',
                bbox_to_anchor=(0.5, 0.99),
                ncol=len(alg_pairs),  # One column per algorithm
                fontsize=font_config['legend_size'],
                frameon=False,
                handlelength=1.5,
                handletextpad=0.5,
                columnspacing=0.6,
                prop={'family': font_config['family']})
        
        # Adjust layout
        plt.tight_layout()
        # plt.subplots_adjust(top=0.878, wspace=0.24)
        plt.subplots_adjust(top=0.864, wspace=0.24)
        
        # Save figure
        figure_path = os.path.join(self.base_paths[0], 'figures')
        os.makedirs(figure_path, exist_ok=True)
        filename = f"par_latency_comparison_all.pdf"
        plt.savefig(os.path.join(figure_path, filename), format='pdf', 
                bbox_inches='tight', pad_inches=0.022, dpi=20000)
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

# machine_names = ["Machine 1", "Machine 2"]
machine_names = ['Platform A', 'Platform B']

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