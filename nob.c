// Build script based on Nob https://github.com/tsoding/nob.h
#define NOB_IMPLEMENTATION
#include "nob.h"
#include <time.h>

const char *compiler_path;
const char *archiver_path;

const char *demos_dir = "demos";
const char *modules_dir = "modules";
const char *deps_dir = "deps";

const char *build_dir = "build";

// Helper struct to manage build targets (modules/demos)
typedef struct {
    const char *name;         // Module or name (usually the directory name)
    const char *dir_path;     // Path to the module source directory
    Nob_File_Paths src_files; // Full paths to source files (.c)
    Nob_File_Paths obj_files; // Full paths to object files in build dir
    const char *target_path;  // Full path to the final library or executable in
                              // build dir
    const char
        *build_subdir;  // Path to the specific subdirectory within build_dir
    bool is_executable; // True if it's a demo (executable), false if module
                        // (library)
} BuildTarget;

// Dynamic array of BuildTargets
typedef struct {
    BuildTarget *items;
    size_t count;
    size_t capacity;
} BuildTargets;

// Define common flags - adjust as needed
#ifdef _WIN32
    #define OBJ_EXT ".obj"
    #define STATIC_LIB_EXT ".lib"
    #define EXE_EXT ".exe"
    // Add necessary libraries for Windows (e.g., user32, gdi32)
    const char *common_cflags[] = {"/std:c11", "/W4", "/wd4100", "/wd4201", "/nologo", "/Zi", "/I."}; // Added /I. to include from root
    const char *common_ldflags[] = {"/DEBUG"};
    const char *common_libs[] = { "user32.lib", "gdi32.lib" /* Add more if needed */ };
#else
    #define OBJ_EXT ".o"
    #define STATIC_LIB_EXT ".a"
    #define EXE_EXT ""
    const char *common_cflags[] = {"-std=c23", "-pthread","-Wall", "-Wextra", "-pedantic", "-Wno-missing-field-initializers" ,"-ggdb", "-I.", "-Ideps", "-Ideps/lua/src", "-Imodules/baselib", "-DSOKOL_GLCORE"}; // Added -I. to include from root
    const char *common_ldflags[] = {"-lm", "-lGL", "-ldl", "-lX11", "-lXi", "-lXcursor", "-lasound"}; // Link math library by default
    const char *common_libs[] = {}; // Add libs like -lglfw, -lvulkan if needed globally
#endif

void print_usage(const char *program_name);
void clean_dir(const char *path);
bool collect_modules(const char *root, BuildTargets *modules, bool is_exe);
bool compile_object_file(const char *src_path, const char *obj_path,
                        const char *target_include_dir);
bool build_static_library(BuildTarget *target);
bool build_executable(BuildTarget *target,
                        Nob_File_Paths *module_lib_paths);
bool parse_module(const char *module_dir_path);
 
int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    const char *program_name = nob_shift_args(&argc, &argv); // Get program name (nob)

    if (!nob_mkdir_if_not_exists(build_dir)) {
        nob_log(NOB_ERROR, "Failed to create build directory: %s", build_dir);
        return 1;
    }

    // --- Argument Parsing ---
    bool clean_build = false;
    bool test_build = false;

    while (argc > 0) {
        const char *arg = nob_shift_args(&argc, &argv);
        if (strcmp(arg, "clean") == 0) {
            clean_build = true;
        } else if (strcmp(arg, "test") == 0) {
            test_build = true;
        } else {
            print_usage(program_name);
            nob_log(NOB_ERROR, "Unknown argument: %s", arg);
            return 1;
        }
    }

    if (test_build) {
        parse_module("deps/lua");
        return 0;
    }


    // --- Set Platform Specific Tools ---
#ifdef _WIN32
    compiler_path = "cl.exe";
    archiver_path = "lib.exe";
    // Ensure cl.exe and lib.exe are in PATH (e.g., by running from VS Developer Command Prompt)
