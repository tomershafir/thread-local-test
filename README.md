# thread-local-test

A test suite to validate various aspects of implicit thread local storage implementation on target platforms.

Implicit thread local storage means that the program does not directly manage it, but rather uses a platform provided implementation. Usually, the interface is part of the programming langauge, like thread_local storage classifier in C++11 and C23.

The implementation is rather complex, and involves cooperation of compiler, static linker, dynamic linker, kernel, ISA, and langauge runtime. Here is a [deep dive reference](https://chao-tic.github.io/blog/2018/12/25/tls).

## Build

To build the test suite, run the following command:

```bash
./build.py
```

It will generate test executables under the `build` directory.
