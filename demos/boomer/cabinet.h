#ifndef CABINET_H
#define CABINET_H

#include "sokol_app.h"

typedef struct Cab_Cabinet {
    char *name;
} Cab_Cabinet;

sapp_desc cab_sokol_main(int argc, char* argv[], void (*init_cb)(Cab_Cabinet*), void (*update_cb)(Cab_Cabinet*));
 
#endif // CABINET_H