#else
    // Try clang first, fallback to gcc/ar
    if (nob_file_exists("/usr/bin/clang")) {
        compiler_path = "clang";
        archiver_path = "ar";
    } else if (nob_file_exists("/usr/bin/gcc")) {
        compiler_path = "gcc";
        archiver_path = "ar";
    } else {
        nob_log(NOB_ERROR, "Could not find a suitable C compiler (clang or gcc). Please install one.");
        return 1;
    }
    nob_log(NOB_INFO, "Using compiler: %s", compiler_path);
#endif
    
        // --- Clean Build ---
    if (clean_build) {
        nob_log(NOB_INFO, "Cleaning build directory: %s", build_dir);
        clean_dir(build_dir); // Clean the build directory
        nob_log(NOB_INFO, "Clean finished.");
        return 0; // Exit after cleaning
    }

    // --- Setup Build Environment ---
    if (!nob_mkdir_if_not_exists(build_dir)) return 1;

    // --- Discover Build Targets ---
    BuildTargets modules = {0};
    BuildTargets demos = {0};
    Nob_File_Paths root_dirents = {0};
    Nob_File_Paths demo_dirents = {0};
    Nob_File_Paths module_lib_paths = {0}; // To store paths of built static libs

    // Discover Modules (subdirs in root with module.txt)
    nob_log(NOB_INFO, "--- Discovering Modules ---");
    collect_modules(deps_dir, &modules, false); // Collect modules from the deps directory
    collect_modules(modules_dir, &modules, false); // Collect modules from the modules directory

    // Discover Demos (subdirs in demos/)
    nob_log(NOB_INFO, "--- Discovering Demos ---");
    collect_modules(demos_dir, &demos, true); // Collect demos from the demos directory

    // --- Build Phase ---

    // Build Modules (Static Libraries)
    nob_log(NOB_INFO, "--- Building Modules ---");
    for (size_t i = 0; i < modules.count; ++i) {
        BuildTarget *module = &modules.items[i];

        //build_module(module);
        nob_log(NOB_INFO, "Building module: %s", module->name);
        if (!nob_mkdir_if_not_exists(module->build_subdir)) return 1;

        bool module_ok = true;
        for (size_t j = 0; j < module->src_files.count; ++j) {
            if (!compile_object_file(module->src_files.items[j], module->obj_files.items[j], module->dir_path)) {
                module_ok = false;
                break; // Stop compiling this module on first error
            }
        }

        if (!module_ok) {
            nob_log(NOB_ERROR, "Failed to compile all object files for module %s", module->name);
            return 1; // Abort build
        }

        // Archive the library
        if (!build_static_library(module)) {
             nob_log(NOB_ERROR, "Failed to build static library for module %s", module->name);
             return 1; // Abort build
        }
        nob_da_append(&module_lib_paths, module->target_path); // Add successful library to list for demos
    }

    // Build Demos (Executables)
    nob_log(NOB_INFO, "--- Building Demos ---");
     for (size_t i = 0; i < demos.count; ++i) {
        BuildTarget *demo = &demos.items[i];
        nob_log(NOB_INFO, "Building demo: %s", demo->name);
         if (!nob_mkdir_if_not_exists(demo->build_subdir)) return 1;

        bool demo_ok = true;
        for (size_t j = 0; j < demo->src_files.count; ++j) {
             if (!compile_object_file(demo->src_files.items[j], demo->obj_files.items[j], demo->dir_path)) {
                demo_ok = false;
                break;
            }
        }

         if (!demo_ok) {
            nob_log(NOB_ERROR, "Failed to compile all object files for demo %s", demo->name);
            return 1; // Abort build
        }

        // Link the executable
        if (!build_executable(demo, &module_lib_paths)) {
            nob_log(NOB_ERROR, "Failed to link executable for demo %s", demo->name);
            return 1; // Abort build
        }
    }

    // --- Cleanup ---
    // Free dynamically allocated data within targets (src_files, obj_files arrays)
    // Note: The strings *inside* these arrays were allocated with temp_strdup.
    // If temp allocator was reset/rewound, these pointers might be invalid.
    // If main() managed the temp allocator correctly (only rewinding locally), it might be okay.
    // For robustness, consider using heap allocation (malloc/free) for paths stored in BuildTarget,
    // or a dedicated arena allocator.
    for (size_t i = 0; i < modules.count; ++i) {
        nob_da_free(modules.items[i].src_files);
        nob_da_free(modules.items[i].obj_files);
    }
    nob_da_free(modules); // Free the array of modules itself

    for (size_t i = 0; i < demos.count; ++i) {
        nob_da_free(demos.items[i].src_files);
        nob_da_free(demos.items[i].obj_files);
    }
    nob_da_free(demos); // Free the array of demos itself

    nob_da_free(module_lib_paths); // Free the list of library paths

    // Temp allocator is implicitly reset on exit, but explicit reset is good practice if needed
    // nob_temp_reset();

    nob_log(NOB_INFO, "Build finished successfully!");
    return 0;
}

