#include "arena.h"
#include <stdlib.h>

Cab_Arena *cab_arena_create(int initial_capacity) {
    Cab_Arena *arena = (Cab_Arena *)malloc(sizeof(Cab_Arena) + initial_capacity);
    if (!arena) {
        return NULL;
    }
    arena->size = 0;
    arena->capacity = initial_capacity;
    arena->data = ((char *)arena + sizeof(Cab_Arena));
    return arena;
}

void cab_arena_destroy(Cab_Arena *arena) {
    free(arena);
}

void *cab_arena_alloc(Cab_Arena *arena, int size) {
    if (arena->size + size > arena->capacity) {
        return NULL;
    }
    void *ptr = (char *)arena->data + arena->size;
    arena->size += size;
    return ptr;
}

void cab_arena_reset(Cab_Arena *arena) {
    arena->size = 0;
}
