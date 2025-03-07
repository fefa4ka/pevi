aux_source_directory(. SOURCE_FILES)
add_executable(editor ${SOURCE_FILES})

target_link_libraries(editor
    PRIVATE
    raylib
    lr
)
