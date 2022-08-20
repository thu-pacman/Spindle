cmake_minimum_required(VERSION 3.4)
project(Spindle CXX C)

find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH cmake ${LLVM_CMAKE_DIR})
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})

include(fmt)

include(common)

if (NOT LLVM_ENABLE_RTTI)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fno-rtti)
endif ()

add_llvm_pass_plugin(Spindle spindle/entry.cpp)
target_include_directories(Spindle PRIVATE
	include

	detector
	tracer
)
target_link_libraries(Spindle PRIVATE fmt::fmt-header-only)
target_link_libraries(Spindle PRIVATE $<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>)

add_subdirectory(SpindlePass)
