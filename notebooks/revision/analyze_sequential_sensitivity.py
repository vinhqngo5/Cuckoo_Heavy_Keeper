import os
import re
import json
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator
import matplotlib.font_manager as fm


def load_material_colors(filepath='material-colors.json'):
    """Load material design colors from JSON file."""
    with open(filepath, 'r') as f:
        colors = json.load(f)
    return colors

class CuckooParameterSensitivityAnalyzer:
    # Regular expression pattern to extract results from log files
    RESULT_PATTERN = r'RESULT_SUMMARY: FrequencyEstimator=(\w+(?:\s*\w+)*) TotalHeavyHitters=(\d+) TotalHeavyHitterCandidates=(\d+) Precision=([\d.e-]+) Recall=([\d.e-]+) F1Score=([\d.e-]+) ARE=([\d.e-]+) AAE=([\d.e-]+) ExecutionTime=([\d.e-]+) Throughput=([\d.e-]+)'
    
    def __init__(self):
        """Initialize the analyzer with hardcoded folder paths and configuration."""
        # B parameter sensitivity folders
        self.b_folders = {
            "1.008": "experiments_sensitivity_20250314/sequential_1.008",
            "1.08": "experiments_sensitivity_20250314/sequential_1.08",
            "1.8": "experiments_sensitivity_20250314/sequential_1.8"
        }
        
        # Heavy entries sensitivity folders
        self.heavy_entries_folders = {
            "2": "experiments_sensitivity_20250314/sequential_heavyentries_2",
            "4": "experiments_sensitivity_20250314/sequential_heavyentries_4",
            "8": "experiments_sensitivity_20250314/sequential_heavyentries_8"
        }
        
        # Load colors for consistent styling
        self.material_colors = load_material_colors("/home/vinh/Q32024/CuckooHeavyKeeper/notebooks/material-colors.json")
        
        # Define fixed parameters - we only care about varying BASE_UNIT
        self.fixed_params = {
            'DIST_PARAM': '1.2',
            'THETA': '0.0005'
        }
        
        # Define line styles for different configurations
        self.line_styles = {
            # For b parameter
            "1.008": ("--", "^", self.material_colors["blue"]["500"]),  # Changed from "o" to "^" (triangle)
            "1.08": ("-", "x", self.material_colors["purple"]["700"]),  # Keep consistent with original
            "1.8": (":", "D", self.material_colors["red"]["500"]),     # Changed from "s" to "D" (diamond)
            
            # For heavy entries - use different markers to avoid overlap
            "2": ("-", "x", self.material_colors["purple"]["700"]),  
            "4": ("--", "^", self.material_colors["blue"]["700"]),  # Changed from "o" to "^" (triangle)
            "8": (":", "D", self.material_colors["red"]["500"])    # Changed from "s" to "D" (diamond)
        }

    def read_log_file(self, file_path):
        """Read and parse a log file to extract results."""
        results = []
        try:
            with open(file_path, 'r') as f:
                for line in f:
                    if match := re.search(self.RESULT_PATTERN, line):
                        # Only collect CuckooHeavyKeeper results
                        if match.group(1) == 'CuckooHeavyKeeper':
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
        except Exception as e:
            print(f"Error reading {file_path}: {e}")
        return results

    def collect_sensitivity_data(self, folders_dict):
        """Collect sensitivity data from the specified folders."""
        sensitivity_data = {}
        
        for param_value, folder_path in folders_dict.items():
            sensitivity_data[param_value] = {}
            
            # Walk through the experiment directory structure
            for base_unit in ["2", "4", "8", "16", "32"]:
                # Construct the path to the specific experiment configuration
                exp_path = os.path.join(
                    folder_path,
                    f"DIST_PARAM={self.fixed_params['DIST_PARAM']}",
                    f"BASE_UNIT={base_unit}",
                    f"THETA={self.fixed_params['THETA']}",
                    "logs"
                )
                
                if not os.path.exists(exp_path):
                    print(f"Path does not exist: {exp_path}")
                    continue
                
                # Find log files in this directory
                log_files = [f for f in os.listdir(exp_path) if f.endswith('.log')]
                
                all_results = []
                for log_file in log_files:
                    file_path = os.path.join(exp_path, log_file)
                    results = self.read_log_file(file_path)
                    all_results.extend(results)
                
                # Calculate average ARE for this BASE_UNIT value
                if all_results:
                    avg_are = np.mean([r['ARE'] for r in all_results])
                    sensitivity_data[param_value][base_unit] = avg_are
                else:
                    print(f"No valid results found in {exp_path}")
        
        return sensitivity_data

    def analyze_and_visualize(self):
        """Analyze both parameter sensitivities and create visualization."""
        # Collect data for both parameter types
        b_sensitivity_data = self.collect_sensitivity_data(self.b_folders)
        heavy_entries_sensitivity_data = self.collect_sensitivity_data(self.heavy_entries_folders)
        
        # Create the visualization
        self.create_side_by_side_plots(b_sensitivity_data, heavy_entries_sensitivity_data)

    def create_side_by_side_plots(self, b_data, heavy_entries_data):
        """Create two side-by-side plots with consistent styling."""
        # Set up the figure with two subplots side by side
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(5.6, 1.6))
        
        # Get the notebooks directory (one level up from the revision directory)
        notebooks_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

        # Reference fonts relative to the notebooks directory
        font_path = os.path.join(notebooks_dir, "fonts", "LinLibertine_R.ttf")
        fm.fontManager.addfont(font_path)

        # Also add the bold variant for titles
        bold_font_path = os.path.join(notebooks_dir, "fonts", "LinLibertine_RB.ttf")
        fm.fontManager.addfont(bold_font_path)
        
        # Font configuration
        font_config = {
            # 'family': 'serif',
            'family': 'Linux Libertine',
            'title_size': 12,
            'label_size': 12,
            'tick_size': 10,
            'legend_size': 10,  # Reduced font size for legend
            'annotation_size': 8  # Small font for annotations
        }
        
        plt.rcParams['font.family'] = font_config['family']
        plt.rcParams['font.serif'] = ['Linux Libertine']
        
        # Plot heavy entries sensitivity
        self._create_sensitivity_plot(
            ax1, heavy_entries_data,
            "CHK: heavy entries per bucket = 2, 4, 8",
            "Memory Size (KB)",
            "log10(ARE)"
        )
        
        # Plot b parameter sensitivity
        self._create_sensitivity_plot(
            ax2, b_data, 
            "CHK: decay base b=1.008, 1.08, 1.8",
            "Memory Size (KB)",
            "log10(ARE)"
        )
        
        # Add legend inside each subplot positioned at the middle to avoid overlap with annotations
        ax1.legend(loc='upper center', fontsize=font_config['legend_size'], 
                  ncol=len(b_data), 
                  bbox_to_anchor=(0.5, 0.97), handlelength=1.5, handletextpad=0.5)
        
        ax2.legend(loc='upper center', fontsize=font_config['legend_size'], 
                  ncol=len(heavy_entries_data), 
                  bbox_to_anchor=(0.5, 0.97), handlelength=1.5, handletextpad=0.5)
        
        # Adjust layout
        plt.tight_layout()
        
        # Save the figure
        os.makedirs("figures", exist_ok=True)
        plt.savefig("figures/parameter_sensitivity.pdf", format='pdf', bbox_inches='tight', pad_inches=0.03, dpi=2000)
        plt.savefig("figures/parameter_sensitivity.png", dpi=3000)
        plt.close()

    def _create_sensitivity_plot(self, ax, data, title, xlabel, ylabel):
        """Helper method to create a sensitivity plot on the given axis."""
        # Font configuration
        font_config = {
            'family': 'serif',
            'title_size': 10,
            'label_size': 8,
            'tick_size': 8,
            'annotation_size': 8  # Small font for annotations
        }
        
        # For each parameter value, plot the ARE vs BASE_UNIT
        param_data = {}  # Store data for improvement calculations
        
        for param_value, results in data.items():
            if not results:
                continue
                
            # Get sorted BASE_UNIT values and corresponding ARE values
            base_units = sorted([int(bu) for bu in results.keys()])
            are_values = [np.log10(results[str(bu)]) for bu in base_units]
            raw_values = [results[str(bu)] for bu in base_units]  # Raw values for improvement calculation
            
            # Store data for improvement calculations
            param_data[param_value] = {
                "base_units": base_units,
                "are_values": are_values,
                "raw_values": raw_values,
                "color": self.line_styles[param_value][2]
            }
            
            # Get line style for this parameter value
            linestyle, marker, color = self.line_styles[param_value]
            
            # Plot the line
            ax.plot(
                base_units, 
                are_values,
                label=param_value,
                color=color,
                linestyle=linestyle,
                # marker=marker,  # Commented out as in the original code
                markerfacecolor='none',
                linewidth=2,
                markersize=8
            )
        
        # Calculate and add improvement annotations
        self._add_improvement_annotations(ax, param_data, font_config)
        
        # Style the plot
        ax.set_title(title, fontsize=font_config['title_size'], fontfamily=font_config['family'])
        ax.set_xlabel(xlabel, fontsize=font_config['label_size'], fontfamily=font_config['family'], labelpad=0.1)
        ax.set_ylabel(ylabel, fontsize=font_config['label_size'], fontfamily=font_config['family'], labelpad=0.1)
        
        # Set x-axis to show all base_unit values
        ax.set_xticks(sorted([int(bu) for bu in set().union(*[r.keys() for r in data.values()])]))
        
        # Use fewer y ticks for cleaner appearance
        ax.yaxis.set_major_locator(MaxNLocator(nbins=5))
        
        # Set grid
        ax.grid(True, linestyle='--', alpha=0.3)
        
        # Set borders
        for spine in ax.spines.values():
            spine.set_visible(True)
            spine.set_linewidth(0.1)
            spine.set_color('black')
        
        ax.tick_params(
            axis='both',
            which='both',
            direction='in',
            labelsize=font_config['tick_size'],
            pad=4
        )
        
        # Apply font family to tick labels explicitly
        for label in ax.get_xticklabels() + ax.get_yticklabels():
            label.set_fontfamily(font_config['family'])

    def _format_improvement(self, value):
        """Helper to format improvement values in a consistent way."""
        if value >= 100:
            return f"{int(value)}"
        elif abs(value - round(value)) < 0.05:  # If very close to an integer
            return f"{int(round(value))}" 
        else:
            return f"{value:.1f}"

    def _add_improvement_annotations(self, ax, param_data, font_config):
        """Add annotations showing how much better each configuration is compared to the worst."""
        if not param_data:
            return
        
        # Get the maximum base_unit value for positioning annotations
        max_base_unit = max(max(data["base_units"]) for data in param_data.values())
        
        # For ARE, lower is better, so we need to find the worst (highest) ARE for each base_unit
        improvement_data = {}
        
        # Calculate improvements for each parameter across all base_units
        for param_value, data in param_data.items():
            improvements = []
            
            for i, base_unit in enumerate(data["base_units"]):
                # Find all values for this base_unit across parameters
                base_unit_values = {}
                for p, p_data in param_data.items():
                    if base_unit in p_data["base_units"]:
                        idx = p_data["base_units"].index(base_unit)
                        base_unit_values[p] = p_data["raw_values"][idx]
                
                # Find the worst value for this base_unit
                if base_unit_values:
                    worst_value = max(base_unit_values.values())  # For ARE, highest is worst
                    # Calculate improvement factor (worst/current)
                    if base_unit_values[param_value] > 0:  # Avoid division by zero
                        improvement = worst_value / base_unit_values[param_value]
                        improvements.append(improvement)
            
            if improvements:
                min_improvement = min(improvements)
                max_improvement = max(improvements)
                avg_improvement = sum(improvements) / len(improvements)
                
                # Store for annotation
                improvement_data[param_value] = {
                    "min": min_improvement,
                    "max": max_improvement,
                    "avg": avg_improvement,
                    "color": data["color"],
                    "y_value": data["are_values"][-1]  # Y value for the last point
                }
        
        # Sort parameter values by average improvement (descending)
        sorted_params = sorted(improvement_data.keys(), 
                              key=lambda p: improvement_data[p]["avg"], 
                              reverse=True)
        
        # Get y-axis range for spacing annotations
        y_range = ax.get_ylim()
        y_span = y_range[1] - y_range[0]
        
        # Position annotations at the right edge with proper spacing
        x_pos = max_base_unit * 1.04
        ax.set_xlim(right=max_base_unit * 1.25)  # Extend x-axis to make room
        
        # Set initial y-position at the bottom
        start_y = y_range[0] + 0.1 * y_span
        
        # Add annotations with proper spacing
        for i, param in enumerate(sorted_params):
            imp_data = improvement_data[param]
            
            # Format improvement text
            if abs(imp_data["max"] - imp_data["min"]) < 0.8:  # Small range
                imp_text = f"{self._format_improvement(imp_data['min'])}×"
            else:
                min_text = self._format_improvement(imp_data["min"])
                max_text = self._format_improvement(imp_data["max"])
                imp_text = f"{min_text}-{max_text}×"
            
            # Calculate y position with spacing
            y_pos = start_y + i * (y_span * 0.15)
            
            # Add annotation
            ax.annotate(
                imp_text,
                xy=(x_pos, y_pos),
                color=imp_data["color"],
                fontsize=font_config['annotation_size'],
                fontfamily=font_config['family'],
                ha='left',
                va='center'
            )

if __name__ == "__main__":
    analyzer = CuckooParameterSensitivityAnalyzer()
    analyzer.analyze_and_visualize()