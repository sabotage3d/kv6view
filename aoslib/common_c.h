/*
    Copyright (c) Mathias Kaerlev 2011-2012.

    This file is part of pyspades.

    pyspades is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pyspades is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with pyspades.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef COMMON_H
#define COMMON_H

#include <math.h>

struct Vector
{
    float x, y, z;
};

struct IntVector
{
    int x, y, z;
};

// silly VOXLAP functions
float distance3d(float x1, float y1, float z1, float x2, float y2, float z2)
{
	return sqrtf(pow(x2-x1, 2) + pow(y2-y1,2) + pow(z2-z1,2));
}

inline void ftol(float f, int *a)
{
    *a = (int)f;
}

inline void fcossin (float a, float *c, float *s)
{
    *c = cos(a);
    *s = sin(a);
}
#endif