// --- Helper Functions ---

bool parse_module(const char *module_dir_path) {
    nob_log(NOB_INFO, "Parsing module: %s", module_dir_path);
    return true;
}



// Recursively find files with a specific extension in a directory
bool find_files_recursive(const char *dir_path, const char *extension, Nob_File_Paths *result) {
    bool success = true;
    Nob_File_Paths children = {0};

    if (!nob_read_entire_dir(dir_path, &children)) {
        nob_return_defer(false);
    }

    for (size_t i = 0; i < children.count; ++i) {
        const char *child_name = children.items[i];
        if (strcmp(child_name, ".") == 0 || strcmp(child_name, "..") == 0) {
            continue;
        }

        const char *child_path = nob_temp_sprintf("%s/%s", dir_path, child_name);
        nob_log(NOB_INFO, "Checking %s from %s", child_path, child_name);
        Nob_File_Type file_type = nob_get_file_type(child_path);

        switch (file_type) {
            case NOB_FILE_REGULAR:
                if (nob_sv_end_with(nob_sv_from_cstr(child_name), extension)) {
                   nob_da_append(result, nob_temp_strdup(child_path));
                }
                break;
            case NOB_FILE_DIRECTORY:
                if (!find_files_recursive(child_path, extension, result)) {
                    nob_return_defer(false);
                }
                break;
            case NOB_FILE_SYMLINK: // Decide how to handle symlinks if needed
                 nob_log(NOB_WARNING, "Skipping symlink: %s", child_path);
                 break;
            case NOB_FILE_OTHER:
                 nob_log(NOB_WARNING, "Skipping unsupported file type: %s", child_path);
                 break;
            default:
                 nob_log(NOB_ERROR, "Could not determine file type for: %s", child_path);
                 nob_return_defer(false);
        }
    }

defer:
    nob_da_free(children); // Free the list of child names (allocated by nob_read_entire_dir)
    return success;
}

