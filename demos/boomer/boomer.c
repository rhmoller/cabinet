#include "cabinet.h"

void init(Cab_Cabinet* cabinet) {
}

void update(Cab_Cabinet* cabinet) {
}

sapp_desc sokol_main(int argc, char* argv[]) {
    return cab_sokol_main(argc, argv, init, update);
}

