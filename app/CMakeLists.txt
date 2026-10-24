set(APP_NAME app)

set(SRC
    ./src/main.c
)

set(INC

)

set(INC_DIRS
    ./inc
    ${PROJECT_ROOT}/mandelbrot/inc
)

add_executable(${APP_NAME}
    ${SRC}
    ${INC}
)

target_include_directories(${APP_NAME}
    PRIVATE ${INC_DIRS}
)

target_link_libraries(${APP_NAME}
    PRIVATE SDL3::SDL3  # SDL3 Graphics Lib
    PRIVATE mandelbrot
)
