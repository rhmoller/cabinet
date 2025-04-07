#ifndef CAB_ARENA_H
#define CAB_ARENA_H

typedef struct Cab_Arena {
    int size;
    int capacity;
    void *data;
} Cab_Arena;

Cab_Arena *cab_arena_create(int initial_capacity);
void cab_arena_destroy(Cab_Arena *arena);

void *cab_arena_alloc(Cab_Arena *arena, int size);
void cab_arena_reset(Cab_Arena *arena);

#endif // CAB_ARENA_H


