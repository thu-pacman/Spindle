file(GLOB SPINDLE_PASS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_llvm_pass_plugin(SpindlePass ${SPINDLE_PASS_SOURCES})

target_include_directories(SpindlePass PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(SpindlePass PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif(APPLE)
