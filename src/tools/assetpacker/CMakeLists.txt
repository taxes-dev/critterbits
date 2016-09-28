add_executable(assetpacker main.cpp)
target_link_libraries(assetpacker
    ${ZSTD_LIBRARIES})