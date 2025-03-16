# cabinet
A small game engine primarily targeting web based games with lowpoly pixelart graphics

## Requirements

- npm for the web builds
- a c compiler for native builds (I use Clang)
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
