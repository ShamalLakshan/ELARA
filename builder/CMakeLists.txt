add_executable(elara_builder
    main.cpp
    tag_reader.cpp
    art_processor.cpp
    index_writer.cpp
    logger.cpp
)

target_include_directories(elara_builder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${TAGLIB_INCLUDE_DIRS}
)

target_link_libraries(elara_builder PRIVATE
    elara_shared
    stb
    ${TAGLIB_LIBRARIES}
)

target_compile_options(elara_builder PRIVATE
    ${TAGLIB_CFLAGS_OTHER}
    -Wno-missing-field-initializers
    -Wno-unused-but-set-variable
)

install(TARGETS elara_builder DESTINATION bin)