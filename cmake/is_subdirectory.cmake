function(is_subdirectory parent_abs sub_abs result)
    if (sub_abs MATCHES "${parent_abs}$")
        set(${result} TRUE PARENT_SCOPE)
    else()
        set(${result} FALSE PARENT_SCOPE)
    endif()
endfunction()