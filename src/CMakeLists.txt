add_executable(critterbits
    main.cpp animation.cpp engine.cpp engineconfiguration.cpp enginecounters.cpp
    engineeventqueue.cpp entity.cpp guipanel.cpp inputmanager.cpp memory.cpp
    rectregioncombiner.cpp rendering.cpp resourceloader.cpp scene.cpp
    scenemanager.cpp script.cpp scriptengine.cpp scriptsupport.cpp sprite.cpp
    spritemanager.cpp tilemap.cpp tilemapregion.cpp tilesetimagemanager.cpp
    tomlparser.cpp viewport.cpp $<TARGET_OBJECTS:duktape>)
target_link_libraries(critterbits
    ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_GFX_LIBRARY}
    ${TMXPARSER_LIBRARIES} ${TINYXML2_LIBRARIES})
install(TARGETS critterbits RUNTIME DESTINATION ${BIN_DIR})