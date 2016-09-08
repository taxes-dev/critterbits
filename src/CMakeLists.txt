add_executable(critterbits
    main.cpp engine.cpp engineconfiguration.cpp enginecounters.cpp
    engineeventqueue.cpp entity.cpp files.cpp inputmanager.cpp
    rectregioncombiner.cpp rendering.cpp scene.cpp scenemanager.cpp
    script.cpp scriptengine.cpp sprite.cpp spritemanager.cpp tilemap.cpp
    tilemapregion.cpp viewport.cpp yaml.cpp $<TARGET_OBJECTS:duktape>)
target_link_libraries(critterbits
    ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_TTF_LIBRARY} ${SDL2_GFX_LIBRARY}
    ${LIBYAML_LIBRARY} ${LIBTMX_LIBRARY} ${ZLIB_LIBRARIES} ${LIBXML2_LIBRARIES})
install(TARGETS critterbits RUNTIME DESTINATION ${BIN_DIR})