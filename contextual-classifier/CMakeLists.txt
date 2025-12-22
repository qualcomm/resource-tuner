# Include directories for the entire project
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Include)
include_directories(${CMAKE_SOURCE_DIR}/modula/Components/Include)
include_directories(${CMAKE_SOURCE_DIR}/modula/Common/Include)
include_directories(${CMAKE_SOURCE_DIR}/modula/CoreModules/Include)

# Define the parser library
set(PARSER_LIB_SOURCES
    FeatureExtractor.cpp
    FeaturePruner.cpp
)

add_library(parser SHARED ${PARSER_LIB_SOURCES})

set_target_properties(parser PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
)

target_include_directories(parser
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

# Define the ML inference library
add_library(ml_inference_lib STATIC
    MLInference.cpp
)

target_link_libraries(ml_inference_lib
    PRIVATE
        fasttext
)

target_include_directories(ml_inference_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

# Installation rules
install(TARGETS parser DESTINATION lib)
install(TARGETS ml_inference_lib DESTINATION lib)
install(FILES Configs/ignore-tokens.txt DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/classifier)
install(FILES Configs/classifier-blocklist.txt DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/classifier)
install(FILES ${CMAKE_SOURCE_DIR}/contextual-classifier/Artifacts/fasttext_model_supervised.bin DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/classifier)
