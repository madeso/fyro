function(add_stb_library name)
    add_library(stb_${name} STATIC
        ${name}/src/${name}.cc
    )
    target_include_directories(stb_${name}
        PUBLIC
            ${name}/include/
    )
    add_library(stb::${name} ALIAS stb_${name})
endfunction()

add_stb_library(image)
add_stb_library(truetype)
add_stb_library(rect_pack)
