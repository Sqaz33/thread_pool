function(add_address_sanitizer target_name)
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "Target '${target_name}' does not exist.")
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target_name} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(${target_name} PRIVATE -fsanitize=address)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        # ASan is supported in MSVC starting with version 16.9
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.29)
            target_compile_options(${target_name} PRIVATE /fsanitize=address)
            target_link_options(${target_name} PRIVATE /fsanitize=address)
        else()
            message(WARNING "Address sanitizer is not supported in this version of MSVC.")
        endif()
    else()
        message(WARNING "Address sanitizer is not explicitly supported for this compiler: ${CMAKE_CXX_COMPILER_ID}.")
    endif()
endfunction()