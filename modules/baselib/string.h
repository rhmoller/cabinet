#ifndef CAB_STRING_H
#define CAB_STRING_H

typedef struct Cab_String {
    int size;
    int capacity;
    char *data;
} Cab_String;

#endif // CAB_STRING_H
