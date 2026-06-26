cmake_minimum_required(VERSION 3.20)
project(ByteHound VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TINYCFG_BUILD_CLI "Build tinycfg-cli executable" ON)
option(TINYCFG_BUILD_TESTS "Build unit tests" ON)
option(TINYCFG_BUILD_FUZZERS "Build libFuzzer targets" OFF)
option(TINYCFG_ENABLE_SANITIZERS "Enable ASan/UBSan for non-fuzzer targets" OFF)
option(TINYCFG_ENABLE_LEGACY_TRAINING_FIXTURE
       "Build intentionally vulnerable educational legacy parser fixture" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Sanitizers)

add_library(tinycfg
    src/ast.cpp
    src/lexer.cpp
    src/parser.cpp
    src/serializer.cpp
)
add_library(tinycfg::tinycfg ALIAS tinycfg)

target_include_directories(tinycfg
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

tinycfg_apply_warnings(tinycfg)

if(TINYCFG_ENABLE_SANITIZERS AND NOT TINYCFG_BUILD_FUZZERS)
    tinycfg_apply_sanitizers(tinycfg)
elseif(TINYCFG_BUILD_FUZZERS)
    target_compile_options(tinycfg PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -g
    )
    target_link_options(tinycfg PRIVATE -fsanitize=address,undefined)
endif()

if(TINYCFG_BUILD_CLI)
    add_executable(tinycfg-cli src/cli.cpp)
    target_link_libraries(tinycfg-cli PRIVATE tinycfg::tinycfg)
    tinycfg_apply_warnings(tinycfg-cli)
    if(TINYCFG_ENABLE_SANITIZERS AND NOT TINYCFG_BUILD_FUZZERS)
        tinycfg_apply_sanitizers(tinycfg-cli)
    endif()
endif()

if(TINYCFG_BUILD_TESTS)
    enable_testing()
    add_executable(tinycfg_tests
        tests/test_main.cpp
        tests/lexer_tests.cpp
        tests/parser_tests.cpp
        tests/serializer_tests.cpp
        tests/regression_tests.cpp
    )
    target_link_libraries(tinycfg_tests PRIVATE tinycfg::tinycfg)
    target_include_directories(tinycfg_tests PRIVATE tests)
    target_compile_definitions(tinycfg_tests PRIVATE TINYCFG_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
    tinycfg_apply_warnings(tinycfg_tests)
    if(TINYCFG_ENABLE_SANITIZERS AND NOT TINYCFG_BUILD_FUZZERS)
        tinycfg_apply_sanitizers(tinycfg_tests)
    endif()
    add_test(NAME tinycfg_unit_tests COMMAND tinycfg_tests)
endif()

function(tinycfg_add_fuzzer target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE tinycfg::tinycfg)
    tinycfg_apply_warnings(${target})

    if(DEFINED ENV{LIB_FUZZING_ENGINE})
        string(STRIP "$ENV{LIB_FUZZING_ENGINE}" _fuzzer_engine)
    else()
        set(_fuzzer_engine "")
    endif()

    if(NOT _fuzzer_engine STREQUAL "")
        separate_arguments(_fuzzer_flags NATIVE_COMMAND "${_fuzzer_engine}")
        target_compile_options(${target} PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
            -g
        )
        target_link_options(${target} PRIVATE
            -fsanitize=address,undefined
            ${_fuzzer_flags}
        )
    else()
        tinycfg_apply_fuzzer_sanitizers(${target})
    endif()
endfunction()

if(TINYCFG_BUILD_FUZZERS)
    tinycfg_add_fuzzer(fuzz_lexer fuzz/fuzz_lexer.cpp)
    tinycfg_add_fuzzer(fuzz_parser fuzz/fuzz_parser.cpp)
    tinycfg_add_fuzzer(fuzz_roundtrip fuzz/fuzz_roundtrip.cpp)
    tinycfg_add_fuzzer(fuzz_mutation_sequence fuzz/fuzz_mutation_sequence.cpp)
endif()

if(TINYCFG_ENABLE_LEGACY_TRAINING_FIXTURE)
    enable_testing()

    add_library(legacy_parser
        training/legacy_parser/legacy_parser.cpp
    )
    target_include_directories(legacy_parser
        PUBLIC training/legacy_parser
    )
    tinycfg_apply_warnings(legacy_parser)

    add_executable(fuzz_legacy_parser training/legacy_parser/fuzz_legacy_parser.cpp)
    target_link_libraries(fuzz_legacy_parser PRIVATE legacy_parser)
    tinycfg_apply_warnings(fuzz_legacy_parser)
    tinycfg_apply_fuzzer_sanitizers(fuzz_legacy_parser)

    add_executable(legacy_parser_tests
        tests/test_main.cpp
        training/legacy_parser/legacy_parser_tests.cpp
    )
    target_link_libraries(legacy_parser_tests PRIVATE legacy_parser tinycfg::tinycfg)
    target_include_directories(legacy_parser_tests PRIVATE tests)
    target_compile_definitions(legacy_parser_tests PRIVATE TINYCFG_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
    tinycfg_apply_warnings(legacy_parser_tests)
    if(TINYCFG_ENABLE_SANITIZERS AND NOT TINYCFG_BUILD_FUZZERS)
        tinycfg_apply_sanitizers(legacy_parser_tests)
    endif()
    add_test(NAME legacy_parser_tests COMMAND legacy_parser_tests)
endif()
