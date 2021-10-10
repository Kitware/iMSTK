message("clearing ${dir}")

file(GLOB OLD_FILES ${dir}/*.cs)

# Remove old files if they exist
if (OLD_FILES)
    file(REMOVE ${OLD_FILES})
endif()