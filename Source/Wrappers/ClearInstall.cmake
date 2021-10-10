message("clearing and installing ${source_dir} ${dest_dir}")

# Create destination directory if doesn't exist
file(MAKE_DIRECTORY ${dest_dir})

file(GLOB OLD_FILES ${dest_dir}/*.cs)

# Remove old files if they exist
if (OLD_FILES)
    file(REMOVE ${OLD_FILES})
endif()

# Gather and copy the new ones
file(GLOB NEW_FILES ${source_dir}/*.cs)
if (NEW_FILES)
    file(COPY ${NEW_FILES} DESTINATION ${dest_dir})
endif()