// Parse module.txt to find source files relative to the module directory
bool parse_module_txt(const char *module_dir_path, Nob_File_Paths *src_files) {
    bool result = true;
    const char *module_txt_path = nob_temp_sprintf("%s/module.txt", module_dir_path);
    Nob_String_Builder sb = {0};
    bool use_recursive_find = false;

    nob_log(NOB_INFO, "Parsing %s", module_txt_path);
    src_files->count = 0;
    if (!nob_read_entire_file(module_txt_path, &sb)) {
        nob_log(NOB_ERROR, "Could not read %s", module_txt_path);
        nob_return_defer(false);
    }

    Nob_String_View content = nob_sb_to_sv(sb);
    Nob_String_View content_copy_for_first_line_check = content;

    // --- Check the first non-empty line ---
    while (content_copy_for_first_line_check.count > 0) {
        Nob_String_View first_line = nob_sv_trim(nob_sv_chop_by_delim(&content_copy_for_first_line_check, '\n'));
        if (first_line.count > 0) {
            // Found the first non-empty line
            if (nob_sv_eq(first_line, nob_sv_from_cstr("*.c"))) {
                nob_log(NOB_INFO, "Found '*.c' in %s, searching recursively for .c files.", module_txt_path);
                use_recursive_find = true;
            }
            break; // Stop checking after the first valid line
        }
    }

    // --- Populate src_files based on the check ---
    if (use_recursive_find) {
        // Use find_files_recursive to populate src_files
        if (!find_files_recursive(module_dir_path, ".c", src_files)) {
            nob_log(NOB_ERROR, "Failed to recursively find source files in %s", module_dir_path);
            nob_return_defer(false);
        }
        // Check if any files were found
        if (src_files->count == 0) {
             nob_log(NOB_WARNING, "No .c files found recursively in %s despite using '*.c'.", module_dir_path);
             // Continue, maybe it's intentional? Or return false? Let's continue.
        }
    } else {
        while (content.count > 0) {
            Nob_String_View line = nob_sv_trim(nob_sv_chop_by_delim(&content, '\n'));
            if (line.count > 0) {
                // Assume line is a relative path from module_dir_path
                const char *rel_src_path_cstr = nob_temp_sv_to_cstr(line); // Uses temp allocator
                const char *abs_src_path = nob_temp_sprintf("%s/%s", module_dir_path, rel_src_path_cstr);
                 // Make a persistent copy for the src_files array
                nob_da_append(src_files, nob_temp_strdup(abs_src_path));
            }
        }
    }

defer:
    nob_sb_free(sb);
    return result;
}

// Compile a single source file to an object file if needed
bool compile_object_file(const char *src_path, const char *obj_path, const char *target_include_dir) {
    bool result = true;
    Nob_Cmd cmd = {0};

    // Check if rebuild is needed
    int rebuild_needed = nob_needs_rebuild1(obj_path, src_path);
    if (rebuild_needed < 0) return false; // Error occurred during check
    if (rebuild_needed == 0) {
        nob_log(NOB_INFO, "Object file %s is up to date.", obj_path);
        nob_return_defer(true); // No rebuild needed
    }

    nob_log(NOB_INFO, "Compiling %s -> %s", src_path, obj_path);

    // Ensure build subdirectory exists
    // This requires getting the directory part of obj_path
    // Let's assume build_target->build_subdir was already created in main()

    nob_cmd_append(&cmd, compiler_path);
    nob_da_append_many(&cmd, common_cflags, NOB_ARRAY_LEN(common_cflags));
    // Add include path for the target itself (module or demo dir)
    nob_cmd_append(&cmd, nob_temp_sprintf("-I%s", target_include_dir));
#ifdef _WIN32
    nob_cmd_append(&cmd, nob_temp_sprintf("/Fo%s", obj_path)); // Output object file
    nob_cmd_append(&cmd, "/c");                                // Compile only
#else
    nob_cmd_append(&cmd, "-c");                                // Compile only
    nob_cmd_append(&cmd, "-o", obj_path);                      // Output object file
#endif
    nob_cmd_append(&cmd, src_path);                            // Input source file

    if (!nob_cmd_run_sync_and_reset(&cmd)) {
        nob_log(NOB_ERROR, "Failed to compile %s", src_path);
        nob_return_defer(false);
    }

defer:
    nob_cmd_free(cmd); // Free command buffer
    return result;
}

