import os
import subprocess
from itertools import product


# # common configs
# algorithms = ["cuckoo_heavy_keeper"]
# parallel_designs = ["GLOBAL_HASHMAP", "QPOPSS"]
# evaluate_modes = ["throughput", "latency"]
# accuracy_stream_sizes = [10000000]
# num_threads = [10, 20, 30, 40, 50, 60, 70]
# heavy_query_rates = [0, 1, 5, 10]
# dist_params = [1.2]
# theta = 0.00005
# bucket_num = 128
# num_runs = 300
# accuracy_when = ["start", "ivl", "end"]
# accuracy_error_sources = {
#     "start": ["algo", "algo_df"],
#     "end": ["algo", "algo_df"],
#     "ivl": ["algo_df_continuous"]
# }
# parallel_design_targets = {
#     "GLOBAL_HASHMAP": "example_delegation_heavyhitter_cuckoo_heavy_keeper",
#     "QPOPSS": "example_delegation_heavyhitter_cuckoo_heavy_keeper_QPOPSS"
# }

# # configs for throughput experiment
# algorithms = ["cuckoo_heavy_keeper"]
# # parallel_designs = ["GLOBAL_HASHMAP", "QPOPSS"]
# parallel_designs = ["QPOPSS"]
# evaluate_modes = ["throughput"]
# accuracy_stream_sizes = [10000000]
# # num_threads = [1, 2, 5, 10, 20, 30, 40, 50, 60, 70]
# num_threads = [60, 70]
# heavy_query_rates = [5, 10]
# dist_params = [1.2, 1.5]
# theta = 0.00005
# bucket_num = 128
# num_runs = 30
# accuracy_when = []
# accuracy_error_sources = {}
# parallel_design_targets = {
#     "GLOBAL_HASHMAP": "example_delegation_heavyhitter_cuckoo_heavy_keeper",
#     "QPOPSS": "example_delegation_heavyhitter_cuckoo_heavy_keeper_QPOPSS"
# }
# duration = 1

# configs for throughput latency experiment
# algorithms = ["cuckoo_heavy_keeper"]
# parallel_designs = ["GLOBAL_HASHMAP", "QPOPSS"]
# evaluate_modes = ["latency"]
# accuracy_stream_sizes = [10000000]
# num_threads = [2, 5, 10, 20, 30, 40, 50, 60, 70]
# heavy_query_rates = [0, 1, 5, 10]
# dist_params = [1.2, 1.5]
# theta = 0.00005
# bucket_num = 128
# num_runs = 30
# accuracy_when = []
# accuracy_error_sources = {}
# parallel_design_targets = {
#     "GLOBAL_HASHMAP": "example_delegation_heavyhitter_cuckoo_heavy_keeper",
#     "QPOPSS": "example_delegation_heavyhitter_cuckoo_heavy_keeper_QPOPSS"
# }
# duration = 100

# configs for accuracy experiment
algorithms = ["cuckoo_heavy_keeper"]
parallel_designs = ["GLOBAL_HASHMAP", "QPOPSS"]
evaluate_modes = ["accuracy"]
accuracy_stream_sizes = [10000000]
# num_threads = [10, 20, 30, 40, 50, 60, 70]
num_threads = [30]
heavy_query_rates = [0, 1]
# dist_params = [1.2, 1.5]
dist_params = [1.5]
theta = 0.00005
bucket_num = 128
num_runs = 100
# accuracy_when = ["start", "ivl", "end"]
accuracy_when = [ "ivl", "end"]
accuracy_error_sources = {
    "start": ["algo", "algo_df"],
    "end": ["algo", "algo_df"],
    "ivl": ["algo_df_continuous"]
}
parallel_design_targets = {
    "GLOBAL_HASHMAP": "example_delegation_heavyhitter_cuckoo_heavy_keeper",
    "QPOPSS": "example_delegation_heavyhitter_cuckoo_heavy_keeper_QPOPSS"
}
duration = 1

def build_project(algorithm, parallel_design, evaluate_mode, stream_size=None, when=None, error_source=None):
    # Prepare DELEGATION_FLAGS
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
    
    # Join flags into one string for DELEGATION_FLAGS
    cmake_flags = ";".join(delegation_flags)

    # Set up paths
    project_root = "/home/vinh/Q32024/CuckooHeavyKeeper"
    build_dir = os.path.join(project_root, "build", "release")

    # Completely remove and recreate the build directory
    os.makedirs(build_dir, exist_ok=True)

    # Update CMake cache using the release configure preset
    update_cmd = (
        f'cmake --preset release '
        f'-S {project_root} '
        f'-B {build_dir} '
        f'-DDELEGATION_FLAGS="{cmake_flags}"'
    )
    print(f"Running CMake configure command:\n{update_cmd}")
    subprocess.run(update_cmd, shell=True, check=True)


    # Compile only the specific target using the release build preset
    build_cmd = (
        f"cmake --build {build_dir} "
        f"--target {parallel_design_targets[parallel_design]}"
    )
    
    print(f"Running build command:\n{build_cmd}")
    subprocess.run(build_cmd, shell=True, check=True)

# Run function
def run_project(parallel_design, num_threads, dist_param, cuckoo_theta, heavy_query_rate):
    print (f"Running project with num_threads={num_threads}, dist_param={dist_param}, cuckoo_theta={cuckoo_theta}, heavy_query_rate={heavy_query_rate}, parallel_design={parallel_design}")
    executable = f"./build/release/bin/release/{parallel_design_targets[parallel_design]}"
    runtime_config = (
        f"--app.num_threads {num_threads} "
        f"--app.dist_param {dist_param} "
        f"--app.duration {duration} "
        f"--cuckooheavykeeper.bucket_num {bucket_num} "
        f"--app.theta {theta} "
        f"--cuckooheavykeeper.theta {cuckoo_theta} "
        f"--delegationheavyhitter.heavy_query_rate {heavy_query_rate} "
        f"--app.num_runs {num_runs}" # number of run times
    )

    cmd = f"{executable} {runtime_config} &>> /home/vinh/Q32024/CuckooHeavyKeeper/build/release/bin/release/log.txt"
    print(f"Running command:\n{cmd}")
    max_attempts = 4
    for attempt in range(max_attempts):
      try:
        subprocess.run(cmd, shell=True, check=True)
        break  # Success, exit the loop
      except Exception as e:
        print(f"Attempt {attempt + 1} failed with error: {e}")
        if attempt == max_attempts - 1:  # Last attempt failed
          print(f"Failed after {max_attempts} attempts. Exiting.")
          # exit(1)  # Exit with error code 1


if __name__ == "__main__":
    for algorithm in algorithms:
        for parallel_design in parallel_designs:
            for evaluate_mode in evaluate_modes:
                if evaluate_mode == "throughput" or evaluate_mode == "latency":
                    build_project(algorithm, parallel_design, evaluate_mode)
                    for threads, dist_param, heavy_query_rate in product(num_threads, dist_params, heavy_query_rates):
                        run_project(parallel_design, threads, dist_param, theta, heavy_query_rate)
                elif evaluate_mode == "accuracy":
                    for stream_size, when in product(accuracy_stream_sizes, accuracy_when):
                        for error_source in accuracy_error_sources[when]:
                            print (f"Running accuracy project with stream_size={stream_size}, when={when}, error_source={error_source}")
                            build_project(algorithm, parallel_design, evaluate_mode, stream_size, when, error_source)
                            for threads, dist_param, heavy_query_rate in product(num_threads, dist_params, heavy_query_rates):
                                run_project(parallel_design, threads, dist_param, theta, heavy_query_rate)


