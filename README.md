# cabinet
A small game engine primarily targeting web based games with lowpoly pixelart graphics

## Requirements

- npm for the web builds
- a c compiler for native builds (I use Clang)
- GLFW for native window management

## Build and run

Before first build, you must bootstrap the C build system by running
```bash
$ ./CC -o nob nob.c
```

Then you must install the NPM dependencies by running
```bash
$ npm install
```

Now you can run the development server which will automatically rebuild the web and native build when you save a file

```bash
$ npm run dev
```
