add_executable(critterbits
    main.cpp engine.cpp engineconfiguration.cpp files.cpp
    scene.cpp scenemanager.cpp tilemap.cpp yaml.cpp)
target_link_libraries(critterbits
    ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_TTF_LIBRARY}
    ${LIBYAML_LIBRARY} ${LIBTMX_LIBRARY} ${ZLIB_LIBRARIES} ${LIBXML2_LIBRARIES})
install(TARGETS critterbits RUNTIME DESTINATION ${BIN_DIR})