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

# ............