// Build a static library from object files if needed
bool build_static_library(BuildTarget *target) {
    bool result = true;
    Nob_Cmd cmd = {0};

    // Check if rebuild is needed (library depends on all its object files)
    int rebuild_needed = nob_needs_rebuild(target->target_path,
                                           (const char **)target->obj_files.items, // Cast needed
                                           target->obj_files.count);
    if (rebuild_needed < 0) return false; // Error
    if (rebuild_needed == 0) {
        nob_log(NOB_INFO, "Static library %s is up to date.", target->target_path);
        nob_return_defer(true); // No rebuild needed
    }

    nob_log(NOB_INFO, "Archiving static library %s", target->target_path);

#ifdef _WIN32
    // Windows LIB command: lib /OUT:target_path obj1.obj obj2.obj ...
    // We might need to delete the old library first if LIB doesn't overwrite cleanly
    nob_delete_file(target->target_path); // Try deleting first
    nob_cmd_append(&cmd, archiver_path);
    nob_cmd_append(&cmd, nob_temp_sprintf("/OUT:%s", target->target_path));
    nob_da_append_many(&cmd, (const char **)target->obj_files.items, target->obj_files.count);
#else
    // POSIX ar command: ar rcs target_path obj1.o obj2.o ...
    nob_cmd_append(&cmd, archiver_path);
    nob_cmd_append(&cmd, "rcs"); // r = insert/replace, c = create, s = create index
    nob_cmd_append(&cmd, target->target_path);
    nob_da_append_many(&cmd, (const char **)target->obj_files.items, target->obj_files.count);
#endif

    if (!nob_cmd_run_sync_and_reset(&cmd)) {
        nob_log(NOB_ERROR, "Failed to archive static library %s", target->target_path);
        nob_return_defer(false);
    }

defer:
    nob_cmd_free(cmd);
    return result;
}

// Build an executable from object files and libraries if needed
bool build_executable(BuildTarget *target, Nob_File_Paths *static_libs) {
    bool result = true;
    Nob_Cmd cmd = {0};
    Nob_File_Paths dependencies = {0}; // Combine objects and libraries for dependency check

    // Add object files as dependencies
    nob_da_append_many(&dependencies, (const char **)target->obj_files.items, target->obj_files.count);
    // Add static libraries as dependencies
    nob_da_append_many(&dependencies, static_libs->items, static_libs->count);

    // Check if rebuild is needed
    int rebuild_needed = nob_needs_rebuild(target->target_path,
                                           (const char **)dependencies.items,
                                           dependencies.count);
    if (rebuild_needed < 0) nob_return_defer(false); // Error
    if (rebuild_needed == 0) {
        nob_log(NOB_INFO, "Executable %s is up to date.", target->target_path);
        nob_return_defer(true); // No rebuild needed
    }

    nob_log(NOB_INFO, "Linking executable %s", target->target_path);

#ifdef _WIN32
    // Windows Link command: cl /Fe:target_path obj1.obj ... lib1.lib ... /link common_libs ... ldflags
    nob_cmd_append(&cmd, compiler_path); // Use compiler for linking on Windows
    nob_cmd_append(&cmd, nob_temp_sprintf("/Fe%s", target->target_path)); // Output exe
    nob_da_append_many(&cmd, (const char **)target->obj_files.items, target->obj_files.count); // Add objects
    nob_da_append_many(&cmd, static_libs->items, static_libs->count); // Add module libs
    nob_cmd_append(&cmd, "/link");
    nob_da_append_many(&cmd, common_ldflags, NOB_ARRAY_LEN(common_ldflags));
    nob_da_append_many(&cmd, common_libs, NOB_ARRAY_LEN(common_libs)); // Add system libs
#else
    // POSIX Link command: cc -o target_path obj1.o ... lib1.a ... ldflags common_libs
    nob_cmd_append(&cmd, compiler_path);
    nob_cmd_append(&cmd, "-o", target->target_path); // Output exe
    nob_da_append_many(&cmd, (const char **)target->obj_files.items, target->obj_files.count); // Add objects
    // Add library paths and library names correctly
    // This might require extracting paths and base names if libs are not in standard locations
    // Assuming static_libs contains full paths:
    nob_da_append_many(&cmd, static_libs->items, static_libs->count); // Add module libs
    nob_da_append_many(&cmd, common_ldflags, NOB_ARRAY_LEN(common_ldflags));
    nob_da_append_many(&cmd, common_libs, NOB_ARRAY_LEN(common_libs)); // Add system libs (like -lm)
#endif

    if (!nob_cmd_run_sync_and_reset(&cmd)) {
        nob_log(NOB_ERROR, "Failed to link executable %s", target->target_path);
        nob_return_defer(false);
    }


defer:
    nob_cmd_free(cmd);
    nob_da_free(dependencies);
    return result;
}

