add_executable(elara_validator
    main.cpp
)

target_include_directories(elara_validator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(elara_validator PRIVATE
    elara_shared
)

install(TARGETS elara_validator DESTINATION bin)