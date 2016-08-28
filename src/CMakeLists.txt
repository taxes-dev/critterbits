add_executable(critterbits main.cpp engine.cpp engineconfiguration.cpp)
target_link_libraries(critterbits ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_TTF_LIBRARY} ${LIBYAML_LIBRARY})
install(TARGETS critterbits RUNTIME DESTINATION ${BIN_DIR})