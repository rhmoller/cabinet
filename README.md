# cabinet

A small game engine that compiles to WebAssembly and native code

DISCLAIMER: This is an early work in progress and is not ready for production use. It is my recreational hobby project and
might not be suitable for others.

The primary goal of this engine is to generate tiny web builds that will load instantly.
The seconday goal is to be able to generate native builds for distribution on itch and steam.

## Requirements

- npm for the web builds
- Clang for WebAssembly and native builds
- Bear for generating compile_commands.json for clangd
- GLFW for native window management

## Build and run

Before first build, you must bootstrap the C build system by running
```bash
$ clang -o nob nob.c
```

Then you must install the NPM dependencies by running
```bash
$ npm install
```

Now you can run the development server which will automatically rebuild the web and native build when you save a file

```bash
$ npm run dev
```

## Credits

- [nob](https://github.com/tsoding/nob.h) the "no build" build system by @tsoding
- [GLFW](https://www.glfw.org/) for window management
- [stb](https://github.com/nothings/stb) for image loading and sprintf implementation
- [Clang](https://clang.llvm.org/) for compiling the C code to native and WebAssembly
- [Bear](https://github.com/rizsotto/Bear) for generating compile_commands.json
- [Vincent](https://github.com/DavidDiPaola/font_vincent) fixed-width 8x8 ASCII font by Quinn Evans
