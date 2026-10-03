include_guard(GLOBAL)
include(linting)

find_program(BUILDCACHE buildcache)
if(BUILDCACHE)
    list(PREPEND BUILDCACHE env BUILDCACHE_LUA_PATH=${CMAKE_CURRENT_LIST_DIR})
endif()

# clang-tidy {{{

find_program(CLANGTIDY clang-tidy)
if(CLANGTIDY)
    if(BUILDCACHE)
        list(PREPEND CLANGTIDY ${BUILDCACHE})
    endif()
    if(NOT CLANGTIDY_CONFIG)
        set(CLANGTIDY_CONFIG ${CMAKE_SOURCE_DIR}/.clang-tidy)
        if(NOT EXISTS ${CLANGTIDY_CONFIG})
            set(CLANGTIDY_CONFIG)
        endif()
    endif()
    if(CLANGTIDY_CONFIG)
        list(APPEND CLANGTIDY --config-file=${CLANGTIDY_CONFIG})
    endif()
    if(CMAKE_COLOR_DIAGNOSTICS)
        list(APPEND CLANGTIDY --use-color)
    endif()
endif()

function(add_clangtidy_target F COMPONENT)
    if(NOT CLANGTIDY)
        return()
    endif()
    get_filename_component(F ${F} ABSOLUTE)
    get_filename_component(TGT ${F} NAME)
    add_custom_target(clang-tidy-${TGT}
        COMMAND ${GA_GROUP} ${CLANGTIDY} ${CLANGTIDY_FLAGS} -p ${CMAKE_BINARY_DIR} ${F}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        VERBATIM
    )
    add_chained_targets(lint lint-${COMPONENT} lint-${TGT} clang-tidy-${TGT})
    add_chained_targets(clang-tidy clang-tidy-${COMPONENT} clang-tidy-${TGT})
endfunction()

# }}}

# cppcheck {{{

find_program(CPPCHECK cppcheck)
if(CPPCHECK)
    if(BUILDCACHE)
        list(PREPEND CPPCHECK ${BUILDCACHE})
    endif()
    if(CMAKE_COLOR_DIAGNOSTICS)
        list(PREPEND CPPCHECK env CLICOLOR_FORCE=1)
    endif()
endif()

function(add_cppcheck_target F COMPONENT)
    if(NOT CPPCHECK)
        return()
    endif()
    get_filename_component(F ${F} ABSOLUTE)
    get_filename_component(TGT ${F} NAME)
    add_custom_target(cppcheck-${TGT}
        COMMAND ${GA_GROUP} ${CPPCHECK} ${CPPCHECK_FLAGS} --project=${CMAKE_BINARY_DIR}/compile_commands.json --template=gcc --file-filter=${F}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        VERBATIM
    )
    add_chained_targets(lint lint-${COMPONENT} lint-${TGT} cppcheck-${TGT})
    add_chained_targets(cppcheck cppcheck-${COMPONENT} cppcheck-${TGT})
endfunction()

# }}}
