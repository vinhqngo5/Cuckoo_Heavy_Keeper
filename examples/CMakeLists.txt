# example_heavyhitter
# 1. Binary with ALGORITHM = count_min
add_executable(example_count_min frequency_estimator/example_heavyhitter.cpp)
target_compile_definitions(example_count_min PRIVATE ALGORITHM=count_min)
target_link_libraries(example_count_min PRIVATE frequency_estimator_objects delegation_sketch_objects)

# 2. Binary with ALGORITHM = heavy_keeper
add_executable(example_heavy_keeper frequency_estimator/example_heavyhitter.cpp)
target_compile_definitions(example_heavy_keeper PRIVATE ALGORITHM=heavy_keeper)
target_link_libraries(example_heavy_keeper PRIVATE frequency_estimator_objects delegation_sketch_objects)

# 3. Binary with ALGORITHM = cuckoo_heavy_keeper
add_executable(example_cuckoo_heavy_keeper frequency_estimator/example_heavyhitter.cpp)
target_compile_definitions(example_cuckoo_heavy_keeper PRIVATE ALGORITHM=cuckoo_heavy_keeper)
target_link_libraries(example_cuckoo_heavy_keeper PRIVATE frequency_estimator_objects delegation_sketch_objects)


# 3. Binary with ALGORITHM = cuckoo_heavy_keeper_test
add_executable(example_cuckoo_heavy_keeper_test frequency_estimator/example_heavyhitter.cpp)
target_compile_definitions(example_cuckoo_heavy_keeper_test PRIVATE ALGORITHM=cuckoo_heavy_keeper)
target_link_libraries(example_cuckoo_heavy_keeper_test PRIVATE frequency_estimator_objects delegation_sketch_objects)

# 4. Binary with ALGORITHM = augmented_sketch
add_executable(example_augmented_sketch frequency_estimator/example_heavyhitter.cpp)
target_compile_definitions(example_augmented_sketch PRIVATE ALGORITHM=augmented_sketch)
target_link_libraries(example_augmented_sketch PRIVATE frequency_estimator_objects delegation_sketch_objects)

# # 5. Binary with ALGORITHM = stream_summary_space_saving
add_executable(example_stream_summary_space_saving frequency_estimator/example_heavyhitter.cpp)
target_compile_definitions(example_stream_summary_space_saving PRIVATE ALGORITHM=stream_summary_space_saving)
target_link_libraries(example_stream_summary_space_saving PRIVATE frequency_estimator_objects delegation_sketch_objects)

# # 6. Binary with ALGORITHM = simple_space_saving
add_executable(example_simple_space_saving frequency_estimator/example_heavyhitter.cpp)
target_compile_definitions(example_simple_space_saving PRIVATE ALGORITHM=simple_space_saving)
target_link_libraries(example_simple_space_saving PRIVATE frequency_estimator_objects delegation_sketch_objects)

# 7. Binary with ALGORITHM = heap_hashmap_space_saving
add_executable(example_heap_hashmap_space_saving frequency_estimator/example_heavyhitter.cpp)
target_compile_definitions(example_heap_hashmap_space_saving PRIVATE ALGORITHM=heap_hashmap_space_saving)
target_link_libraries(example_heap_hashmap_space_saving PRIVATE frequency_estimator_objects delegation_sketch_objects)

# 8 Binary with ALGORITHM = weighted_frequent
add_executable(example_weighted_frequent frequency_estimator/example_heavyhitter.cpp)
target_compile_definitions(example_weighted_frequent PRIVATE ALGORITHM=weighted_frequent)
target_link_libraries(example_weighted_frequent PRIVATE frequency_estimator_objects delegation_sketch_objects)

# 9. Binary with ALGORITHM = optimized_weighted_frequent
add_executable(example_optimized_weighted_frequent frequency_estimator/example_heavyhitter.cpp)
target_compile_definitions(example_optimized_weighted_frequent PRIVATE ALGORITHM=optimized_weighted_frequent)
target_link_libraries(example_optimized_weighted_frequent PRIVATE frequency_estimator_objects delegation_sketch_objects)


