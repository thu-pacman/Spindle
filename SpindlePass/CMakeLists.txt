file(GLOB SPINDLE_PASS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_llvm_library(SpindlePass MODULE ${SPINDLE_PASS_SOURCES})

target_include_directories(SpindlePass PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(SpindlePass PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(SpindlePass PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif(APPLE)
