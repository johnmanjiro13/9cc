CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

build:
	docker build . -t compilerbook

run:
	docker run --rm -it -v $(CURDIR):/9cc -w /9cc compilerbook bash


# They needs to run in the docker container.
9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: build run test clean
