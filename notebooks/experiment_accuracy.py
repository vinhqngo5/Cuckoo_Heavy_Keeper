import os
import subprocess
from itertools import product

# Define your varying build configs
algorithms = ["cuckoo_heavy_keeper"]
# evaluate_modes = ["throughput", "accuracy"]
evaluate_modes = ["accuracy"]
accuracy_stream_sizes = [1000000, 10000000, 20000000, 50000000, 100000000]
accuracy_stream_sizes = [10000000]
accuracy_when = ["start", "ivl", "end"]
accuracy_error_sources = {
    "start": ["algo", "algo_df"],
    "end": ["algo", "algo_df"],
    "ivl": ["algo_df_continuous"]
}

# Runtime parameters
# num_threads = [1, 2, 4, 8, 10, 20, 30, 40, 50, 60, 70]
num_threads = [10, 20, 30, 40, 50, 60, 70]
# num_threads = [70]
dist_params = [1, 1.2, 1.5]
# dist_params = [1.5]
theta = 0.00005
bucket_num = 512

def build_project(algorithm, evaluate_mode, stream_size=None, when=None, error_source=None):
    # Prepare DELEGATION_FLAGS
    delegation_flags = [
        f"ALGORITHM={algorithm}",
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
        "--target example_delegation_heavyhitter_cuckoo_heavy_keeper"
    )
    
    print(f"Running build command:\n{build_cmd}")
    subprocess.run(build_cmd, shell=True, check=True)

# Run function
def run_project(num_threads, dist_param, cuckoo_theta):
    print (f"Running project with num_threads={num_threads}, dist_param={dist_param}, cuckoo_theta={cuckoo_theta}")
    executable = "./build/release/bin/release/example_delegation_heavyhitter_cuckoo_heavy_keeper"
    runtime_config = (
        f"--app.num_threads {num_threads} "
        f"--app.dist_param {dist_param} "
        f"--app.duration 1 "
        f"--cuckooheavykeeper.bucket_num {bucket_num} "
        f"--app.theta {theta} "
        f"--cuckooheavykeeper.theta {cuckoo_theta} "
        f"--app.num_runs 300" # number of run times
    )

    cmd = f"{executable} {runtime_config} >> /home/vinh/Q32024/CuckooHeavyKeeper/build/release/bin/release/log.txt"
    subprocess.run(cmd, shell=True, check=True)


if __name__ == "__main__":
    for algorithm in algorithms:
        for evaluate_mode in evaluate_modes:
            if evaluate_mode == "throughput":
                build_project(algorithm, evaluate_mode)
                for threads, dist_param in product(num_threads, dist_params):
                    run_project(threads, dist_param, theta)
            elif evaluate_mode == "accuracy":
                for stream_size, when in product(accuracy_stream_sizes, accuracy_when):
                    for error_source in accuracy_error_sources[when]:
                        print (f"Running accuracy project with stream_size={stream_size}, when={when}, error_source={error_source}")
                        build_project(algorithm, evaluate_mode, stream_size, when, error_source)
                        for threads, dist_param in product(num_threads, dist_params):
                            run_project(threads, dist_param, theta)
