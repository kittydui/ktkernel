set(MODULEC_EXECUTABLE "${CMAKE_SOURCE_DIR}/modulec/target/debug/modulec"
    CACHE FILEPATH "Path to the modulec compiler")

set(MODULEC_DEFAULT_TARGET "x86_64-elf"
    CACHE STRING "Default target triple for kernel modules")

set(MODULEC_COMPDB_DIR "${CMAKE_BINARY_DIR}/modulec_compdb"
    CACHE PATH "Directory for module compile_commands.json fragments")

set(_MODULEC_COMPDB_FRAGMENTS "" CACHE INTERNAL "List of module compdb fragment paths")

function(add_kernel_module name)
    cmake_parse_arguments(MOD "" "TARGET;OUTPUT_DIR" "SOURCES;INCLUDES;COMPILE_FLAGS" ${ARGN})

    if(NOT MOD_SOURCES)
        message(FATAL_ERROR "add_kernel_module(${name}): SOURCES is required")
    endif()

    if(NOT MOD_TARGET)
        set(MOD_TARGET "${MODULEC_DEFAULT_TARGET}")
    endif()

    if(NOT MOD_OUTPUT_DIR)
        set(MOD_OUTPUT_DIR "${CMAKE_BINARY_DIR}/modules")
    endif()

    file(MAKE_DIRECTORY "${MOD_OUTPUT_DIR}")
    file(MAKE_DIRECTORY "${MODULEC_COMPDB_DIR}")

    set(OUTPUT_FILE "${MOD_OUTPUT_DIR}/${name}.ktdrv")
    set(COMPDB_FRAGMENT "${MODULEC_COMPDB_DIR}/${name}_compdb.json")

    # Resolve source paths relative to the caller's directory
    set(ABSOLUTE_SOURCES "")
    foreach(src ${MOD_SOURCES})
        if(IS_ABSOLUTE "${src}")
            list(APPEND ABSOLUTE_SOURCES "${src}")
        else()
            list(APPEND ABSOLUTE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
        endif()
    endforeach()

    # Resolve include paths relative to the caller's directory
    set(ABSOLUTE_INCLUDES "")
    foreach(inc ${MOD_INCLUDES})
        if(IS_ABSOLUTE "${inc}")
            list(APPEND ABSOLUTE_INCLUDES "${inc}")
        else()
            list(APPEND ABSOLUTE_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/${inc}")
        endif()
    endforeach()

    set(MODULEC_ARGS
        -o "${MOD_OUTPUT_DIR}/${name}"
        --srcs ${ABSOLUTE_SOURCES}
        --target "${MOD_TARGET}"
        --compdb "${COMPDB_FRAGMENT}"
    )

    if(ABSOLUTE_INCLUDES)
        list(APPEND MODULEC_ARGS --includes ${ABSOLUTE_INCLUDES})
    endif()

    if(MOD_COMPILE_FLAGS)
        list(APPEND MODULEC_ARGS --compile-flags ${MOD_COMPILE_FLAGS})
    endif()

    add_custom_command(
        OUTPUT "${OUTPUT_FILE}"
        COMMAND "${MODULEC_EXECUTABLE}" ${MODULEC_ARGS}
        DEPENDS ${ABSOLUTE_SOURCES}
        COMMENT "Building kernel module: ${name}.ktdrv"
        VERBATIM
    )

    add_custom_target(${name} ALL DEPENDS "${OUTPUT_FILE}")

    # Track this fragment for merging
    set(_MODULEC_COMPDB_FRAGMENTS "${_MODULEC_COMPDB_FRAGMENTS};${COMPDB_FRAGMENT}"
        CACHE INTERNAL "List of module compdb fragment paths")
endfunction()

# Call this once after all add_kernel_module() calls to create the merge target.
function(modulec_merge_compdb)
    if(NOT _MODULEC_COMPDB_FRAGMENTS)
        return()
    endif()

    set(MERGE_SCRIPT "${MODULEC_COMPDB_DIR}/merge_compdb.cmake")
    set(MAIN_COMPDB "${CMAKE_BINARY_DIR}/compile_commands.json")
    set(MERGED_COMPDB "${CMAKE_BINARY_DIR}/compile_commands.json")

    # Generate a CMake script that merges the fragments at build time
    file(WRITE "${MERGE_SCRIPT}" "
set(MAIN_COMPDB \"${MAIN_COMPDB}\")
set(MERGED_COMPDB \"${MERGED_COMPDB}\")
set(FRAGMENTS \"${_MODULEC_COMPDB_FRAGMENTS}\")

# Read the main compile_commands.json (strip trailing ] and whitespace)
if(EXISTS \"\${MAIN_COMPDB}\")
    file(READ \"\${MAIN_COMPDB}\" MAIN_CONTENT)
else()
    set(MAIN_CONTENT \"[]\")
endif()

# Strip trailing ]\\n
string(REGEX REPLACE \"\\\\][ \\t\\r\\n]*$\" \"\" MAIN_CONTENT \"\${MAIN_CONTENT}\")

# Check if main has existing entries (non-empty after stripping [)
string(STRIP \"\${MAIN_CONTENT}\" MAIN_STRIPPED)
if(MAIN_STRIPPED STREQUAL \"[\")
    set(NEEDS_COMMA FALSE)
else()
    set(NEEDS_COMMA TRUE)
endif()

# Collect all fragment entries
set(MODULE_ENTRIES \"\")
foreach(FRAG \${FRAGMENTS})
    if(EXISTS \"\${FRAG}\")
        file(READ \"\${FRAG}\" FRAG_CONTENT)
        # Strip outer [ ] from fragment
        string(REGEX REPLACE \"^[ \\t\\r\\n]*\\\\[\" \"\" FRAG_CONTENT \"\${FRAG_CONTENT}\")
        string(REGEX REPLACE \"\\\\][ \\t\\r\\n]*$\" \"\" FRAG_CONTENT \"\${FRAG_CONTENT}\")
        string(STRIP \"\${FRAG_CONTENT}\" FRAG_CONTENT)
        if(NOT FRAG_CONTENT STREQUAL \"\")
            set(MODULE_ENTRIES \"\${MODULE_ENTRIES}\${FRAG_CONTENT},\")
        endif()
    endif()
endforeach()

# Strip trailing comma
string(REGEX REPLACE \",$\" \"\" MODULE_ENTRIES \"\${MODULE_ENTRIES}\")

if(NOT MODULE_ENTRIES STREQUAL \"\")
    if(NEEDS_COMMA)
        set(RESULT \"\${MAIN_CONTENT},\\n\${MODULE_ENTRIES}\\n]\")
    else()
        set(RESULT \"\${MAIN_CONTENT}\\n\${MODULE_ENTRIES}\\n]\")
    endif()
else()
    set(RESULT \"\${MAIN_CONTENT}\\n]\")
endif()

file(WRITE \"\${MERGED_COMPDB}\" \"\${RESULT}\\n\")
message(STATUS \"Merged module compile commands into \${MERGED_COMPDB}\")
")

    add_custom_target(modulec_merge_compdb ALL
        COMMAND ${CMAKE_COMMAND} -P "${MERGE_SCRIPT}"
        COMMENT "Merging module compile_commands.json fragments"
        VERBATIM
    )

    # Ensure merge runs after all module builds
    foreach(frag ${_MODULEC_COMPDB_FRAGMENTS})
        get_filename_component(mod_name "${frag}" NAME_WE)
        string(REPLACE "_compdb" "" mod_name "${mod_name}")
        if(TARGET ${mod_name})
            add_dependencies(modulec_merge_compdb ${mod_name})
        endif()
    endforeach()
endfunction()
