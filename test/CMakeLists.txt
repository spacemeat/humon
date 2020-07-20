# (1) Look for installed version of CppUTest
if(DEFINED ENV{CPPUTEST_HOME})
    message(STATUS "Using CppUTest home: $ENV{CPPUTEST_HOME}")
    set(CPPUTEST_INCLUDE_DIRS $ENV{CPPUTEST_HOME}/include)
    set(CPPUTEST_LIBRARIES $ENV{CPPUTEST_HOME}/lib)
    set(CPPUTEST_LDFLAGS CppUTest CppUTestExt)
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(CPPUTEST REQUIRED cpputest>=3.8)
    message(STATUS "Found CppUTest version ${CPPUTEST_VERSION}")
endif()


# (3) Take care of include directories
include_directories(${CPPUTEST_INCLUDE_DIRS} ../src/)
link_directories(${CPPUTEST_LIBRARIES})

# Tests need to be added as executables first
add_executable(humon-test 
    apiTests.cpp
    commentTests.cpp
    cppTests.cpp
    dataTests.cpp
    errorTests.cpp
    testData.h
    testDataCpp.h
    testMain.cpp
    utf8Tests.cpp
    vectorTests.cpp
    wakkaTests.cpp
)

# I'm using C++17 in the test
target_compile_features(humon-test PRIVATE cxx_std_17)

# Should be linked to the main library, as well as the Catch2 testing library
target_link_libraries(humon-test PRIVATE humon-c ${CPPUTEST_LDFLAGS})

# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
add_test(NAME test COMMAND humon-test) # Command can be a target
