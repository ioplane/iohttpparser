# ============================================================================
# iohttpparser — High-performance HTTP/1.1 parser for C23
# SIMD-accelerated · RFC 9112 · Pull-based incremental API
# ============================================================================

cmake_minimum_required(VERSION 3.25)

project(iohttpparser
    VERSION 0.1.0
    DESCRIPTION "High-performance HTTP/1.1 parser for C23 with SIMD acceleration"
    HOMEPAGE_URL "https://github.com/ioplane/iohttpparser"
    LANGUAGES C
)

# ============================================================================
# C23 Standard (mandatory)
# ============================================================================

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ============================================================================
# Build Options
# ============================================================================

option(IOHTTPPARSER_BUILD_TESTS "Build unit tests" ON)
option(IOHTTPPARSER_BUILD_BENCH "Build benchmarks" OFF)
option(IOHTTPPARSER_BUILD_FUZZ "Build fuzz targets (Clang only)" OFF)
option(IOHTTPPARSER_BUILD_EXAMPLES "Build examples" OFF)
option(IOHTTPPARSER_INSTALL "Generate install targets" ON)

# SIMD options (auto-detected at compile time, runtime dispatch)
option(IOHTTPPARSER_SIMD_SSE42 "Enable SSE4.2 scanner" ON)
option(IOHTTPPARSER_SIMD_AVX2 "Enable AVX2 scanner" ON)

# ============================================================================
# Compile Options
# ============================================================================

add_compile_options(
    -Wall -Wextra -Wpedantic -Werror
    -Wformat=2
    -Wconversion -Wsign-conversion
    -Wdouble-promotion -Wfloat-equal
    -Wshadow -Wundef
    -Wunused -Wuninitialized
    -Wnull-dereference
    -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations
    -Wredundant-decls
    -Wvla
    -fstack-protector-strong
    -fPIC
)

# GCC-specific warning flags (not supported by Clang)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
        -Wformat-overflow=2 -Wformat-truncation=2
    )
endif()

# _FORTIFY_SOURCE requires optimization; only enable for Release/RelWithDebInfo
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(_FORTIFY_SOURCE=3)
endif()

# ============================================================================
# _GNU_SOURCE (required for explicit_bzero, etc.)
# ============================================================================

add_compile_definitions(_GNU_SOURCE)

# ============================================================================
# Library Sources
# ============================================================================

set(IOHTTPPARSER_SOURCES
    src/ihtp_scanner_scalar.c
    src/ihtp_parser.c
    src/ihtp_semantics.c
    src/ihtp_body_decoder.c
)

# SIMD scanner sources (compiled with appropriate -m flags)
if(IOHTTPPARSER_SIMD_SSE42)
    list(APPEND IOHTTPPARSER_SOURCES src/ihtp_scanner_sse42.c)
endif()

if(IOHTTPPARSER_SIMD_AVX2)
    list(APPEND IOHTTPPARSER_SOURCES src/ihtp_scanner_avx2.c)
endif()

# ============================================================================
# Library Targets
# ============================================================================

