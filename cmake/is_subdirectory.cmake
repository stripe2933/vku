function(is_subdirectory parent_dir sub_dir result)
    # Normalize paths
    get_filename_component(parent_abs "${parent_dir}" ABSOLUTE)
    get_filename_component(sub_abs "${sub_dir}" ABSOLUTE)

    if (sub_dir MATCHES "${parent_dir}^")
        set(${result} TRUE PARENT_SCOPE)
    else()
        set(${result} FALSE PARENT_SCOPE)
    endif()
endfunction()