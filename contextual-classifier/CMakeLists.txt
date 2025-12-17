cmake_minimum_required(VERSION 3.15)

# Main project for the classifier daemon
project(classifier_daemon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_options(-Wall -Wextra)

# Find fastText (assuming it's installed or provided via a path)
#find_package(fasttext CONFIG)
#if(fastText_FOUND)
#    message(STATUS "Found fastText: ${fastText_INCLUDE_DIRS}")
#    include_directories(${fastText_INCLUDE_DIRS})
#else()
#    message(WARNING "fastText not found. Please install it or set fastText_DIR.")
#endif()

# Include directories for the entire project
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Define the parser library
set(PARSER_LIB_SOURCES
    src/process_scanner.cpp
    src/proc_tokenizer.cpp
)

add_library(parser SHARED ${PARSER_LIB_SOURCES})

set_target_properties(parser PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
)

target_include_directories(parser
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Define the ML inference library
add_library(ml_inference_lib STATIC
    src/ml_inference.cpp
)

target_link_libraries(ml_inference_lib
    PRIVATE
        fasttext
)

target_include_directories(ml_inference_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Installation rules
install(TARGETS parser DESTINATION lib)
install(TARGETS ml_inference_lib DESTINATION lib)
install(FILES Configs/ignore-tokens.txt DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/classifier)
install(FILES Configs/classifier-blocklist.txt DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/classifier)
install(FILES ${CMAKE_SOURCE_DIR}/contextual-classifier/artifacts/fasttext_model_supervised.bin DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/classifier)
