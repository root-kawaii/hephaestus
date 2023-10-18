set(CMAKE_CXX_STANDARD 17)

add_library (asset_core STATIC 
    asset_main.cpp
)

target_include_directories(asset_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(asset_core PRIVATE json lz4)