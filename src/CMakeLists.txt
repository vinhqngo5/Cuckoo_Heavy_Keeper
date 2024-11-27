# module 1
# Collect all module sources
file(GLOB_RECURSE MODULE1_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/module1/*.cpp")

# Create an object library for module1
add_library(module1_objects OBJECT ${MODULE1_SOURCES})

# frequency_estimator
# Collect all frequency_estimator sources
file(GLOB_RECURSE FREQUENCY_ESTIMATOR_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/frequency_estimator/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/hash/*.cpp")

# Create an object library for frequency_estimator
add_library(frequency_estimator_objects OBJECT ${FREQUENCY_ESTIMATOR_SOURCES})
target_compile_definitions(frequency_estimator_objects PRIVATE ${FREQUENCY_ESTIMATOR_FLAGS})

# delegation_sketch
# Collect all delegation_sketch sources
file(GLOB_RECURSE DELEGATION_SKETCH_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/delegation_sketch/*.cpp")

# Create an object library for delegation_sketch
add_library(delegation_sketch_objects OBJECT ${DELEGATION_SKETCH_SOURCES})
target_compile_definitions(delegation_sketch_objects PRIVATE ${DELEGATION_FLAGS})
target_compile_definitions(delegation_sketch_objects PRIVATE ${FREQUENCY_ESTIMATOR_FLAGS})
