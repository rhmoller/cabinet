#ifndef CABINET_H
#define CABINET_H

#include "sokol_app.h"

typedef struct Cab_Cabinet {
    char *name;
} Cab_Cabinet;

sapp_desc cab_sokol_main(int argc, char* argv[], Cab_Cabinet *cabinet, void (*init_cb)(), void (*update_cb)());
 
#endif // CABINET_H
