include(${CMAKE_CURRENT_LIST_DIR}/triplet.cmake)

# Capture the directory of this module for use in functions
set(_CRASH_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")

if(CMAKE_SCRIPT_MODE_FILE)
    if(NOT EXISTS "${DUMP_SYMS_EXECUTABLE}")
        message(FATAL_ERROR "dump_syms not found: ${DUMP_SYMS_EXECUTABLE}")
    endif()

    if(NOT DEFINED TARGET_FILE OR NOT DEFINED SYMBOLS_ROOT OR NOT DEFINED TARGET_NAME)
        message(FATAL_ERROR "Missing required crash symbol generation variables")
    endif()

    set(temp_sym_file "${SYMBOLS_ROOT}/${TARGET_NAME}.sym.tmp")
    set(final_sym_dir "${SYMBOLS_ROOT}")

    file(MAKE_DIRECTORY "${final_sym_dir}")

    execute_process(
        COMMAND "${DUMP_SYMS_EXECUTABLE}" "${TARGET_FILE}"
        OUTPUT_FILE "${temp_sym_file}"
        RESULT_VARIABLE dump_syms_result
    )

    if(NOT dump_syms_result EQUAL 0)
        file(REMOVE "${temp_sym_file}")
        message(FATAL_ERROR "dump_syms failed for ${TARGET_FILE} with exit code ${dump_syms_result}")
    endif()

    file(STRINGS "${temp_sym_file}" module_line LIMIT_COUNT 1)
    string(REGEX MATCH "^MODULE[ \t]+[^ \t]+[ \t]+[^ \t]+[ \t]+([^ \t]+)[ \t]+.*" _module_match "${module_line}")

    if(NOT CMAKE_MATCH_1)
        file(REMOVE "${temp_sym_file}")
        message(FATAL_ERROR "Unable to parse module id from dump_syms output for ${TARGET_FILE}")
    endif()

    set(module_id "${CMAKE_MATCH_1}")
    set(final_sym_file "${SYMBOLS_ROOT}/${module_id}/${TARGET_NAME}.sym")
    file(MAKE_DIRECTORY "${SYMBOLS_ROOT}/${module_id}")
    file(REMOVE "${final_sym_file}")
    file(RENAME "${temp_sym_file}" "${final_sym_file}")

    message(STATUS "Generated symbol file: ${final_sym_file}")
    return()
endif()

function(generate_sym target_name symbols_root)
    if(NOT symbols_root)
        message(FATAL_ERROR "generate_sym(target_name symbols_root): symbols_root is required")
    endif()

    detect_triplet_platform(platform_triplet)

    find_program(DUMP_SYMS_EXECUTABLE
        NAMES dump_syms dump_syms.exe
        PATHS "$ENV{VCPKG_ROOT}/installed/${platform_triplet}/tools/breakpad"
        NO_DEFAULT_PATH
        REQUIRED
    )

    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
            -DDUMP_SYMS_EXECUTABLE=${DUMP_SYMS_EXECUTABLE}
            -DTARGET_FILE=$<TARGET_FILE:${target_name}>
            -DSYMBOLS_ROOT=${symbols_root}
            -DTARGET_NAME=${target_name}
            -P "${_CRASH_MODULE_DIR}/crash.cmake"
        COMMENT "Generating Breakpad symbols for ${target_name}"
    )
endfunction()