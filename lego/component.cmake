add_library(${PROJECT_NAME} STATIC ${PROJECT_NAME}.c)

target_link_libraries(${PROJECT_NAME} ${LEGO})
target_include_directories(${PROJECT_NAME} INTERFACE .)
