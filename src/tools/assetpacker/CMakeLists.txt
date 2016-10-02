add_executable(assetpacker
    main.cpp $<TARGET_OBJECTS:critterbits-toml>)
target_link_libraries(assetpacker stdc++fs
    ${ZSTD_LIBRARIES})