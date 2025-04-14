#include "cabinet.h"

Cab_Cabinet cabinet = {};

void init() {
}

void update() {
}

sapp_desc sokol_main(int argc, char* argv[]) {
    return cab_sokol_main(argc, argv, &cabinet, init, update);
}

