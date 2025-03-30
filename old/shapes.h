#ifndef SHAPES_H
#define SHAPES_H

#include "cabinet.h"

Geometry *createCubeGeometry(Gfx *gfx);

Geometry *create_text_geometry(Gfx *gfx, float *text_vertices, int nChars);

int write_text(float *vertices, char *text, float xStart, float yStart);

#endif // SHAPES_H
