#define NOB_IMPLEMENTATION
#include "nob.h"

#define SRC_DIR "src"
#define BUILD_DIR "build"
#define WASM_BUILD_DIR "glue/wasm"
#define NATIVE_BUILD_DIR "build/native"

void init_clang_wasm(Nob_Cmd *cmd, const char *output_wasm) {
    nob_cmd_append(cmd,
        "/usr/bin/clang",                    // Emscripten compiler
        "-target", "wasm32", // Target WebAssembly
        "-std=c23",               // C23 standard
        "-nostdlib",              // No standard library
        "-mbulk-memory",          // Bulk memory operations
        "-Wall", "-Wextra",       // Warnings
        "-O2",                    // Optimize for performance
        "-Iinclude",              // Include directory for cglm, cgltf
        "-Isrc",
        "-Wl,--export-all",       // Export all functions
        "-Wl,--no-entry",         // No entry point
        "-Wl,--allow-undefined",  // Allow undefined symbols (implemented in JS land)
        "-o", output_wasm,        // Output WebAssembly
    );
}

void init_emcc_wasm(Nob_Cmd *cmd, const char *output_wasm) {
    nob_cmd_append(cmd,
        "emcc",                    // Emscripten compiler
        "-std=c23",               // C standard
        "-Wall", "-Wextra",       // Warnings
        "-O2",                    // Optimize for performance
        "-Iinclude",              // Include directory for cglm, cgltf
        "-Isrc",                  // 
        "-s", "USE_WEBGL2=1",     // Enable WebGL 2
        "-s", "EXPORT_ES6=1",     // Export as ES6 module
        "-s", "MODULARIZE=1",     // Modularize for Vite
        "-o", output_wasm,        // Output WebAssembly
    );
}


// emcc src/tiny.c -o cube.js -Os -s WASM=1 -s ALLOW_MEMORY_GROWTH=0 -s MINIMAL_RUNTIME=0 -s GL_UNSAFE_OPTS=1 -s ENVIRONMENT=web -s USE_GLFW=0 -s TOTAL_MEMORY=16MB -s USE_WEBGL2 -s EXIT_RUNTIME=0
void init_minimal_emcc(Nob_Cmd *cmd) {
    nob_cmd_append(cmd,
        "emcc",                    // Emscripten compiler
        "-std=c23",               // C standard
        "-o", "tiny.js",
        "-Os",                    // Optimize for size
        "-s", "WASM=1",               // Output WebAssembly,
        "-s", "ALLOW_MEMORY_GROWTH=0", // Allow memory growth
        "-s", "MINIMAL_RUNTIME=0",     // Minimal runtime
        "-s", "GL_UNSAFE_OPTS=1",        // Unsafe GL options
        "-s", "ENVIRONMENT=web",        // Web environment
        "-s", "USE_WEBGL2=1",         // Use WebGL 2
        "-s", "MIN_WEBGL_VERSION=2", // Minimum WebGL version
        "-s", "MAX_WEBGL_VERSION=2", // Maximum WebGL version
        "-s", "USE_GLFW=0",         // Disable GLFW
        "-s", "FILESYSTEM=0",         // Disable file system
        "-s", "ASSERTIONS=0",         // Disable assertions
        "-s", "TOTAL_MEMORY=16MB", // Total memory
        "-D", "SOKOL_GLES3", // Use GLES3
        "src/main.c",
    );
}


void init_clang_native(Nob_Cmd *cmd, const char *output_exe) {
    nob_cmd_append(cmd,
        "clang",                  // Clang compiler
        "-std=c23",               // C23 standard
        "-Wall", "-Wextra",       // Warnings
        "-O2",                    // Optimize for performance
        "-Iinclude",              // Include directory for cglm, cgltf
        "-Isrc",
        // "-lopengl32",             // Link OpenGL on Windows
        "-lGL",                   // Link OpenGL on Linux
        "-lglfw",                 // Link GLFW
        "-lm",                    // Link math library
        "-DNATIVE",               // Define NATIVE
        "-o", output_exe          // Output executable
    );
}

// Collect all .c files from src/ into a Nob_File_Paths
bool collect_c_files(Nob_File_Paths *c_files, Nob_File_Paths *h_files) {
    if (!nob_mkdir_if_not_exists(BUILD_DIR)) return false;
    if (!nob_mkdir_if_not_exists(WASM_BUILD_DIR)) return false;
    if (!nob_mkdir_if_not_exists(NATIVE_BUILD_DIR)) return false;

    Nob_File_Paths children = {0};
    if (!nob_read_entire_dir(SRC_DIR, &children)) return false;

    for (size_t i = 0; i < children.count; i++) {
        Nob_String_Builder sb = {0};
        nob_sb_append_cstr(&sb, SRC_DIR);
        nob_sb_append_cstr(&sb, "/");
        nob_sb_append_cstr(&sb, children.items[i]);
        nob_sb_append_null(&sb);

        if (nob_sv_end_with(nob_sv_from_cstr(children.items[i]), ".c")) {
            nob_da_append(c_files, nob_temp_strdup(sb.items));
        } else if (nob_sv_end_with(nob_sv_from_cstr(children.items[i]), ".h")) {
            nob_da_append(h_files, nob_temp_strdup(sb.items));
        }

        nob_sb_free(sb);
    }

    nob_da_free(children);
    return true;
}


int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, "bear", "--");
    init_minimal_emcc(&cmd);

    nob_cmd_run_sync_and_reset(&cmd);


/*
  Nob_File_Paths c_files = {0};
  Nob_File_Paths h_files = {0};
  if (!collect_c_files(&c_files, &h_files)) {
    return 1;
  }

  Nob_Cmd wasm_cmd = {0};
  init_clang_wasm(&wasm_cmd, WASM_BUILD_DIR "/main.wasm");
  for (size_t i = 0; i < c_files.count; i++) {
    nob_cmd_append(&wasm_cmd, c_files.items[i]);
  }

  if (nob_needs_rebuild(WASM_BUILD_DIR "/main.wasm", h_files.items, h_files.count) > 0 ||
      nob_needs_rebuild(WASM_BUILD_DIR "/main.wasm", c_files.items, c_files.count) > 0) {
    nob_log(NOB_INFO, "Building WebAssembly");
    if (!nob_cmd_run_sync_and_reset(&wasm_cmd)) return 1;
  } else {
    nob_log(NOB_INFO, "WebAssembly build is up to date");
    wasm_cmd.count = 0; // Reset manually if not run
  }

  Nob_Cmd native_cmd = {0};
  // TODO: only use bear if available on command line
  nob_cmd_append(&native_cmd, "bear", "--");
  init_clang_native(&native_cmd, NATIVE_BUILD_DIR "/main.exe");
  for (size_t i = 0; i < c_files.count; i++) {
    nob_cmd_append(&native_cmd, c_files.items[i]);
  }
  nob_cmd_append(&native_cmd, "include/glad.c");
  if (nob_needs_rebuild(NATIVE_BUILD_DIR "/main.exe", h_files.items, h_files.count) > 0 ||
      nob_needs_rebuild(NATIVE_BUILD_DIR "/main.exe", c_files.items, c_files.count) > 0) {
    nob_log(NOB_INFO, "Building native executable");
    if (!nob_cmd_run_sync_and_reset(&native_cmd)) return 1;
  } else {
    nob_log(NOB_INFO, "Native build is up to date");
    native_cmd.count = 0; // Reset manually if not run
  }

  nob_da_free(c_files);
  nob_da_free(h_files);
*/
  return 0;
}
