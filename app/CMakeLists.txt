add_executable(editor
    main.c
    buffer.c
)

target_link_libraries(editor
    PRIVATE
    	lr
	camera
	render
	symbol
)
