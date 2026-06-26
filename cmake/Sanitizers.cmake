function(tinycfg_apply_sanitizers target_name)
    if(NOT TINYCFG_ENABLE_SANITIZERS)
        return()
    endif()

    target_compile_options(${target_name} PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -g
    )
    target_link_options(${target_name} PRIVATE
        -fsanitize=address,undefined
    )
endfunction()

function(tinycfg_apply_fuzzer_sanitizers target_name)
    target_compile_options(${target_name} PRIVATE
        -fsanitize=fuzzer,address,undefined
        -fno-omit-frame-pointer
        -g
    )
    target_link_options(${target_name} PRIVATE
        -fsanitize=fuzzer,address,undefined
    )
endfunction()

function(tinycfg_apply_warnings target_name)
    target_compile_options(${target_name} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wformat=2
        -Wnull-dereference
        -Werror
    )
endfunction()
