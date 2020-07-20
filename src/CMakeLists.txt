# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

set(HEADER_LIST 
    "../include/humon/ansiColors.h"
    "../include/humon/humon.h"
    "../include/humon/humon.hpp")

add_library(humon-c
    ansiColors.c
    encoding.c
    parse.c
    printing.c
    node.c
    tokenize.c
    trove.c
    utils.c
    vector.c
    humon.internal.h
    ${HEADER_LIST}
)


target_include_directories(humon-c PUBLIC ../include/humon)
# configure_file(humon-c.pc.in humon-c.pc @ONLY)

target_include_directories(humon-c PRIVATE src)

# IDEs should put the headers in a nice place
source_group(TREE "${PROJECT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADER_LIST})

