cmake_minimum_required(VERSION 3.20)
project(ctc_simulator VERSION 0.1 LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(LOG_DIR ${CMAKE_SOURCE_DIR}/logs)
set(DATA_DIR ${CMAKE_SOURCE_DIR}/data)

# =============================
# zip extractor
# =============================
set(RUST_EXTRACTOR_DIR ${CMAKE_SOURCE_DIR}/data_pipeline/zip_extractor)

add_custom_command(
    OUTPUT ${RUST_EXTRACTOR_DIR}/target/release/zip_extractor
    COMMAND cargo build --release --manifest-path ${RUST_EXTRACTOR_DIR}/Cargo.toml
    WORKING_DIRECTORY ${RUST_EXTRACTOR_DIR}
    COMMENT "building rust zip extractor..."
)

add_custom_target(build_zip_extractor DEPENDS ${RUST_EXTRACTOR_DIR}/target/release/zip_extractor)

add_custom_target(unzip_data ALL
    COMMAND ${RUST_EXTRACTOR_DIR}/target/release/zip_extractor
    DEPENDS build_zip_extractor
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "running rust zip extractor..."
)


# =============================
# extract, transfer, load (etl)
# =============================

file(GLOB_RECURSE ETL_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/data_pipeline/etl/*.cpp
)
add_executable(etl ${ETL_SOURCES})
target_include_directories(etl PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/data_pipeline/etl/include
)
set_target_properties(etl PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_compile_definitions(etl PRIVATE 
    LOG_DIR=\"${LOG_DIR}\"
    DATA_DIR=\"${DATA_DIR}\"
)

add_custom_target(run_preprocessing ALL
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/etl
    DEPENDS etl unzip_data
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "running c++ etl preprocessing..."
)


# =============================
# app
# =============================
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)
add_executable(app ${SOURCES})
target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR}/include)
set_target_properties(app PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

target_compile_definitions(app PRIVATE 
    LOG_DIR=\"${LOG_DIR}\"
    DATA_DIR=\"${DATA_DIR}\"
)

add_dependencies(app run_preprocessing)


# =============================
# tests
# =============================
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/tests/*.cpp
)
file(GLOB_RECURSE PRODUCTION_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)
list(FILTER PRODUCTION_SOURCES EXCLUDE REGEX "main.cpp")
add_executable(ctc_tests ${TEST_SOURCES} ${PRODUCTION_SOURCES})
target_include_directories(ctc_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ctc_tests PRIVATE gmock_main)
set_target_properties(ctc_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

target_compile_definitions(ctc_tests PRIVATE 
    LOG_DIR=\"${LOG_DIR}\"
    DATA_DIR=\"${DATA_DIR}\"
)

add_dependencies(ctc_tests run_preprocessing)

include(GoogleTest)
gtest_discover_tests(ctc_tests)