function(CREATE_EAGER_TEST_EXE TESTLIST)
    foreach (TEST_SRC ${TESTLIST})
        get_filename_component(TEST_NAME ${TEST_SRC} NAME)
        get_filename_component(TEST_DIR ${TEST_SRC} DIRECTORY)
        set(TEST_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SRC})
        # unit_tests and tensor are already taken as target names/executables
        if(${TEST_NAME} STREQUAL "unit_tests")
            set(TEST_TARGET "test_unit_tests")
        elseif(${TEST_NAME} STREQUAL "tensor")
            set(TEST_TARGET "test_tensor")
        else()
            set(TEST_TARGET ${TEST_NAME})
        endif()
        add_executable(${TEST_TARGET} ${TEST_SRC_PATH})

        target_link_libraries(${TEST_TARGET} PUBLIC test_eager_common_libs)
        target_include_directories(${TEST_TARGET} PRIVATE
            ${UMD_HOME}
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/tt_metal
            ${CMAKE_SOURCE_DIR}/tt_metal/common
            ${CMAKE_SOURCE_DIR}/tests
            ${CMAKE_CURRENT_SOURCE_DIR}
        )
        set_target_properties(${TEST_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests/tt_eager/${TEST_DIR})
        list(APPEND EAGER_TEST_TARGETS ${TEST_TARGET})
    endforeach()
    set(EAGER_TEST_TARGETS "${EAGER_TEST_TARGETS}" PARENT_SCOPE)
endfunction()