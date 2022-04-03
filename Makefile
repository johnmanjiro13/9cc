CFLAGS=-std=c11 -g -static

build:
	docker build . -t compilerbook

run:
	docker run --rm -it -v $(CURDIR):/9cc -w /9cc compilerbook bash


# They needs to run in the docker container.
9cc: 9cc.c

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: build run test clean
