# Find LightGBM
find_package(LightGBM CONFIG)
if(LightGBM_FOUND)
    message(STATUS "Found LightGBM: ${LightGBM_INCLUDE_DIRS}")
    include_directories(${LightGBM_INCLUDE_DIRS})
else()
    message(WARNING "LightGBM not found. Please install it or set LightGBM_DIR.")
endif()

# Find fastText (assuming it's installed or provided via a path)
find_package(fastText CONFIG)
if(fastText_FOUND)
    message(STATUS "Found fastText: ${fastText_INCLUDE_DIRS}")
    include_directories(${fastText_INCLUDE_DIRS})
else()
    message(WARNING "fastText not found. Please install it or set fastText_DIR.")
endif()

# Include directories for the entire project
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Define the parser library
set(PARSER_LIB_SOURCES
    src/process_scanner.cpp
    src/proc_tokenizer.cpp
    src/proc_metrics.cpp
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

# Define the classifier executable
add_executable(classifier src/classifier.cpp)

# Link classifier with parser and dl
# Define the ML inference library
add_library(ml_inference_lib STATIC
    src/ml_inference.cpp
)

target_link_libraries(ml_inference_lib
    PRIVATE
        _lightgbm # Corrected library name
        fasttext
)

target_include_directories(ml_inference_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link classifier with parser, dl, and ml_inference_lib
target_link_libraries(classifier dl parser ml_inference_lib _lightgbm fasttext) # Corrected library name

# Installation rules
install(TARGETS parser DESTINATION lib)
install(TARGETS classifier RUNTIME DESTINATION bin)
install(TARGETS ml_inference_lib DESTINATION lib)
install(FILES Configs/IgnoreTokens.txt DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/classifier)
install(FILES Configs/ignore_processes.txt DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/classifier)
install(FILES ${CMAKE_SOURCE_DIR}/artifacts/fasttext_model.bin DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/classifier)
install(FILES ${CMAKE_SOURCE_DIR}/artifacts/lgbm_model.txt DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/classifier)
install(FILES ${CMAKE_SOURCE_DIR}/artifacts/meta.json DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/classifier)
install(FILES ${CMAKE_SOURCE_DIR}/Services/classifier.service DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system/)
