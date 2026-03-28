set(AWEMGR_TEST_RESULTS_JUNIT_XML "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_gtest_results.xml")
set(AWEMGR_COVERAGE_REPORT_HTML "${PROJECT_BINARY_DIR}/lcov/html")

add_custom_command(
    OUTPUT "${AWEMGR_TEST_RESULTS_JUNIT_XML}"
    COMMAND python -m awe_manager.tests.run_ctest_with_aweserver --cwd "${PROJECT_BINARY_DIR}" "${AWEMGR_TEST_RESULTS_JUNIT_XML}"
    DEPENDS all
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
)

install(
    FILES "${AWEMGR_TEST_RESULTS_JUNIT_XML}"
    DESTINATION junit/
    COMPONENT Reports
    OPTIONAL
    EXCLUDE_FROM_ALL
)

if(AWEMGR_TESTS_COVERAGE)
    set(valid_compiler_ids "GNU,Clang")
    set(is_valid_c_compiler "$<AND:$<COMPILE_LANGUAGE:C>,$<C_COMPILER_ID:${valid_compiler_ids}>>")
    set(is_valid_cxx_compiler "$<AND:$<COMPILE_LANGUAGE:CXX>,$<CXX_COMPILER_ID:${valid_compiler_ids}>>")
    add_compile_options(
        -g
        --coverage
        $<$<OR:${is_valid_c_compiler},${is_valid_cxx_compiler}>:-fprofile-abs-path>
    )

    find_program(LCOV lcov)
    
    if(LCOV)
        set(LCOV_ARGS
            --rc lcov_branch_coverage=1
            --directory "${PROJECT_BINARY_DIR}"
            --base-directory "${PROJECT_SOURCE_DIR}"
        )
        set(LCOV_EXCLUDES
            "/usr/lib/*"
            "/usr/include/*"
            "${CMAKE_BINARY_DIR}/_deps/*"
            "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/examples/*"
            "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/components/awe_awc/examples/*"
            "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/components/awe_awc/tests/*"
            "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/components/awe_cmd/tests/*"
            "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/components/awe_config/tests/*"
            "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/components/awe_ctrl/tests/*"
            "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/components/awe_awc/tests/*"
            "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/components/awe_osal/example/*"
            "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/components/awe_osal/tests/*"
            "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/tests/*"
        )

        add_custom_command(
            OUTPUT ${PROJECT_NAME}-coverage.base
            COMMAND ${LCOV} ${LCOV_ARGS} --zerocounters
            COMMAND ${LCOV} ${LCOV_ARGS} --capture --no-external --initial --output-file ${PROJECT_NAME}-coverage.base
            DEPENDS all
            VERBATIM
            COMMAND_EXPAND_LISTS
            COMMENT "coverage: generating baseline..."
        )

        add_custom_target(${PROJECT_NAME}-lcov-run-tests
            DEPENDS "${AWEMGR_TEST_RESULTS_JUNIT_XML}" ${PROJECT_NAME}-coverage.base
        )

        add_custom_command(
            OUTPUT ${PROJECT_NAME}-coverage.capture
            COMMAND ${LCOV} ${LCOV_ARGS} --capture --no-external --output-file ${PROJECT_NAME}-coverage.capture
            DEPENDS ${PROJECT_NAME}-lcov-run-tests
            VERBATIM
            COMMAND_EXPAND_LISTS
            COMMENT "coverage: running tests..."
        )

        add_custom_command(
            OUTPUT ${PROJECT_NAME}-coverage.total
            COMMAND ${LCOV} ${LCOV_ARGS} --add-tracefile ${PROJECT_NAME}-coverage.base --add-tracefile ${PROJECT_NAME}-coverage.capture --output-file ${PROJECT_NAME}-coverage.total
            DEPENDS ${PROJECT_NAME}-coverage.capture
            VERBATIM
            COMMAND_EXPAND_LISTS
            COMMENT "coverage: combining capture with baseline..."
        )

        add_custom_command(
            OUTPUT ${PROJECT_NAME}-coverage.info
            COMMAND ${LCOV} ${LCOV_ARGS} --remove ${PROJECT_NAME}-coverage.total --output-file ${PROJECT_NAME}-coverage.info ${LCOV_EXCLUDES}
            DEPENDS ${PROJECT_NAME}-coverage.total
            VERBATIM
            COMMAND_EXPAND_LISTS
            COMMENT "coverage: removing exclusions..."
        )

        add_custom_target(coverage
            DEPENDS ${PROJECT_NAME}-coverage.info
            COMMAND ${LCOV} ${LCOV_ARGS} --summary ${PROJECT_NAME}-coverage.info
            VERBATIM
            COMMAND_EXPAND_LISTS
        )

        install(
            FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-coverage.info
            DESTINATION lcov/
            COMPONENT Reports
            EXCLUDE_FROM_ALL
            OPTIONAL
        )

        find_program(GENHTML genhtml)
        if(GENHTML)
            set(GENHTML_ARGS
                --demangle-cpp
                --rc genhtml_branch_coverage=1
            )
            add_custom_target(coverage-html
                COMMAND ${GENHTML} ${GENHTML_ARGS} --output-directory "${AWEMGR_COVERAGE_REPORT_HTML}" --title "${PROJECT_NAME} Test Coverage Report" ${PROJECT_NAME}-coverage.info
                DEPENDS ${PROJECT_NAME}-coverage.info
                BYPRODUCTS "${AWEMGR_COVERAGE_REPORT_HTML}/"
                VERBATIM
                COMMAND_EXPAND_LISTS
                COMMENT "coverage: generating html report: ${AWEMGR_COVERAGE_REPORT_HTML}/index.html"
            )

            install(
                DIRECTORY "${AWEMGR_COVERAGE_REPORT_HTML}/"
                DESTINATION coverage/${PROJECT_NAME}
                COMPONENT HtmlReports
                EXCLUDE_FROM_ALL
                OPTIONAL
            )
        endif()
    endif()
endif()
