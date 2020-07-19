cmake_minimum_required(VERSION 3.13.4)

project(humon VERSION 0.0.0 
                DESCRIPTION "Human-Usable Machine Object Notation"
                LANGUAGES C CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    # Let's ensure -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Let's nicely support folders in IDEs
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Docs only available if this is the main app
    find_package(Doxygen)
    if(Doxygen_FOUND)
	    add_subdirectory(docs)
    else()
	    message(STATUS "Doxygen not found, not building docs")
    endif()
endif()

macro(FetchContent_MakeAvailable NAME)
    FetchContent_GetProperties(${NAME})
    if(NOT ${NAME}_POPULATED)
        FetchContent_Populate(${NAME})
        add_subdirectory(${${NAME}_SOURCE_DIR} ${${NAME}_BINARY_DIR})
    endif()
endmacro()


# FetchContent added in CMake 3.11, downloads during the configure step
include(FetchContent)


add_library(humon-c
    src/ansiColors.c
    src/encoding.c
    src/parse.c
    src/printing.c
    src/node.c
    src/tokenize.c
    src/trove.c
    src/utils.c
    src/vector.c
    src/ansiColors.h
    src/humon.internal.h
    include/humon/humon.h
)

set_target_properties(humon-c PROPERTIES 
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    PUBLIC_HEADER include/humon/humon.h
)

target_include_directories(humon-c PUBLIC include/humon)
configure_file(humon-c.pc.in humon-c.pc @ONLY)

target_include_directories(humon-c PRIVATE src)

# add_library(humon-cpp)
# target_link_libraries(humon-cpp PUBLIC humon-c)

set_target_properties(humon-cpp PROPERTIES 
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    PUBLIC_HEADER include/humon/humon.hpp
)

target_include_directories(humon-cpp PUBLIC include/humon)
configure_file(humon-c.pc.in humon-c.pc @ONLY)

target_include_directories(humon-cpp PRIVATE src)


add_executable(hux apps/hux/hux.cpp)
target_compile_features(app PRIVATE cxx_std_17)
target_link_libraries(app PRIVATE modern_library fmt::fmt)

