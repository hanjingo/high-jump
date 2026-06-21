function(add_subdirectory_all target_dir)
    file(GLOB children RELATIVE ${target_dir} ${target_dir}/*)

    foreach(child ${children})
        set(module_path "${target_dir}/${child}")

        if(IS_DIRECTORY ${module_path} AND EXISTS "${module_path}/CMakeLists.txt")
            add_subdirectory(${module_path})
            message(STATUS "Added subdirectory: ${module_path}")
        endif()
    endforeach()
endfunction()

function(add_subdirectory_all_exclude target_dir)
    set(exclude_dirs ${ARGN})
    file(GLOB children RELATIVE ${target_dir} ${target_dir}/*)

    foreach(child ${children})
        set(module_path "${target_dir}/${child}")
        list(FIND exclude_dirs ${child} exclude_index)

        if(exclude_index GREATER -1)
            continue()
        endif()

        if(IS_DIRECTORY ${module_path} AND EXISTS "${module_path}/CMakeLists.txt")
            add_subdirectory(${module_path})
            message(STATUS "Added subdirectory: ${module_path}")
        endif()
    endforeach()
endfunction()
