aux_source_directory(. SOURCE_FILES)
add_executable(editor ${SOURCE_FILES})

target_link_libraries(editor
    PRIVATE
    raylib
    ${LR_LIB}
    ${LR_TEXT_LIB}
    m  # Add math library for color calculations
)
