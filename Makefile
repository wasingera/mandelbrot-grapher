
.PHONY: all run build config

all: run

run: build
	@./build/mandelbrot #2> out.log

build: 
	@cmake --build build

config:
	@cmake -S . -B build