# Parallel versions of CHK
# mCHK-I
## throughput 
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=cuckoo_heavy_keeper"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=cuckoo_heavy_keeper"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=QPOPSS"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=throughput"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mCHKI_throughput delegation_sketch/example_delegation_heavyhitter_QPOPSS.cpp)
target_compile_definitions(example_mCHKI_throughput PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mCHKI_throughput PRIVATE frequency_estimator_objects delegation_sketch_objects)
## latency
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=cuckoo_heavy_keeper"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=cuckoo_heavy_keeper"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=QPOPSS"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=latency"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mCHKI_latency delegation_sketch/example_delegation_heavyhitter_QPOPSS.cpp)
target_compile_definitions(example_mCHKI_latency PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mCHKI_latency PRIVATE frequency_estimator_objects delegation_sketch_objects)

# mCHK-q
## throughput 
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=cuckoo_heavy_keeper"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=cuckoo_heavy_keeper"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=GLOBAL_HASHMAP"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=throughput"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mCHKQ_throughput delegation_sketch/example_delegation_heavyhitter.cpp)
target_compile_definitions(example_mCHKQ_throughput PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mCHKQ_throughput PRIVATE frequency_estimator_objects delegation_sketch_objects)
## latency
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=cuckoo_heavy_keeper"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=cuckoo_heavy_keeper"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=GLOBAL_HASHMAP"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=latency"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mCHKQ_latency delegation_sketch/example_delegation_heavyhitter.cpp)
target_compile_definitions(example_mCHKQ_latency PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mCHKQ_latency PRIVATE frequency_estimator_objects delegation_sketch_objects)


# Parallel versions of CMS
# mCMS-I
## throughput 
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=count_min"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=count_min"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=QPOPSS"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=throughput"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mCMSI_throughput delegation_sketch/example_delegation_heavyhitter_QPOPSS.cpp)
target_compile_definitions(example_mCMSI_throughput PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mCMSI_throughput PRIVATE frequency_estimator_objects delegation_sketch_objects)

# mCMS-q
## throughput 
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=count_min"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=count_min"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=GLOBAL_HASHMAP"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=throughput"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mCMSQ_throughput delegation_sketch/example_delegation_heavyhitter.cpp)
target_compile_definitions(example_mCMSQ_throughput PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mCMSQ_throughput PRIVATE frequency_estimator_objects delegation_sketch_objects)

## mCMS-I latency
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=count_min"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=count_min" 
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=QPOPSS"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=latency"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mCMSI_latency delegation_sketch/example_delegation_heavyhitter_QPOPSS.cpp)
target_compile_definitions(example_mCMSI_latency PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mCMSI_latency PRIVATE frequency_estimator_objects delegation_sketch_objects)

## mCMS-Q latency 
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=count_min"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=count_min"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=GLOBAL_HASHMAP"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=latency"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mCMSQ_latency delegation_sketch/example_delegation_heavyhitter.cpp)
target_compile_definitions(example_mCMSQ_latency PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mCMSQ_latency PRIVATE frequency_estimator_objects delegation_sketch_objects)

# Parallel versions of AS
# mAS-I
## throughput 
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=augmented_sketch"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=augmented_sketch"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=QPOPSS"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=throughput"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mASI_throughput delegation_sketch/example_delegation_heavyhitter_QPOPSS.cpp)
target_compile_definitions(example_mASI_throughput PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mASI_throughput PRIVATE frequency_estimator_objects delegation_sketch_objects)

# mAS-q
## throughput 
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=augmented_sketch"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=augmented_sketch"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=GLOBAL_HASHMAP"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=throughput"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mASQ_throughput delegation_sketch/example_delegation_heavyhitter.cpp)
target_compile_definitions(example_mASQ_throughput PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mASQ_throughput PRIVATE frequency_estimator_objects delegation_sketch_objects)


# AS latency
## mAS-I latency
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=augmented_sketch"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=augmented_sketch"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=QPOPSS"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=latency"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mASI_latency delegation_sketch/example_delegation_heavyhitter_QPOPSS.cpp)
target_compile_definitions(example_mASI_latency PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mASI_latency PRIVATE frequency_estimator_objects delegation_sketch_objects)

## mAS-Q latency
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=augmented_sketch"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=augmented_sketch"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=GLOBAL_HASHMAP"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=latency"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mASQ_latency delegation_sketch/example_delegation_heavyhitter.cpp)
target_compile_definitions(example_mASQ_latency PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mASQ_latency PRIVATE frequency_estimator_objects delegation_sketch_objects)


# Parallel versions of HK
# mHK-I
## throughput 
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=heavy_keeper"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=heavy_keeper"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=QPOPSS"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=throughput"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mHKI_throughput delegation_sketch/example_delegation_heavyhitter_QPOPSS.cpp)
target_compile_definitions(example_mHKI_throughput PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mHKI_throughput PRIVATE frequency_estimator_objects delegation_sketch_objects)

