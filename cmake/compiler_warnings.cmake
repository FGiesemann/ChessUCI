function(add_compiler_warnings TARGET_NAME)
    target_compile_options(${TARGET_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall;-Wextra;-Wpedantic;-Wshadow;-Wformat=2;-Wconversion;-Wnon-virtual-dtor;-Wold-style-cast;-Wcast-align;-Wunused;-Wunreachable-code;-Wnull-dereference;-Wdouble-promotion;-Werror=return-type>
        $<$<CXX_COMPILER_ID:Clang>:-Wall;-Wextra;-Wpedantic;-Wshadow;-Wno-c++98-compat;-Wno-c++98-compat-pedantic;-Wno-pre-c++20-compat-pedantic;-Wformat=2;-Wconversion;-Wnon-virtual-dtor;-Wold-style-cast;-Wcast-align;-Wunused;-Wunreachable-code;-Wnull-dereference;-Wdouble-promotion;-Werror=return-type;-Werror=implicit-function-declaration;-Werror=incompatible-pointer-types;-Wno-shadow-field-in-constructor>
        $<$<CXX_COMPILER_ID:MSVC>:/W4;/WX>
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
    )

    # Disable warnings for braced scalar initialization in Clang (erroneous warning for designated initializers)
    target_compile_options(${TARGET_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:Clang>:-Wno-braced-scalar-init>
    )

    target_compile_options(${TARGET_NAME} PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<NOT:$<BOOL:CMAKE_DEBUG_POSTFIX>>>:-Werror>
        $<$<AND:$<CXX_COMPILER_ID:Clang>,$<NOT:$<BOOL:CMAKE_DEBUG_POSTFIX>>>:-Werror>
    )

    target_compile_options(${TARGET_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
    )
endfunction()
