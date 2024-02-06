add_executable(${PROJECT_NAME} ${PROJECT_NAME}.c ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_NAME=${PROJECT_NAME} VERSION=${PROJECT_VERSION})

if(CMAKE_CROSSCOMPILING AND CMAKE_BUILD_TYPE MATCHES Debug)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        PRE_LINK
        COMMAND grep "^[^\\#].*\$ " ${PROJECT_NAME}.i > ${PROJECT_NAME}.eu.c
        COMMENT "Cleaning preprocessed file"
        DEPENDS ${PROJECT_NAME}.i
        )

    add_custom_command(
        TARGET ${PROJECT_NAME}
        PRE_LINK
        COMMAND clang-format ${PROJECT_NAME}.eu.c > ${PROJECT_NAME}.e.c
        COMMENT "Cleaning preprocessed file"
        DEPENDS ${PROJECT_NAME}.eu.c
        )
endif()

set(HAL_LIBS_DEFS ${HAL_LIBS})
list(TRANSFORM HAL_LIBS_DEFS PREPEND -DHAL_)
list(TRANSFORM HAL_LIBS PREPEND ${EER_HAL_LIB}_)
add_definitions(${HAL_LIBS_DEFS})
target_link_libraries(${PROJECT_NAME} ${LIBS} ${HAL_LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})


# Board Support Configuration
find_package(yq QUIET)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/bsc.yml)
    execute_process(COMMAND yq -o=json ".${BOARD}" ${CMAKE_CURRENT_SOURCE_DIR}/bsc.yml
        OUTPUT_VARIABLE BOARD_DEFINITIONS_JSON)

    execute_process(COMMAND bash -c "echo -e '${BOARD_DEFINITIONS_JSON}' | jq -rj '
    [ paths(scalars) as $p
    | [ ( [ $p[] | tostring | ascii_upcase ] | join(\"_\") )
    , ( getpath($p) | tostring )
    ]
    | join(\"=\")
    ] | join(\";\")
    '"
    OUTPUT_VARIABLE BOARD_DEFINITIONS)
message(STATUS "Board Support Config for ${PROJECT_NAME} ${BOARD}: ${BOARD_DEFINITIONS}")
target_compile_definitions(${PROJECT_NAME} PRIVATE ${BOARD_DEFINITIONS})
endif()

# TODO: calculate size
# get_property(${project_name}_linked_libs target ${project_name} property link_interface_libraries)
# add_custom_target(${project_name}.usage
#     ${EER_LIB_PATH}/eer-nm ${PROJECT_NAME} ${${PROJECT_NAME}_LINKED_LIBS}
#     DEPENDS ${PROJECT_NAME}
#     )

if(EXISTS "${ARCH_PATH}/build.cmake")
    include(${ARCH_PATH}/build.cmake)
endif()

