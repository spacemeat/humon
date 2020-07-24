set(HEADER_LIST 
    "../include/humon/humon.hpp")

add_executable(hux hux/hux.cpp)

target_compile_features(hux PRIVATE cxx_std_17)

target_include_directories(hux PRIVATE ../include/humon)

target_link_libraries(hux PRIVATE humon-c)
