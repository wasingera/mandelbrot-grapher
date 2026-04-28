
.PHONY: all run build config

all: run

run: build
	@./build/mandelbrot

build: config
	@cmake --build build

config:
	@cmake -S . -B build
