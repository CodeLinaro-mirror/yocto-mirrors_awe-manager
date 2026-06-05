if(NOT PROJECT_IS_TOP_LEVEL)
    # Will conflict with the top level project if it defines similar functionality.
    if(AWEMGR_STATIC_ANALYSIS)
        message(WARNING "cppcheck will only run if AWE Manager is the top level project.")
    endif()

    return()
endif()

# This function dumps all macros natively set by the compiler to ${file}.
# It only works with gcc/clang.
function(dump_macros file lang)
    macro(dump_macros_warning_exit str)
        message(FATAL_ERROR "${CMAKE_CURRENT_FUNCTION}: can't dump macro for compiler \"${CMAKE_${lang}_COMPILER}\". ${str}")
        return()
    endmacro()

    if((CMAKE_${lang}_COMPILER_ID STREQUAL "GNU") OR (CMAKE_${lang}_COMPILER_ID STREQUAL "CLANG"))
        set(args
            -dM
            -E
        )
        if(lang STREQUAL "C")
            list(APPEND args -x c)
        elseif(lang STREQUAL "CXX")
            list(APPEND args -x c++)
        else()
            dump_macros_warning_exit("invalid lang \"${lang}\".")
        endif()
    else()
        set(args )
        dump_macros_warning_exit("CMAKE_${lang}_COMPILER_ID is unset or invalid.")
    endif()

    if(NOT CMAKE_${lang}_COMPILER)
        message(FATAL_ERROR "CMAKE_${lang}_COMPILER not set.")
    endif()

    file(TOUCH ${CMAKE_BINARY_DIR}/empty)
    execute_process(
        COMMAND ${CMAKE_${lang}_COMPILER} ${args} -o ${file} ${CMAKE_BINARY_DIR}/empty
    )
endfunction()

find_program(CPPCHECK cppcheck)

if(CPPCHECK)
    # cppcheck needs this file
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(COMPILE_COMMANDS "${CMAKE_BINARY_DIR}/compile_commands.json")
    set(CPPCHECK_SUPPRESSIONS "${PROJECT_SOURCE_DIR}/cppcheck_suppressions.txt")
    set(COMPILER_MACROS_HEADER ${CMAKE_CURRENT_BINARY_DIR}/compiler_macros.h)

    dump_macros(${COMPILER_MACROS_HEADER} C)

    set(CPPCHECK_ARGS
        --premium=misra-c-2012
        --std=c99
        --xml
        --enable=all
        --xml-version=3
        --check-level=exhaustive
        --verbose
        --relative-paths=${PROJECT_SOURCE_DIR}
        --suppressions-list=${CPPCHECK_SUPPRESSIONS}
        --include=${COMPILER_MACROS_HEADER}
    )
    
    set(CPPCHECK_REPORT_XML ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-cppcheck.xml)
    set(CPPCHECK_REPORT_CHECKERS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-cppcheck-checkers.txt)
    
    add_custom_command(
        OUTPUT ${CPPCHECK_REPORT_XML}
        COMMAND ${CPPCHECK} ${CPPCHECK_ARGS} --checkers-report="${CPPCHECK_REPORT_CHECKERS}" --output-file="${CPPCHECK_REPORT_XML}" --project="${COMPILE_COMMANDS}" || true
        BYPRODUCTS ${CPPCHECK_REPORT_CHECKERS}
        DEPENDS all ${CPPCHECK_SUPPRESSIONS} ${COMPILER_MACROS_HEADER}
        COMMAND_EXPAND_LISTS
    )
    
    add_custom_target(cppcheck DEPENDS ${CPPCHECK_REPORT_XML})
    
    install(
        FILES ${CPPCHECK_REPORT_XML}
        DESTINATION cppcheck/${PROJECT_NAME}_cppcheck_report.xml
        COMPONENT Reports
        EXCLUDE_FROM_ALL
        OPTIONAL
    )

    find_program(CPPCHECK_HTMLREPORT cppcheck-htmlreport)
    if(CPPCHECK_HTMLREPORT)
        set(CPPCHECK_REPORT_HTML "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_cppcheck_html")
        add_custom_command(
            OUTPUT "${CPPCHECK_REPORT_HTML}/index.html"
            BYPRODUCTS "${CPPCHECK_REPORT_HTML}/"
            COMMAND ${CPPCHECK_HTMLREPORT} --report-dir=${CPPCHECK_REPORT_HTML} --source-dir=${PROJECT_SOURCE_DIR} --file=${CPPCHECK_REPORT_XML}
            MAIN_DEPENDENCY "${CPPCHECK_REPORT_XML}"
            VERBATIM
            COMMAND_EXPAND_LISTS
        )

        add_custom_target(cppcheck-html DEPENDS "${CPPCHECK_REPORT_HTML}/index.html")

        install(
            DIRECTORY "${CPPCHECK_REPORT_HTML}/"
            DESTINATION cppcheck/${PROJECT_NAME}
            COMPONENT HtmlReports
            EXCLUDE_FROM_ALL
            OPTIONAL
        )
    endif()

    find_program(COMPLIANCE_REPORT compliance-report)
    if(COMPLIANCE_REPORT)
        # Generate MISRA compliance report.
        set(MISRA_COMPLIANCE_REPORT_HTML "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_misra_compliance.html")

        set(MISRA_COMPLIANCE_REPORT_ARGS
            --misra-c-2023
            --project-version=2.3
        )

        add_custom_command(
            OUTPUT "${MISRA_COMPLIANCE_REPORT_HTML}"
            COMMAND ${COMPLIANCE_REPORT} ${MISRA_COMPLIANCE_REPORT_ARGS} --project-name=${PROJECT_NAME} --output-file=${MISRA_COMPLIANCE_REPORT_HTML} ${CPPCHECK_REPORT_XML} || true
            MAIN_DEPENDENCY "${CPPCHECK_REPORT_XML}"
            VERBATIM
            COMMAND_EXPAND_LISTS
        )

        add_custom_target(misra DEPENDS "${MISRA_COMPLIANCE_REPORT_HTML}")

        install(
            FILES "${MISRA_COMPLIANCE_REPORT_HTML}"
            DESTINATION misra/
            RENAME "${PROJECT_NAME}.html"
            COMPONENT HtmlReports
            EXCLUDE_FROM_ALL
            OPTIONAL
        )
    endif()
else()
    message(WARNING "cppcheck was not found. skipping...")
endif()