# mHK-q
## throughput 
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=heavy_keeper"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=heavy_keeper"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=GLOBAL_HASHMAP"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=throughput"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mHKQ_throughput delegation_sketch/example_delegation_heavyhitter.cpp)
target_compile_definitions(example_mHKQ_throughput PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mHKQ_throughput PRIVATE frequency_estimator_objects delegation_sketch_objects)


# Parallel versions of SS
# mSS-I
## throughput 
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=heap_hashmap_space_saving"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=heap_hashmap_space_saving"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=QPOPSS"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=throughput"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mSSI_throughput delegation_sketch/example_delegation_heavyhitter_QPOPSS.cpp)
target_compile_definitions(example_mSSI_throughput PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mSSI_throughput PRIVATE frequency_estimator_objects delegation_sketch_objects)

# mSS-q
## throughput 
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=heap_hashmap_space_saving"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=heap_hashmap_space_saving"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=GLOBAL_HASHMAP"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=throughput"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mSSQ_throughput delegation_sketch/example_delegation_heavyhitter.cpp)
target_compile_definitions(example_mSSQ_throughput PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mSSQ_throughput PRIVATE frequency_estimator_objects delegation_sketch_objects)

# SS latency 
## mSS-I latency
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=heap_hashmap_space_saving"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=heap_hashmap_space_saving"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=QPOPSS"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=latency"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mSSI_latency delegation_sketch/example_delegation_heavyhitter_QPOPSS.cpp)
target_compile_definitions(example_mSSI_latency PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mSSI_latency PRIVATE frequency_estimator_objects delegation_sketch_objects)