void print_usage(const char *program_name) {
    nob_log(NOB_INFO, "Usage: %s [options]", program_name);
    nob_log(NOB_INFO, "Options:");
    nob_log(NOB_INFO, "  clean    Remove the build directory");
    // Add more options if needed
}

void clean_dir(const char *path) {
    Nob_File_Paths dirents = {0};

    if (!nob_read_entire_dir(path, &dirents)) {
        nob_log(NOB_ERROR, "Failed to read build directory: %s", path);
        return;
    }

    for (size_t i = 0; i < dirents.count; ++i) {
        const char *entry_path = dirents.items[i];
        if (strcmp(entry_path, ".") == 0 || strcmp(entry_path, "..") == 0) {
            continue; // Skip special dirs
        }
        nob_delete_file(entry_path); // Delete files and directories
    }

    nob_da_free(dirents); // Free directory listing
}

bool collect_modules(const char *root, BuildTargets *modules, bool is_exe) {
    Nob_File_Paths module_paths = {0};
    if (!nob_read_entire_dir(root, &module_paths)) {
        return false;
    }
    for (size_t i = 0; i < module_paths.count; ++i) {
        const char *entry_name = module_paths.items[i];
        if (strcmp(entry_name, ".") == 0 || strcmp(entry_name, "..") == 0) {
            continue; // Skip special dirs
        }

        const char *entry_path = nob_temp_sprintf("%s/%s", root, entry_name); // Uses temp allocator
        if (nob_get_file_type(entry_path) == NOB_FILE_DIRECTORY) {
            const char *module_txt_path = nob_temp_sprintf("%s/module.txt", entry_path);
            if (nob_file_exists(module_txt_path) == 1) {
                // Found a module directory
                nob_log(NOB_INFO, "Found module: %s", entry_name);
                BuildTarget module = {0};
                module.name = nob_temp_strdup(entry_name); // Persistent copy needed
                module.dir_path = nob_temp_strdup(entry_path); // Persistent copy needed
                module.is_executable = is_exe;
                module.build_subdir = nob_temp_sprintf("%s/%s", build_dir, module.name);
                char *ext = is_exe ? EXE_EXT : STATIC_LIB_EXT;
                module.target_path = nob_temp_sprintf("%s/%s%s", module.build_subdir, module.name, ext);
                
                if (!parse_module_txt(module.dir_path, &module.src_files)) {
                    nob_log(NOB_ERROR, "Failed to parse module.txt for %s", module.name);
                    return false;
                }

                if (module.src_files.count == 0) {
                    nob_log(NOB_WARNING, "Module %s has no source files listed in module.txt. Skipping.", module.name);
                    // Free allocated strings for this skipped module
                    // Need careful temp allocator rewinding or heap allocation for names/paths
                    // Let's assume temp allocation works if managed carefully in main scope
                    continue;
                }

                // Determine object file paths
                for (size_t j = 0; j < module.src_files.count; ++j) {
                    const char *src_file_path = module.src_files.items[j];
                    const char *src_file_name = nob_path_name(src_file_path); // Get "file.c"
                    const char *obj_file_name = nob_temp_sprintf("%.*s%s", (int)strlen(src_file_name) - 2, src_file_name, OBJ_EXT); // Replace .c with .obj/.o
                    const char *obj_file_path = nob_temp_sprintf("%s/%s", module.build_subdir, obj_file_name);
                    nob_da_append(&module.obj_files, nob_temp_strdup(obj_file_path)); // Persistent copy
                }

                nob_da_append(modules, module); // Append the module to the list
            }
        }
    } 
    nob_da_free(module_paths); // Free the list of module paths
    return true;
}


