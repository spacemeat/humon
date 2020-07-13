add_library(Humon trove.c node.c tokenize.c parse.c printing.c utils.c ansiColors.c)
target_include_directories(Humon PUBLIC ${CMAKE_SOURCE_DIR}/inc)
target_include_directories(Humon PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
