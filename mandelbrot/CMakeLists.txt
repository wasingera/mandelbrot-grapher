set(LIB_NAME mandelbrot)

set(SRC
    ./src/mandelbrot.c
)

if(CUDAToolkit_FOUND)
    list(APPEND SRC ./src/mandelbrot_kernel.cu)
endif()

set(INC
    ./inc/mandelbrot.h
    ./inc/mandelbrot_kernel.h
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

if(CUDAToolkit_FOUND)
    target_compile_definitions(${LIB_NAME}
        PUBLIC CUDA_EN
    )

    target_link_libraries(${LIB_NAME}
        PUBLIC CUDA::cudart
    )
endif()
