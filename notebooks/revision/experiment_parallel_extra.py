import os
import subprocess
from itertools import product

# Algorithm mappings
algorithms = [
    "count_min",           # CMS
    "heavy_keeper",        # HK 
    "cuckoo_heavy_keeper", # CHK
    "augmented_sketch",    # AS
    "heap_hashmap_space_saving" # SS
]

# Algorithm abbreviation mapping
algo_abbrev = {
    "count_min": "CMS",
    "heavy_keeper": "HK",
    "cuckoo_heavy_keeper": "CHK",
    "augmented_sketch": "AS",
    "heap_hashmap_space_saving": "SS"
}

parallel_designs = ["GLOBAL_HASHMAP", "QPOPSS"]
evaluate_modes = ["throughput", "latency"]
accuracy_stream_sizes = [10000000]
num_threads = [10, 20, 30, 40, 50, 60, 70]
heavy_query_rates = [0, 1, 10]
dist_params = [1.5]
theta = 0.00005
base_unit = 1
num_runs = 30
accuracy_when = ["ivl"]

def get_algorithm_params(algorithm, base_unit, theta):
    params = {
        "count_min": f"--countmin.width={32*base_unit} --countmin.depth=8",
        "heavy_keeper": f"--heavykeeper.m2={85*base_unit}",
        "cuckoo_heavy_keeper": f"--cuckooheavykeeper.bucket_num={32*base_unit} --cuckooheavykeeper.theta={theta}",
        "augmented_sketch": f"--augmentedsketch.width={29 + (base_unit-1) * 32} --augmentedsketch.depth=8",
        "heap_hashmap_space_saving": f"--spacesaving.k={43*base_unit}"
    }
    return params[algorithm]

def get_executable_name(algorithm, parallel_design, evaluate_mode):
    # Get algorithm abbreviation
    abbrev = algo_abbrev[algorithm]
    # Add Q suffix for QPOPSS design
    q_suffix = "I" if parallel_design == "QPOPSS" else "Q"
    return f"example_m{abbrev}{q_suffix}_{evaluate_mode}"

# Build parallel_design_targets dynamically
parallel_design_targets = {}
for pd in parallel_designs:
    parallel_design_targets[pd] = {
        mode: {
            algo: get_executable_name(algo, pd, mode)
            for algo in algorithms
        }
        for mode in evaluate_modes
    }

accuracy_error_sources = {
    "start": ["algo", "algo_df"],
    "end": ["algo", "algo_df"],
    "ivl": ["algo_df_continuous"]
}

duration = 1

def build_project(algorithm, parallel_design, evaluate_mode, stream_size=None, when=None, error_source=None):
    delegation_flags = [
        f"ALGORITHM={algorithm}",
        f"PARALLEL_DESIGN={parallel_design}",
        f"MODE=heavy_hitter",
        f"EVALUATE_MODE={evaluate_mode}",
    ]
    
    if evaluate_mode == "accuracy":
        if stream_size:
            delegation_flags.append(f"EVALUATE_ACCURACY_STREAM_SIZE={stream_size}")
        if when:
            delegation_flags.append(f"EVALUATE_ACCURACY_WHEN={when}")
        if error_source:
            delegation_flags.append(f"EVALUATE_ACCURACY_ERROR_SOURCES={error_source}")
    
    cmake_flags = ";".join(delegation_flags)
    project_root = "/home/vinh/Q32024/CuckooHeavyKeeper"
    build_dir = os.path.join(project_root, "build", "release")
    os.makedirs(build_dir, exist_ok=True)

    update_cmd = (
        f'cmake --preset release '
        f'-S {project_root} '
        f'-B {build_dir} '
        f'-DDELEGATION_FLAGS="{cmake_flags}"'
    )
    print(f"Running CMake configure command:\n{update_cmd}")
    subprocess.run(update_cmd, shell=True, check=True)

    target_name = parallel_design_targets[parallel_design][evaluate_mode][algorithm]
    build_cmd = f"cmake --build {build_dir} --target {target_name}"
    print(f"Running build command:\n{build_cmd}")
    subprocess.run(build_cmd, shell=True, check=True)

def run_project(algorithm, parallel_design, num_threads, dist_param, cuckoo_theta, heavy_query_rate, evaluate_mode):
    print(f"Running project with algorithm={algorithm}, num_threads={num_threads}, dist_param={dist_param}, heavy_query_rate={heavy_query_rate}")
    
    executable = f"./build/bin/release/{parallel_design_targets[parallel_design][evaluate_mode][algorithm]}"
    algorithm_params = get_algorithm_params(algorithm, base_unit, theta)
    
    runtime_config = (
        f"--app.num_threads {num_threads} "
        f"--app.dist_param {dist_param} "
        f"--app.duration {duration} "
        f"--app.theta {theta} "
        f"--delegationheavyhitter.heavy_query_rate {heavy_query_rate} "
        f"--app.num_runs {num_runs} "
        f"{algorithm_params}"
    )

    cmd = f"{executable} {runtime_config}"
    print(f"Running command:\n{cmd}")
    
    max_attempts = 4
    for attempt in range(max_attempts):
        try:
            subprocess.run(cmd, shell=True, check=True)
            break
        except Exception as e:
            print(f"Attempt {attempt + 1} failed with error: {e}")
            if attempt == max_attempts - 1:
                print(f"Failed after {max_attempts} attempts. Exiting.")

if __name__ == "__main__":
    for algorithm in algorithms:
        for parallel_design in parallel_designs:
            for evaluate_mode in evaluate_modes:
                if evaluate_mode in ["throughput", "latency"]:
                    for threads, dist_param, heavy_query_rate in product(num_threads, dist_params, heavy_query_rates):
                        run_project(algorithm, parallel_design, threads, dist_param, theta, heavy_query_rate, evaluate_mode)
                elif evaluate_mode == "accuracy":
                    for stream_size, when in product(accuracy_stream_sizes, accuracy_when):
                        for error_source in accuracy_error_sources[when]:
                            print(f"Running accuracy with stream_size={stream_size}, when={when}, error_source={error_source}")
                            for threads, dist_param, heavy_query_rate in product(num_threads, dist_params, heavy_query_rates):
                                run_project(algorithm, parallel_design, threads, dist_param, theta, heavy_query_rate, evaluate_mode)