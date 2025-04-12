#include "arena.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    Cab_Arena *arena = cab_arena_create(1024);
    //cab_arena_destroy(arena);
    return 0;
}
