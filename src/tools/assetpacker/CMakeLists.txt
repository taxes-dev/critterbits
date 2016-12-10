add_executable(assetpacker
    main.cpp $<TARGET_OBJECTS:critterbits-toml>)
target_link_libraries(assetpacker ${ZSTD_LIBRARIES})
if(WIN32)
    target_link_libraries(assetpacker wsock32 ws2_32)
elseif(UNIX)
    target_link_libraries(assetpacker stdc++fs)
endif()