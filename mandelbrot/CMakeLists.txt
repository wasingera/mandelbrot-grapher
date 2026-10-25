set(LIB_NAME mandelbrot)

set(SRC
    ./src/mandelbrot.c
)

set(INC
    ./inc/mandelbrot.h
)

set(INC_DIRS
    ./inc
)

add_library(${LIB_NAME}
    ${SRC}
    ${INC}
)

target_include_directories(${LIB_NAME}
    PUBLIC ${INC_DIRS}
)

target_link_libraries(${LIB_NAME}

)

target_compile_options(${LIB_NAME}
    PRIVATE -Wall -Wextra
)
