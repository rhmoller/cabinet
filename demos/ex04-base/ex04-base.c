#include "arena.h"
#include <stdio.h>

void do_something() {
    Cab_Arena *arena = cab_arena_create(2048);
    cab_arena_alloc(arena, 128);
    char *buf = cab_arena_alloc(arena, 512);
    sprintf(buf, "Allocated %d bytes in the arena.\n", arena->size);
    printf("%s", buf);
    printf("Arena size: %d\n", arena->capacity);
    cab_arena_destroy(arena);
}

int main(int argc, char *argv[]) {
    printf("Hello again, World!\n");
    for (int i = 0; i < 2; i++) {
        do_something();
    }
    return 0;
}
