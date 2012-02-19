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

#include "common_c.h"
#include <math.h>

#define GOLDRAT 0.3819660112501052f //Golden Ratio: 1 - 1/((sqrt(5)+1)/2)
#define PI 3.141592653589793f
#define LUT_POINTS (255)
#define ZMULK (2.0f / LUT_POINTS)
#define ZADDK (ZMULK*0.5f - 1)

Vector univec[256];

bool custom_isnan(float var)
{
    volatile float d = var;
    return d != d;
}

void init_ind2vec(long i, float *x, float *y, float *z)
{
    float r;
    (*z) = (float)i*ZMULK + ZADDK;
    r = sqrtf(1.f - (*z)*(*z));
    fcossin((float)i*(GOLDRAT*PI*2),x,y);
    (*x) *= r;
    (*y) *= r;
}

void initialize_vector_table()
{
    for(int z=LUT_POINTS-1;z>=0;z--)
        init_ind2vec(z, &univec[z].x, &univec[z].y, &univec[z].z);
    // no shading
    univec[LUT_POINTS].x = univec[LUT_POINTS].y = univec[LUT_POINTS].z = 0;
}

Vector * index_to_vector(long i)
{
    return &univec[i];
}