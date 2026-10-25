set(APP_NAME app)

set(SRC
    ./src/app.c
)

set(INC
    ./inc/app_defs.h
)

set(INC_DIRS
    ./inc
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

target_compile_options(${APP_NAME}
    PRIVATE -Wall -Wextra
)
