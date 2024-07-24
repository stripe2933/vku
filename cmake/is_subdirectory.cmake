function(is_subdirectory parent_dir sub_dir result)
    # Normalize paths
    get_filename_component(parent_abs "${parent_dir}" ABSOLUTE)
    get_filename_component(sub_abs "${sub_dir}" ABSOLUTE)

    # Initial check for path equality
    if (parent_abs STREQUAL sub_abs)
        set(${result} FALSE PARENT_SCOPE)
        return()
    endif()

    # Iterate up the directory tree from sub_dir
    set(current_dir "${sub_abs}")
    while (NOT current_dir STREQUAL parent_abs)
        get_filename_component(current_dir "${current_dir}" DIRECTORY)

        # If we reach the root and haven't matched, it's not a subdirectory
        if (current_dir STREQUAL "/")
            set(${result} FALSE PARENT_SCOPE)
            return()
        endif()

        # If the current_dir is equal to the parent_dir, then sub_dir is a subdirectory
        if (current_dir STREQUAL parent_abs)
            set(${result} TRUE PARENT_SCOPE)
            return()
        endif()
    endwhile()

    # Default case if nothing else matches
    set(${result} FALSE PARENT_SCOPE)
endfunction()