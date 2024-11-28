## Table of Contents

1. [Design](#design)
   - [Cuckoo Heavy Keeper](#cuckoo-heavy-keeper)
   - [Parallel Cuckoo Heavy Keeper](#parallel-cuckoo-heavy-keeper)
   - [Directory Description](#directory-description)

2. [Prerequisites](#prerequisites)
   - [System Requirements](#system-requirements)
   - [Install Dependencies](#install-dependencies)
   - [Python Setup](#python-setup)
   - [Build Configuration](#build-configuration)

3. [Compile & Run](#compile--run)
   - [Sequential Heavy Hitter Examples](#sequential-heavy-hitter-examples)
     - [Available Algorithms](#available-algorithms)
     - [Compile Instructions](#compile-instructions)
     - [Basic Usage](#basic-usage)
     - [Example Commands](#example-commands)
   - [Parallel Heavy Hitter Examples](#parallel-heavy-hitter-examples)
     - [Available Algorithms](#available-algorithms-1)
     - [Compile Instructions](#compile-instructions-1)
     - [Basic Usage](#basic-usage-1)
     - [Example Commands](#example-commands-1)

4. [Reproduce](#reproduce)
   - [Sequential Heavy Hitter Figures](#sequential-heavy-hitter-figures)
   - [Parallel Heavy Hitter Figures](#parallel-heavy-hitter-figures)


## Design
### Cuckoo Heavy Keeper
- [`src/frequency_estimator/CuckooHeavyKeeper.hpp`](src/frequency_estimator/CuckooHeavyKeeper.hpp): Contains the source code for the CuckoHeavyKeeper.
- [`src/frequency_estimator/`](src/frequency_estimator/): Contains the source code for the all heavy hitter algorithms.

### Parallel Cuckoo Heavy Keeper
- [`src/delegation_sketch`](src/delegation_sketch): Contains the source code for the parallel designs.

### Directory Description
- [`.devcontainer/`](.devcontainer/): Contains configuration files for the development container.
  - [`devcontainer.json`](.devcontainer/devcontainer.json): Configuration file for the development container.
  - [`Dockerfile`](.devcontainer/Dockerfile): Dockerfile to set up the development container environment.
  - [`reinstall-cmake.sh`](.devcontainer/reinstall-cmake.sh): Script to reinstall CMake from source if needed.

- [`.vscode/`](.vscode/): Contains Visual Studio Code specific settings and configurations.
  - [`launch.json`](.vscode/launch.json): Configuration for debugging the project.
  - [`settings.json`](.vscode/settings.json): General settings for the workspace.

- [`3rd/`](3rd/): Contains third-party libraries and dependencies.
  - [`json/`](3rd/json/): Directory for JSON-related third-party libraries.

- [`examples/`](examples/): Contains example projects and modules.
  - [`CMakeLists.txt`](examples/CMakeLists.txt): CMake configuration file for the examples.
  - [`concurrent_data_structure/`](examples/concurrent_data_structure/): Directory for concurrent data structure examples.
  - [`delegation_sketch/`](examples/delegation_sketch/): Directory for delegation sketch examples.
  - [`frequency_estimator/`](examples/frequency_estimator/): Directory for frequency estimator examples.

- [`notebooks/`](notebooks/): Contains Jupyter notebooks for experiments and analysis.

- [`src/`](src/): Contains the source code of the project.

- [`CMakeLists.txt`](CMakeLists.txt): Top-level CMake configuration file.

- [`CMakePresets.json`](CMakePresets.json): CMake presets configuration file.

- [`.clang-format`](.clang-format): Clang-format configuration file.

- [`.clang-tidy`](.clang-tidy): Clang-tidy configuration file.

- [`.gitattributes`](.gitattributes): Git attributes file.

- [`.gitignore`](.gitignore): Git ignore file.

- [`rerun.txt`](rerun.txt): File to track rerun information.

## Prerequisites
### System Requirements
- C++ compiler (Clang) supporting C++20
- CMake (version 3.10 or higher)
- Python (version 3.11 or higher)

### Install Dependencies

For openSUSE:
* C++ and build tools:
```bash
sudo zypper install -y gcc gcc-c++ cmake clang clang-tools
```
* Python environment:
```bash
sudo zypper install -y python311 python311-devel
```

For Ubuntu:
* C++ and build tools:
```bash
sudo apt update
sudo apt install -y build-essential cmake clang clang-tidy clang-format
```
* Python environment:
```bash
sudo apt install -y python3.11 python3.11-venv
```

### Python Setup
```bash
# Create virtual environment
python3.11 -m venv .venv

# Activate virtual environment
source .venv/bin/activate

# Install required packages
pip install pandas matplotlib seaborn
```

###  Build Configuration
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=release
```

## Compile & Run
### Sequential Heavy Hitter Examples

The project provides multiple heavy hitter algorithm implementations that can be built and run separately.

#### Available Algorithms
1. Count-Min Sketch
2. Heavy Keeper
3. Cuckoo Heavy Keeper
4. Augmented Sketch
5. Stream Summary Space Saving
6. Simple Space Saving
7. Heap HashMap Space Saving
8. Weighted Frequent
9. Optimized Weighted Frequent

#### Compile Instructions
```bash
# Cd build directory
cd build

# Build specific algorithm (build to: ./build/bin/release)
make example_count_min
make example_heavy_keeper
make example_cuckoo_heavy_keeper
make example_augmented_sketch
make example_stream_summary_space_saving
make example_simple_space_saving
make example_heap_hashmap_space_saving
make example_weighted_frequent
make example_optimized_weighted_frequent

# help
./bin/release/example_cuckoo_heavy_keeper --help
```

#### Basic Usage
```bash
./bin/release/example_cuckoo_heavy_keeper [OPTIONS]
```
#### --help Example
<div style="max-height: 400px; overflow-y: auto; border: 1px solid #ccc; padding: 10px;">

```bash
./bin/release/example_cuckoo_heavy_keeper -h

  --app.mode:
      Mode of the application: count/top-k/heavy_hitter
      Default value: count (String)

  --app.num_runs:
      Number of runs for the application
      Default value: 10 (32-bit integer)

  --app.line_read:
      Number of lines to read from the input file
      Default value: 1000000 (32-bit integer)

  --app.dom_size:
      Domain size for the dataset
      Default value: 1000000 (32-bit integer)

  --app.tuples_no:
      Number of tuples for the dataset
      Default value: 6000000 (32-bit integer)

  --delegation.dist_type:
      Distribution type for the dataset
      Default value: 1 (32-bit integer)

  --app.dist_param:
      Distribution parameter for the dataset
      Default value: 1.3 (Double-precision number)

  --app.dist_shuff:
      Distribution shuffle for the dataset
      Default value: 0 (Double-precision number)

  --app.dataset:
      Dataset: CAIDA/zipf
      Default value: zipf (String)

  --app.theta:
      Theta value for the finding heavy hitters
      Default value: 0.01 (Float-precision number)

  --app.duration:
      Duration of the benchmark
      Default value: 1 (32-bit integer)

  --cuckooheavykeeper.bucket_num:
      Number of buckets
      Default value: 1024 (32-bit integer)

  --cuckooheavykeeper.max_loop:
      Maximum loop for cuckoo hashing
      Default value: 10 (32-bit integer)

  --cuckooheavykeeper.bits_per_item:
      Number of bits per item
      Default value: 16 (32-bit integer)

  --cuckooheavykeeper.theta:
      Theta value for the cuckoo heavy keeper
      Default value: 0.1 (Float-precision number)

  --cuckooheavykeeper.HK_b:
      b value for the cuckoo heavy keeper
      Default value: 1.08 (Float-precision number)
```
</div> 

### Example Commands

<div style="max-height: 400px; overflow-y: auto; border: 1px solid #ccc; padding: 10px;">

```bash
./bin/release/example_cuckoo_heavy_keeper --app.line_read=5000000 --app.theta  0.0005 --cuckooheavykeeper.bucket_num=128 --app.dist_param 1.2 --cuckooheavykeeper.theta 0.0005 --app.num_runs 10

+-------------------------------------------+
| CuckooHeavyKeeperConfig                   |
+-------------------------------------------+
| BUCKET_NUM                      : 128     |
| MAX_LOOP                        : 10      |
| BITS_PER_ITEM                   : 16      |
| THETA                           : 0.000500|
| HK_b                            : 1.080000|
+-------------------------------------------+

+-------------------------------------------+
| SequentialAppConfig                       |
+-------------------------------------------+
| MODE                            : count   |
| NUM_RUNS                        : 10      |
| LINE_READ                       : 5000000 |
| DOM_SIZE                        : 1000000 |
| tuples_no                       : 6000000 |
| DIST_TYPE                       : 1       |
| DIST_PARAM                      : 1.200000|
| DIST_SHUFF                      : 0.000000|
| DATASET                         : zipf    |
| THETA                           : 0.000500|
| DURATION                        : 1       |
+-------------------------------------------+

## Run 0
# sketch + heap heavy hitters
Execution Time: 385.268
Throughput: 1.2978e+07
Count distinct: 179708
Total heavy hitters: 142
Total heavy_hitter candidates: 140
# sample set: true heavy_hitter_counter
ARE: 0.00254966
AAE: 7.58451
Precision: 139 over 140
Recall: 139 over 142
F1 Score: 0.985816
---------------------
# sample set: heavy_hitter candidates
ARE: 189.216
AAE: 195.114
---------------------
RESULT_SUMMARY: FrequencyEstimator=CuckooHeavyKeeper TotalHeavyHitters=142 TotalHeavyHitterCandidates=140 Precision=0.992857 Recall=0.978873 F1Score=0.985816 ARE=0.00254966 AAE=7.58451 ExecutionTime=385.268 Throughput=12977991.900570
.....
```
</div>

### Parallel Heavy Hitter Examples
The project provides multiple heavy hitter algorithm implementations that can be built and run in parallel wrapper.

#### Available Algorithms
1. Count-Min Sketch
2. Heavy Keeper
3. Cuckoo Heavy Keeper
4. Augmented Sketch
5. Stream Summary Space Saving
6. Simple Space Saving
7. Heap HashMap Space Saving
8. Weighted Frequent
9. Optimized Weighted Frequent

#### Compile Instructions
```bash
# Cd build directory
cd build

# Build specific algorithm (build to: ./build/bin/release)
make example_mCHKI_throughput
make example_mCHKI_latency
make example_mCHKQ_throughput
make example_mCHKQ_latency


# help
./bin/release/example_mCHKI_throughput --help
```

#### Basic Usage
```bash
./bin/release/example_mCHKI_throughput [OPTIONS]
```

#### --help Example


<div style="max-height: 400px; overflow-y: auto; border: 1px solid #ccc; padding: 10px;">

```bash
./bin/release/example_mCHKI_throughput --help 
  --app.mode:
      Mode of the application: count/top-k/heavy_hitter
      Default value: count (String)

  --app.num_runs:
      Number of runs for the application
      Default value: 10 (32-bit integer)

  --app.line_read:
      Number of lines to read from the input file
      Default value: 1000000 (32-bit integer)

  --app.dom_size:
      Domain size for the dataset
      Default value: 1000000 (32-bit integer)

  --app.tuples_no:
      Number of tuples for the dataset
      Default value: 6000000 (32-bit integer)

  --delegation.dist_type:
      Distribution type for the dataset
      Default value: 1 (32-bit integer)

  --app.dist_param:
      Distribution parameter for the dataset
      Default value: 1.3 (Double-precision number)

  --app.dist_shuff:
      Distribution shuffle for the dataset
      Default value: 0 (Double-precision number)

  --app.dataset:
      Dataset: CAIDA/zipf
      Default value: zipf (String)

  --app.theta:
      Theta value for the finding heavy hitters
      Default value: 0.01 (Float-precision number)

  --app.duration:
      Duration of the benchmark
      Default value: 1 (32-bit integer)

  --app.num_threads:
      Number of threads for the parallel application
      Default value: 20 (32-bit integer)

  --cuckooheavykeeper.bucket_num:
      Number of buckets
      Default value: 1024 (32-bit integer)

  --cuckooheavykeeper.max_loop:
      Maximum loop for cuckoo hashing
      Default value: 10 (32-bit integer)

  --cuckooheavykeeper.bits_per_item:
      Number of bits per item
      Default value: 16 (32-bit integer)

  --cuckooheavykeeper.theta:
      Theta value for the cuckoo heavy keeper
      Default value: 0.1 (Float-precision number)

  --cuckooheavykeeper.HK_b:
      b value for the cuckoo heavy keeper
      Default value: 1.08 (Float-precision number)

  --delegationheavyhitter.filter_size:
      Filter size for the delegation heavy hitter sketch
      Default value: 16 (32-bit integer)

  --delegationheavyhitter.query_rate:
      Query rate for the delegation heavy hitter sketch
      Default value: 0 (Double-precision number)

  --delegationheavyhitter.heavy_query_rate:
      Query rate for the delegation heavy hitter sketch for querying all heavy
      hitters
      Default value: 0.1 (Double-precision number)
````
</div>


### Example Commands
<div style="max-height: 400px; overflow-y: auto; border: 1px solid #ccc; padding: 10px;">

```bash
./bin/release/example_mCHKI_throughput --app.num_threads 70 --app.dist_param 1.5 --app.duration 1 --cuckooheavykeeper.bucket_num 128 --app.theta 5e-05 --cuckooheavykeeper.theta 5e-05 --delegationheavyhitter.heavy_query_rate 0 --app.num_runs 100
+------------------------------------------------------+
| DelegationBuildConfig                                |
+------------------------------------------------------+
| algorithm                       : cuckoo_heavy_keeper|
| mode                            : heavy_hitter       |
| evaluate_mode                   : throughput         |
| parallel_design                 : QPOPSS             |
| evaluate_accuracy_when          : ivl                |
| evaluate_accuracy_error_sources : algo_df_continuous |
| evaluate_accuracy_stream_size   : 10000000           |
+------------------------------------------------------+

+-------------------------------------------+
| ParallelAppConfig                         |
+-------------------------------------------+
| MODE                            : count   |
| NUM_RUNS                        : 100     |
| LINE_READ                       : 1000000 |
| DOM_SIZE                        : 1000000 |
| tuples_no                       : 6000000 |
| DIST_TYPE                       : 1       |
| DIST_PARAM                      : 1.500000|
| DIST_SHUFF                      : 0.000000|
| DATASET                         : zipf    |
| THETA                           : 0.000050|
| DURATION                        : 1       |
| NUM_THREADS                     : 70      |
+-------------------------------------------+

+-------------------------------------------+
| DelegationHeavyHitterConfig               |
+-------------------------------------------+
| FILTER_SIZE                     : 16      |
| QUERY_RATE                      : 0.000000|
| HEAVY_QUERY_RATE                : 0.000000|
+-------------------------------------------+

+-------------------------------------------+
| CuckooHeavyKeeperConfig                   |
+-------------------------------------------+
| BUCKET_NUM                      : 128     |
| MAX_LOOP                        : 10      |
| BITS_PER_ITEM                   : 16      |
| THETA                           : 0.000050|
| HK_b                            : 1.080000|
+-------------------------------------------+

filename: ./Cuckoo_Heavy_Keeper/experiments/ALGORITHM=cuckoo_heavy_keeper/MODE=heavy_hitter/PARARLLEL_DESIGN=QPOPSS/EVALUATE_MODE=throughput/EVALUATE_ACCURACY_WHEN=ivl/EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous/EVALUATE_ACCURACY_STREAM_SIZE=10000000/NUM_THREADS=70/DIST_PARAM=1.500000/THETA=0.000050/HEAVY_QUERY_RATE=0.000000/+2024-11-28_214502_log.json
+------------------------------------------------------+
| DelegationBuildConfig                                |
+------------------------------------------------------+
| algorithm                       : cuckoo_heavy_keeper|
| mode                            : heavy_hitter       |
| evaluate_mode                   : throughput         |
| parallel_design                 : QPOPSS             |
| evaluate_accuracy_when          : ivl                |
| evaluate_accuracy_error_sources : algo_df_continuous |
| evaluate_accuracy_stream_size   : 10000000           |
+------------------------------------------------------+

+-------------------------------------------+
| ParallelAppConfig                         |
+-------------------------------------------+
| MODE                            : count   |
| NUM_RUNS                        : 100     |
| LINE_READ                       : 1000000 |
| DOM_SIZE                        : 1000000 |
| tuples_no                       : 5979563 |
| DIST_TYPE                       : 1       |
| DIST_PARAM                      : 1.500000|
| DIST_SHUFF                      : 0.000000|
| DATASET                         : zipf    |
| THETA                           : 0.000050|
| DURATION                        : 1       |
| NUM_THREADS                     : 70      |
+-------------------------------------------+

+-------------------------------------------+
| DelegationHeavyHitterConfig               |
+-------------------------------------------+
| FILTER_SIZE                     : 16      |
| QUERY_RATE                      : 0.000000|
| HEAVY_QUERY_RATE                : 0.000000|
+-------------------------------------------+

+-------------------------------------------+
| CuckooHeavyKeeperConfig                   |
+-------------------------------------------+
| BUCKET_NUM                      : 128     |
| MAX_LOOP                        : 10      |
| BITS_PER_ITEM                   : 16      |
| THETA                           : 0.000050|
| HK_b                            : 1.080000|
+-------------------------------------------+

thread: 0 start: 0 end: 85422 end-start:85422
thread: 1 start: 85422 end: 170844 end-start:85422
thread: 2 start: 170844 end: 256266 end-start:85422
thread: 3 start: 256266 end: 341688 end-start:85422
thread: 4 start: 341688 end: 427110 end-start:85422
thread: 5 start: 427110 end: 512532 end-start:85422
thread: 6 start: 512532 end: 597954 end-start:85422
...
```
</div>

## Reproduce 
### Sequential Heavy Hitter Figures

To reproduce the sequential heavy hitter figure, follow these steps:

1. **Install Prerequisites**: Ensure you have all the necessary prerequisites installed. Refer to the [Prerequisites](#prerequisites) section above.

2. **Run the Experiment**: Execute the following command to run the sequential heavy hitter experiment and generate the results:

    ```sh
    python ./notebooks/experiment_sequential.py
    ```

    This will run the experiment and produce the results needed to generate the sequential heavy hitter figures.

3. **Plot the Results**: Execute the following command to analyze the results and generate the plot:

    ```sh
    python ./notebooks/analyze_sequential.py
    ```

    This will analyze the results from the experiment and generate the plot for the sequential heavy hitter figures in `experiments/sequential/figures`

### Parallel Heavy Hitter Figures 

To reproduce the parallel heavy hitter figures, follow these steps:

1. **Install Prerequisites**: Ensure you have all the necessary prerequisites installed. Refer to the [Prerequisites](#prerequisites) section above.

2. **Run the Parallel Experiments**: Execute the following command to run the parallel heavy hitter experiments and generate the results:

    ```sh
    python ./notebooks/experiment_parallel.py
    ```

    This will run the experiments and produce the results needed for analyzing parallel performance.

3. **Plot the Throughput Results**: Execute the following command to analyze and plot throughput:

    ```sh 
    python ./notebooks/analyze_throughput_par.py
    ```

    This will generate the throughput plots in `experiments/figures`.

4. **Plot the Latency Results**: Execute the following command to analyze and plot latency:

    ```sh
    python ./notebooks/analyze_latency_par.py 
    ```
    
    This will generate the latency plots in `experiments/figures`.

The figures will show parallel scaling behavior including throughput and latency metrics across different thread counts.