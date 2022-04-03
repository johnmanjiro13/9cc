# 9cc

Small C compiler designed in https://www.sigbus.info/compilerbook.

# Run on MacOS

```
$ make build

$ make run
docker run --rm -it -v ~/dev/src/github.com/johnmanjiro13/9cc:/9cc -w /9cc compilerbook bash

# run in ther container
$ make test
```
