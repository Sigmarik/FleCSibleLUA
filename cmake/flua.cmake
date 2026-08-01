# flua_components.cmake
# Usage: target_flua_components(<target> <generator-exe-or-target> <inputs...>)
#
# Generates a single <target>_flua_components.cpp in the caller's binary dir by running
# <generator> <output.cpp> <inputs...> with WORKING_DIRECTORY set to the caller's binary dir.
# The generated file is added to the target's sources and the target depends on the generation step.

function(target_flua_components tgt)
    if(NOT TARGET ${tgt})
        message(FATAL_ERROR "target_flua_components: target '${tgt}' does not exist")
    endif()
    if(ARGC LESS 1)
        message(FATAL_ERROR "target_flua_components requires at least: <target> <inputs...>")
    endif()

    if (NOT DEFINED FLUA_GENERATOR)
        message(FATAL_ERROR "FLUA_GENERATOR variable is not set. Please set it to the path of the generator executable or a CMake target.")
    endif()

    # Capture the caller's binary dir at function invocation time
    set(_caller_bin_dir "${CMAKE_CURRENT_BINARY_DIR}")

    # Collect inputs (ARGN contains inputs)
    set(_inputs ${ARGN})

    get_target_property(_flua_target_includes ${tgt} INCLUDE_DIRECTORIES)

    set(_include_paths)
    if (NOT "${_flua_target_includes}" STREQUAL _flua_target_includes-NOTFOUND)
        foreach(_incl IN LISTS _flua_target_includes)
            list(APPEND _include_paths "-I")
            list(APPEND _include_paths "${_incl}")
        endforeach()
    else()
        message(WARNING "target_flua_components requires target include paths to be defined\nHint: Call target_include_paths before target_flua_components")
    endif()

    # Ensure output directory under caller's binary dir
    set(_out_dir "${_caller_bin_dir}/generated/${tgt}")
    file(MAKE_DIRECTORY "${_out_dir}")

    # Output file path
    set(_out_file "${_out_dir}/${tgt}_flua_components.cpp")

    # Determine generator invocation: CMake target or external path
    if(TARGET ${FLUA_GENERATOR})
        set(_gen_cmd "$<TARGET_FILE:${FLUA_GENERATOR}>")
        set(_extra_dep ${FLUA_GENERATOR})
    else()
        set(_gen_cmd "${FLUA_GENERATOR}")
        set(_extra_dep "")
    endif()

    # Build COMMAND arguments: output first, then inputs
    # Use VERBATIM so we pass arguments safely; don't quote inside COMMAND list
    set(_cmd)
    list(APPEND _cmd ${_gen_cmd} "${_out_file}")
    foreach(_i IN LISTS _inputs)
        list(APPEND _cmd "${_i}")
    endforeach()
    foreach(_i IN LISTS _include_paths)
        list(APPEND _cmd "${_i}")
    endforeach()

    get_filename_component(_call_cwd "${CMAKE_PARENT_LIST_FILE}" DIRECTORY)

    add_custom_command(
            OUTPUT "${_out_file}"
            COMMAND ${CMAKE_COMMAND} -E echo "Generating ${_out_file}"
            COMMAND ${_cmd}
            DEPENDS ${_inputs} ${_extra_dep}
            WORKING_DIRECTORY "${_call_cwd}"
            COMMENT "Generating FLua components for target ${tgt} (cwd=${_call_cwd})"
            VERBATIM
    )

    # Unique custom target name per target
    string(REPLACE "/" "_" _safe "${tgt}_flua_gen")
    add_custom_target(${_safe} DEPENDS "${_out_file}")

    # Ensure the main target depends on the generation target
    add_dependencies(${tgt} ${_safe})

    # Add the generated file to the target's sources
    target_sources(${tgt} PRIVATE "${_out_file}")

    # Expose the generated path as a target property (optional)
    set_property(TARGET ${tgt} PROPERTY FLUA_GENERATED_SOURCE "${_out_file}")
endfunction()