# Static library (primary)
add_library(iohttpparser STATIC ${IOHTTPPARSER_SOURCES})
target_include_directories(iohttpparser
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# Shared library
add_library(iohttpparser_shared SHARED ${IOHTTPPARSER_SOURCES})
set_target_properties(iohttpparser_shared PROPERTIES
    OUTPUT_NAME iohttpparser
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)
target_include_directories(iohttpparser_shared
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# SIMD compile flags per source file
if(IOHTTPPARSER_SIMD_SSE42)
    set_source_files_properties(src/ihtp_scanner_sse42.c PROPERTIES
        COMPILE_FLAGS "-msse4.2"
    )
    target_compile_definitions(iohttpparser PRIVATE IOHTTPPARSER_HAVE_SSE42)
    target_compile_definitions(iohttpparser_shared PRIVATE IOHTTPPARSER_HAVE_SSE42)
endif()

if(IOHTTPPARSER_SIMD_AVX2)
    set_source_files_properties(src/ihtp_scanner_avx2.c PROPERTIES
        COMPILE_FLAGS "-mavx2"
    )
    target_compile_definitions(iohttpparser PRIVATE IOHTTPPARSER_HAVE_AVX2)
    target_compile_definitions(iohttpparser_shared PRIVATE IOHTTPPARSER_HAVE_AVX2)
endif()

# Alias for use via add_subdirectory()
add_library(iohttpparser::iohttpparser ALIAS iohttpparser)

# ============================================================================
# Testing
# ============================================================================

if(IOHTTPPARSER_BUILD_TESTS)
    enable_testing()

    # Unity test framework (compile from source)
    find_path(UNITY_INCLUDE_DIR unity/unity.h
        PATHS /usr/local/include /usr/include
    )
    find_path(UNITY_SRC_DIR unity.c
        PATHS /usr/local/src/unity /usr/src/unity
    )

    if(UNITY_INCLUDE_DIR AND UNITY_SRC_DIR)
        message(STATUS "Found Unity: ${UNITY_SRC_DIR}/unity.c")
        add_library(unity STATIC ${UNITY_SRC_DIR}/unity.c)
        target_include_directories(unity PUBLIC ${UNITY_INCLUDE_DIR} ${UNITY_INCLUDE_DIR}/unity)
    else()
        message(WARNING "Unity test framework not found — tests disabled")
    endif()

    # Test helper macro
    macro(ihtp_add_test TEST_NAME TEST_SRC)
        if(TARGET unity)
            add_executable(${TEST_NAME} ${TEST_SRC})
            target_include_directories(${TEST_NAME} PRIVATE
                ${CMAKE_SOURCE_DIR}/src
            )
            target_link_libraries(${TEST_NAME} PRIVATE
                unity
                iohttpparser
                ${ARGN}
            )
            # Unity test functions lack prototypes by design
            target_compile_options(${TEST_NAME} PRIVATE
                -Wno-missing-prototypes -Wno-missing-declarations
            )
            add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        endif()
    endmacro()

    ihtp_add_test(test_scanner tests/unit/test_scanner.c)
    ihtp_add_test(test_parser tests/unit/test_parser.c)
    ihtp_add_test(test_semantics tests/unit/test_semantics.c)
    ihtp_add_test(test_body_decoder tests/unit/test_body_decoder.c)
endif()

# ============================================================================
# Fuzz Testing (Clang only)
# ============================================================================

if(IOHTTPPARSER_BUILD_FUZZ AND CMAKE_C_COMPILER_ID STREQUAL "Clang")
    macro(ihtp_add_fuzz FUZZ_NAME FUZZ_SRC)
        add_executable(${FUZZ_NAME} ${FUZZ_SRC})
        target_include_directories(${FUZZ_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
        target_link_libraries(${FUZZ_NAME} PRIVATE -fsanitize=fuzzer iohttpparser ${ARGN})
        target_compile_options(${FUZZ_NAME} PRIVATE -fsanitize=fuzzer-no-link)
    endmacro()

    ihtp_add_fuzz(fuzz_parser tests/fuzz/fuzz_parser.c)
    ihtp_add_fuzz(fuzz_chunked tests/fuzz/fuzz_chunked.c)
endif()

# ============================================================================
# Benchmarks
# ============================================================================

if(IOHTTPPARSER_BUILD_BENCH)
    add_executable(bench_parser bench/bench_parser.c)
    target_link_libraries(bench_parser PRIVATE iohttpparser)
endif()

# ============================================================================
# Examples
# ============================================================================

if(IOHTTPPARSER_BUILD_EXAMPLES)
    add_executable(example_basic examples/basic_parse.c)
    target_link_libraries(example_basic PRIVATE iohttpparser)
endif()

# ============================================================================
# Code Quality Targets
# ============================================================================

# clang-format
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    file(GLOB_RECURSE ALL_C_SOURCES
        ${CMAKE_SOURCE_DIR}/src/*.c
        ${CMAKE_SOURCE_DIR}/src/*.h
        ${CMAKE_SOURCE_DIR}/include/*.h
        ${CMAKE_SOURCE_DIR}/tests/*.c
    )
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${ALL_C_SOURCES}
        COMMENT "Running clang-format"
        VERBATIM
    )
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT} --dry-run --Werror ${ALL_C_SOURCES}
        COMMENT "Checking clang-format compliance"
        VERBATIM
    )
endif()

# clang-tidy
find_program(CLANG_TIDY clang-tidy)
if(CLANG_TIDY)
    add_custom_target(lint
        COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR}
            ${CMAKE_SOURCE_DIR}/src/*.c
        COMMENT "Running clang-tidy"
        VERBATIM
    )
endif()

# cppcheck
find_program(CPPCHECK cppcheck)
if(CPPCHECK)
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK}
            --enable=all --inconclusive --std=c23
            --suppress=missingIncludeSystem
            -I ${CMAKE_SOURCE_DIR}/src
            -I ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src/
        COMMENT "Running cppcheck"
        VERBATIM
    )
endif()

# PVS-Studio
find_program(PVS_STUDIO pvs-studio-analyzer)
find_program(PLOG_CONVERTER plog-converter)
if(PVS_STUDIO AND PLOG_CONVERTER)
    add_custom_target(pvs-studio
        COMMAND ${PVS_STUDIO} analyze
            -f ${CMAKE_BINARY_DIR}/compile_commands.json
            -o ${CMAKE_BINARY_DIR}/pvs-studio.log
            -j 4
        COMMAND ${PLOG_CONVERTER}
            -t errorfile
            -a "GA:1,2"
            ${CMAKE_BINARY_DIR}/pvs-studio.log
        COMMENT "Running PVS-Studio analysis (GA errors + warnings)"
        VERBATIM
    )
endif()

# CodeChecker
find_program(CODECHECKER CodeChecker)
if(CODECHECKER)
    add_custom_target(codechecker
        COMMAND ${CODECHECKER} analyze
            ${CMAKE_BINARY_DIR}/compile_commands.json
            -o ${CMAKE_BINARY_DIR}/cc-results
            --analyzers clangsa clang-tidy
            -j 4
        COMMAND ${CODECHECKER} parse
            ${CMAKE_BINARY_DIR}/cc-results
            --trim-path-prefix ${CMAKE_SOURCE_DIR}/
        COMMENT "Running CodeChecker analysis (Clang SA + clang-tidy)"
        VERBATIM
    )
endif()

# Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# ============================================================================
# Install
# ============================================================================

if(IOHTTPPARSER_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    # Install library targets
    install(TARGETS iohttpparser iohttpparser_shared
        EXPORT iohttpparserTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    # Install public headers
    install(DIRECTORY include/iohttpparser
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # pkg-config
    configure_file(
        ${CMAKE_SOURCE_DIR}/iohttpparser.pc.in
        ${CMAKE_BINARY_DIR}/iohttpparser.pc
        @ONLY
    )
    install(FILES ${CMAKE_BINARY_DIR}/iohttpparser.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )

    # CMake package config
    install(EXPORT iohttpparserTargets
        FILE iohttpparserTargets.cmake
        NAMESPACE iohttpparser::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/iohttpparser
    )

    write_basic_package_version_file(
        ${CMAKE_BINARY_DIR}/iohttpparserConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        ${CMAKE_SOURCE_DIR}/cmake/iohttpparserConfig.cmake.in
        ${CMAKE_BINARY_DIR}/iohttpparserConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/iohttpparser
    )

    install(FILES
        ${CMAKE_BINARY_DIR}/iohttpparserConfig.cmake
        ${CMAKE_BINARY_DIR}/iohttpparserConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/iohttpparser
    )
endif()