## mSS-Q latency
set (FREQUENCY_ESTIMATOR_FLAGS
    "ALGORITHM=heap_hashmap_space_saving"
    CACHE STRING "Flags for compiling the frequency estimator" FORCE
)
set(DELEGATION_FLAGS
    "ALGORITHM=heap_hashmap_space_saving"
    "MODE=heavy_hitter"
    "PARALLEL_DESIGN=GLOBAL_HASHMAP"
    "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
    "EVALUATE_MODE=latency"
    "EVALUATE_ACCURACY_WHEN=ivl"
    "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
    CACHE STRING "Flags for compiling the delegation sketch" FORCE
)
add_executable(example_mSSQ_latency delegation_sketch/example_delegation_heavyhitter.cpp)
target_compile_definitions(example_mSSQ_latency PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
target_link_libraries(example_mSSQ_latency PRIVATE frequency_estimator_objects delegation_sketch_objects)

# example_delegation_heavyhitter
# 1. Binary with ALGORITHM = cuckoo_heavy_keeper
# set (FREQUENCY_ESTIMATOR_FLAGS
#     "ALGORITHM=cuckoo_heavy_keeper"
#     CACHE STRING "Flags for compiling the frequency estimator" FORCE
# )
# EVALUATE_ACCURACY_WHEN=start && EVALUATE_ACCURACY_ERROR_SOURCES=algo/algo_df
# EVALUATE_ACCURACY_WHEN=ivl_end && EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous
# set(DELEGATION_FLAGS
#     "ALGORITHM=cuckoo_heavy_keeper"
#     "MODE=heavy_hitter"
#     "PARALLEL_DESIGN=GLOBAL_HASHMAP"
#     # "EVALUATE_MODE=accuracy"
#     "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
#     # "EVALUATE_MODE=throughput"
#     "EVALUATE_MODE=latency"
#     "EVALUATE_ACCURACY_WHEN=start"
#     # "EVALUATE_ACCURACY_WHEN=ivl"
#     # "EVALUATE_ACCURACY_ERROR_SOURCES=algo"
#     # "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df"
#     "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
#     CACHE STRING "Flags for compiling the delegation sketch" FORCE
# )
# add_executable(example_delegation_heavyhitter_cuckoo_heavy_keeper delegation_sketch/example_delegation_heavyhitter.cpp)
# target_compile_definitions(example_delegation_heavyhitter_cuckoo_heavy_keeper PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
# target_link_libraries(example_delegation_heavyhitter_cuckoo_heavy_keeper PRIVATE frequency_estimator_objects delegation_sketch_objects)

# example_delegation_heavyhitter_QPOPSS
# 1. Binary with ALGORITHM = cuckoo_heavy_keeper
# set (FREQUENCY_ESTIMATOR_FLAGS
#     "ALGORITHM=cuckoo_heavy_keeper"
#     CACHE STRING "Flags for compiling the frequency estimator" FORCE
# )
# EVALUATE_ACCURACY_WHEN=start && EVALUATE_ACCURACY_ERROR_SOURCES=algo/algo_df
# EVALUATE_ACCURACY_WHEN=ivl_end && EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous
# set(DELEGATION_FLAGS
#     "ALGORITHM=cuckoo_heavy_keeper"
#     "MODE=heavy_hitter"
#     # "PARALLEL_DESIGN=GLOBAL_HASHMAP"
#     "PARALLEL_DESIGN=QPOPSS"
#     # "EVALUATE_MODE=accuracy"
#     "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
#     "EVALUATE_MODE=throughput"
#     # "EVALUATE_MODE=latency"
#     # "EVALUATE_ACCURACY_WHEN=start"
#     "EVALUATE_ACCURACY_WHEN=ivl"
#     # "EVALUATE_ACCURACY_ERROR_SOURCES=algo"
#     # "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df"
#     "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
#     CACHE STRING "Flags for compiling the delegation sketch" FORCE
# )
# add_executable(example_delegation_heavyhitter_cuckoo_heavy_keeper_QPOPSS delegation_sketch/example_delegation_heavyhitter_QPOPSS.cpp)
# target_compile_definitions(example_delegation_heavyhitter_cuckoo_heavy_keeper_QPOPSS PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
# target_link_libraries(example_delegation_heavyhitter_cuckoo_heavy_keeper_QPOPSS PRIVATE frequency_estimator_objects delegation_sketch_objects)







# # concurrent data structures
# # create an executable for concurrent_data_structure/example_treiber_stack.cpp
# add_executable(example_treiber_stack concurrent_data_structure/example_treiber_stack.cpp)

# # create an executable for concurrent_data_structure/example_ms_queue.cpp
# add_executable(example_ms_queue concurrent_data_structure/example_ms_queue.cpp)

# # create an executable for concurrent_data_structure/example_lcr_queue.cpp
# add_executable(example_lcr_queue concurrent_data_structure/example_lcr_queue.cpp)

# # create an executable for concurrent_data_structure/example_lockbased_concurrent_heap.cpp 
# add_executable(example_lockbased_concurrent_heap concurrent_data_structure/example_lockbased_concurrent_heap.cpp)

# # delegation sketch 
# # 1. Binary with ALGORITHM = count_min
# add_executable(example_delegation_sketch_count_min delegation_sketch/example_delegation_sketch.cpp)
# target_compile_definitions(example_delegation_sketch_count_min PRIVATE ALGORITHM=count_min)
# # target_link_libraries(example_delegation_sketch_count_min PRIVATE frequency_estimator_objects delegation_sketch_objects)

# # 2. Binary with ALGORITHM = augmented_sketch
# add_executable(example_delegation_sketch_augmented_sketch delegation_sketch/example_delegation_sketch.cpp)
# target_compile_definitions(example_delegation_sketch_augmented_sketch PRIVATE ALGORITHM=augmented_sketch)
# # target_link_libraries(example_delegation_sketch_augmented_sketch PRIVATE frequency_estimator_objects delegation_sketch_objects)


# # 3. Binary with ALGORITHM = cuckoo_heavy_keeper
# set (FREQUENCY_ESTIMATOR_FLAGS
#     "ALGORITHM=cuckoo_heavy_keeper"
#     CACHE STRING "Flags for compiling the frequency estimator" FORCE
# )
# # set(DELEGATION_FLAGS
# #     "ALGORITHM=cuckoo_heavy_keeper"
# #     "MODE=heavy_hitter"
# #     "EVALUATE_MODE=accuracy"
# #     # "EVALUATE_MODE=throughput"
# #     # "EVALUATE_ACCURACY_WHEN=start"
# #     "EVALUATE_ACCURACY_WHEN=ivl_end"
# #     # "EVALUATE_ACCURACY_ERROR_SOURCES=algo"
# #     "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
# #     CACHE STRING "Flags for compiling the delegation sketch" FORCE
# # )
# add_executable(example_delegation_sketch_cuckoo_heavy_keeper delegation_sketch/example_delegation_sketch.cpp)
# target_compile_definitions(example_delegation_sketch_cuckoo_heavy_keeper PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
# # target_link_libraries(example_delegation_sketch_cuckoo_heavy_keeper PRIVATE frequency_estimator_objects delegation_sketch_objects)

# # 4. Binary with ALGORITHM = cuckoo_heavy_keeper
# set (FREQUENCY_ESTIMATOR_FLAGS
#     "ALGORITHM=cuckoo_heavy_keeper"
#     CACHE STRING "Flags for compiling the frequency estimator" FORCE
# )
# # EVALUATE_ACCURACY_WHEN=start && EVALUATE_ACCURACY_ERROR_SOURCES=algo/algo_df
# # EVALUATE_ACCURACY_WHEN=ivl_end && EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous
# set(DELEGATION_FLAGS
#     "ALGORITHM=cuckoo_heavy_keeper"
#     "MODE=heavy_hitter"
#     "PARALLEL_DESIGN=GLOBAL_HASHMAP"
#     # "EVALUATE_MODE=accuracy"
#     "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
#     "EVALUATE_MODE=throughput"
#     # "EVALUATE_ACCURACY_WHEN=start"
#     "EVALUATE_ACCURACY_WHEN=ivl"
#     # "EVALUATE_ACCURACY_ERROR_SOURCES=algo"
#     # "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df"
#     "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
#     CACHE STRING "Flags for compiling the delegation sketch" FORCE
# )
# add_executable(example_delegation_heavyhitter_cuckoo_heavy_keeper delegation_sketch/example_delegation_heavyhitter.cpp)
# target_compile_definitions(example_delegation_heavyhitter_cuckoo_heavy_keeper PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
# # target_link_libraries(example_delegation_heavyhitter_cuckoo_heavy_keeper PRIVATE frequency_estimator_objects delegation_sketch_objects)

# set(DELEGATION_FLAGS
#     "ALGORITHM=cuckoo_heavy_keeper"
#     "MODE=heavy_hitter"
#     "PARALLEL_DESIGN=QPOPSS"
#     # "EVALUATE_MODE=accuracy"
#     "EVALUATE_ACCURACY_STREAM_SIZE=1000000"
#     # "EVALUATE_MODE=throughput"
#     "EVALUATE_MODE=throughput_while_querying"
#     # "EVALUATE_ACCURACY_WHEN=start"
#     "EVALUATE_ACCURACY_WHEN=ivl"
#     # "EVALUATE_ACCURACY_ERROR_SOURCES=algo"
#     # "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df"
#     "EVALUATE_ACCURACY_ERROR_SOURCES=algo_df_continuous"
#     CACHE STRING "Flags for compiling the delegation sketch" FORCE
# )
# add_executable(example_delegation_heavyhitter_cuckoo_heavy_keeper_QPOPSS delegation_sketch/example_delegation_heavyhitter_QPOPSS.cpp)
# target_compile_definitions(example_delegation_heavyhitter_cuckoo_heavy_keeper_QPOPSS PRIVATE ${DELEGATION_FLAGS} ${FREQUENCY_ESTIMATOR_FLAGS})
# # target_link_libraries(example_delegation_heavyhitter_cuckoo_heavy_keeper_QPOPSS PRIVATE frequency_estimator_objects delegation_sketch_objects)

# # 4. Binary for example_delegation_test
# # set(DELEGATION_FLAGS
# #     "ALGORITHM=cuckoo_heavy_keeper"
# #     "MODE=heavy_hitter"
# #     "EVALUATE_MODE=accuracy"
# #     "EVALUATE_ACCURACY_WHEN=start"
# #     "EVALUATE_ACCURACY_ERROR_SOURCES=algo"
# #     CACHE STRING "Flags for compiling the delegation sketch" FORCE
# # )
# add_executable(example_delegation_test delegation_sketch/example_delegation_test.cpp)
# target_compile_definitions(example_delegation_test PRIVATE ALGORITHM=cuckoo_heavy_keeper)
# target_compile_definitions(example_delegation_test PRIVATE ${DELEGATION_FLAGS})
# # target_link_libraries(example_delegation_test PRIVATE frequency_estimator_objects delegation_sketch_objects)

# # prif 
# # 1. Binary with ALGORITHM = weighted_frequent
# add_executable(example_prif_weighted_frequent prif/example_prif.cpp)
# target_compile_definitions(example_prif_weighted_frequent PRIVATE ALGORITHM=weighted_frequent)
# # target_link_libraries(example_prif_weighted_frequent PRIVATE frequency_estimator_objects delegation_sketch_objects)

# # test.cpp 
# # add_executable(test test/test.cpp)
# # target_link_libraries(test PRIVATE frequency_estimator_objects delegation_sketch_objects)
