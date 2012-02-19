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
#include "include_gl.h"
#include "shade.h"
#include <vector>
#include <stdio.h>

using namespace std;

#define FACE_NORMAL_INDEX 1

void initialize()
{
    glewInit();
    initialize_vector_table();
}

struct RGB
{
    unsigned char b, g, r, a;
};

struct VOXType {
    RGB color;
    unsigned short z;
    unsigned char visibility, normal_index;
};

struct KV6Data
{
    unsigned int x_size, y_size, z_size;
    float x_piv, y_piv, z_piv;
    unsigned int voxel_count;
    VOXType * vox;
    unsigned int * x_entries;
    unsigned short * xy_entries;
};

KV6Data * load_kv6(char * filename)
{
    FILE *fil;
    KV6Data tk, *newkv6;
    long i;

    if (!(fil = fopen(filename,"rb")))
        return NULL;

    fseek(fil, 4, SEEK_SET); // skip 'Kvxl'
    fread((void *)&tk,28,1,fil);

    i = tk.voxel_count*sizeof(VOXType) + (tk.x_size*4) + ((
        tk.x_size*tk.y_size)*2);
    newkv6 = (KV6Data *)malloc(i+sizeof(KV6Data));
    if (!newkv6) {
        fclose(fil); 
        return 0;
    }
    memcpy(&newkv6->x_size,&tk.x_size,28);
    newkv6->vox = (VOXType *)(((char*)newkv6)+sizeof(KV6Data));
    newkv6->x_entries = (unsigned int *)(((char*)newkv6->vox) + tk.voxel_count*sizeof(
        VOXType));
    newkv6->xy_entries = (unsigned short *)(((char*)newkv6->x_entries) + 
        tk.x_size*4);
    fread((void *)newkv6->vox,i,1,fil);
    fclose(fil);
    return newkv6;
}

void save_kv6(KV6Data * data, char * filename)
{
    FILE * fp;
    long i;

    if (fp = fopen(filename, "wb"))
    {
        i = 0x6c78764b; // Kvxl
        fwrite(&i,4,1,fp);
        fwrite(&data->x_size,4,1,fp);
        fwrite(&data->y_size,4,1,fp);
        fwrite(&data->z_size,4,1,fp);
        fwrite(&data->x_piv,4,1,fp);
        fwrite(&data->y_piv,4,1,fp);
        fwrite(&data->z_piv,4,1,fp);
        fwrite(&data->voxel_count,4,1,fp);
        fwrite(data->vox,data->voxel_count*sizeof(VOXType), 1, fp);
        fwrite(data->x_entries,data->x_size*sizeof(long),1,fp);
        fwrite(data->xy_entries,data->x_size*data->y_size*sizeof(short),1,fp);
        fclose(fp);
    }
}

/*
char vis in VOXType:
first 6 bits -> visible faces:

if (!isvoxelsolid(x-1,y,z)) voxptr[numvoxs].vis |= 1;
if (!isvoxelsolid(x+1,y,z)) voxptr[numvoxs].vis |= 2;
if (!isvoxelsolid(x,y-1,z)) voxptr[numvoxs].vis |= 4;
if (!isvoxelsolid(x,y+1,z)) voxptr[numvoxs].vis |= 8;
if (!isvoxelsolid(x,y,z-1)) voxptr[numvoxs].vis |= 16;
if (!isvoxelsolid(x,y,z+1)) voxptr[numvoxs].vis |= 32;
*/

#define LEFT_VISIBLE 1
#define RIGHT_VISIBLE 2
#define BACK_VISIBLE 4
#define FRONT_VISIBLE 8
#define TOP_VISIBLE 16
#define BOTTOM_VISIBLE 32

struct KV6Display
{
    GLuint vbo;
    unsigned int vertex_count;
};

#pragma pack(push)
#pragma pack(1)

/*
x, y, z = position
nx, ny, nz = normals (from KV6)
fx, fy, fz = face normals
r, g, b = color
*/

struct Quad
{
    float x1, y1, z1;
    float nx1, ny1, nz1;
    float fx1, fy1, fz1;
    unsigned char r1, g1, b1;
    float x2, y2, z2;
    float nx2, ny2, nz2;
    float fx2, fy2, fz2;
    unsigned char r2, g2, b2;
    float x3, y3, z3;
    float nx3, ny3, nz3;
    float fx3, fy3, fz3;
    unsigned char r3, g3, b3;
    float x4, y4, z4;
    float nx4, ny4, nz4;
    float fx4, fy4, fz4;
    unsigned char r4, g4, b4;
};

struct Vertex
{
    float x, y, z;
    float nx, ny, nz;
    float fx, fy, fz;
    unsigned char r, g, b;
};
#pragma pack(pop)

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

void draw_display(KV6Display * display)
{
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, display->vbo);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableVertexAttribArray(FACE_NORMAL_INDEX);
    int stride = sizeof(Vertex);
    glVertexPointer(3, GL_FLOAT, stride, 0);
    glNormalPointer(GL_FLOAT, stride, 
        BUFFER_OFFSET(sizeof(float) * 3));
    glVertexAttribPointer(FACE_NORMAL_INDEX, 3, GL_FLOAT, GL_FALSE, stride,
        BUFFER_OFFSET(sizeof(float) * 6));
    glColorPointer(3, GL_UNSIGNED_BYTE, stride, 
        BUFFER_OFFSET(sizeof(float) * 9));
    glDrawArrays(GL_QUADS, 0, display->vertex_count * 4);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableVertexAttribArray(FACE_NORMAL_INDEX);
}

// XXX should use VBO index arrays for vertices
KV6Display * create_display(KV6Data * data)
{
    KV6Display * display = new KV6Display;
    glGenBuffersARB(1, &display->vbo);
    vector<Quad> out;
    VOXType * point;
    unsigned int x, y, z, c;
    unsigned char vis;
    float gl_x, gl_y, gl_z;
    unsigned int i = 0;
    Quad v;
    for (x = 0; x < data->x_size; x++) {
    for (y = 0; y < data->y_size; y++) {
    for (c = 0; c < data->xy_entries[x * data->y_size+y]; c++) {
        point = &data->vox[i];
        z = point->z;
        vis = point->visibility;
        gl_x = ((float)x - data->x_piv);
        gl_y = -((float)z - data->z_piv);
        gl_z = ((float)y - data->y_piv);
        Vector * normal = index_to_vector(point->normal_index);
        v.r1 = v.r2 = v.r3 = v.r4 = point->color.r;
        v.g1 = v.g2 = v.g3 = v.g4 = point->color.g;
        v.b1 = v.b2 = v.b3 = v.b4 = point->color.b;
        v.nx1 = v.nx2 = v.nx3 = v.nx4 = normal->x;
        v.ny1 = v.ny2 = v.ny3 = v.ny4 = normal->y;
        v.nz1 = v.nz2 = v.nz3 = v.nz4 = normal->z;
        // Top Face
        if (vis & TOP_VISIBLE) {
            v.x1 = gl_x + -0.5f; v.y1 = gl_y +  0.5f; v.z1 = gl_z + -0.5f;
            v.x2 = gl_x + -0.5f; v.y2 = gl_y +  0.5f; v.z2 = gl_z +  0.5f;
            v.x3 = gl_x +  0.5f; v.y3 = gl_y +  0.5f; v.z3 = gl_z +  0.5f;
            v.x4 = gl_x +  0.5f; v.y4 = gl_y +  0.5f; v.z4 = gl_z + -0.5f;
            v.fx1 = v.fx2 = v.fx3 = v.fx4 = 0.0f;
            v.fy1 = v.fy2 = v.fy3 = v.fy4 = 1.0f;
            v.fz1 = v.fz2 = v.fz3 = v.fz4 = 0.0f;
            out.push_back(v);
        }
        // Bottom face
        if (vis & BOTTOM_VISIBLE) {
            v.x1 = gl_x + -0.5f; v.y1 = gl_y + -0.5f; v.z1 = gl_z + -0.5f; // Top right
            v.x2 = gl_x +  0.5f; v.y2 = gl_y + -0.5f; v.z2 = gl_z + -0.5f; // Top left
            v.x3 = gl_x +  0.5f; v.y3 = gl_y + -0.5f; v.z3 = gl_z +  0.5f; // Bottom left
            v.x4 = gl_x + -0.5f; v.y4 = gl_y + -0.5f; v.z4 = gl_z +  0.5f; // Bottom right
            v.fx1 = v.fx2 = v.fx3 = v.fx4 = 0.0f;
            v.fy1 = v.fy2 = v.fy3 = v.fy4 = -1.0f;
            v.fz1 = v.fz2 = v.fz3 = v.fz4 = 0.0f;
            out.push_back(v);
        }
        // Front Face
        if (vis & FRONT_VISIBLE) {
            v.x1 = gl_x + -0.5f; v.y1 = gl_y + -0.5f; v.z1 = gl_z +  0.5f;  // Bottom left
            v.x2 = gl_x +  0.5f; v.y2 = gl_y + -0.5f; v.z2 = gl_z +  0.5f;  // Bottom right
            v.x3 = gl_x +  0.5f; v.y3 = gl_y +  0.5f; v.z3 = gl_z +  0.5f;  // Top right
            v.x4 = gl_x + -0.5f; v.y4 = gl_y +  0.5f; v.z4 = gl_z +  0.5f;  // Top left
            v.fx1 = v.fx2 = v.fx3 = v.fx4 = 0.0f;
            v.fy1 = v.fy2 = v.fy3 = v.fy4 = 0.0f;
            v.fz1 = v.fz2 = v.fz3 = v.fz4 = 1.0f;
            out.push_back(v);
        }
        // Back Face
        if (vis & BACK_VISIBLE) {
            v.x1 = gl_x + -0.5f; v.y1 = gl_y + -0.5f; v.z1 = gl_z + -0.5f;  // Bottom right
            v.x2 = gl_x + -0.5f; v.y2 = gl_y +  0.5f; v.z2 = gl_z + -0.5f;  // Top right
            v.x3 = gl_x +  0.5f; v.y3 = gl_y +  0.5f; v.z3 = gl_z + -0.5f;  // Top left
            v.x4 = gl_x +  0.5f; v.y4 = gl_y + -0.5f; v.z4 = gl_z + -0.5f;  // Bottom left
            v.fx1 = v.fx2 = v.fx3 = v.fx4 = 0.0f;
            v.fy1 = v.fy2 = v.fy3 = v.fy4 = 0.0f;
            v.fz1 = v.fz2 = v.fz3 = v.fz4 = -1.0f;
            out.push_back(v);
        }
        // Right face
        if (vis & RIGHT_VISIBLE) {
            v.x1 = gl_x +  0.5f; v.y1 = gl_y + -0.5f; v.z1 = gl_z + -0.5f;  // Bottom right
            v.x2 = gl_x +  0.5f; v.y2 = gl_y +  0.5f; v.z2 = gl_z + -0.5f;  // Top right
            v.x3 = gl_x +  0.5f; v.y3 = gl_y +  0.5f; v.z3 = gl_z +  0.5f;  // Top left
            v.x4 = gl_x +  0.5f; v.y4 = gl_y + -0.5f; v.z4 = gl_z +  0.5f;  // Bottom left
            v.fx1 = v.fx2 = v.fx3 = v.fx4 = 1.0f;
            v.fy1 = v.fy2 = v.fy3 = v.fy4 = 0.0f;
            v.fz1 = v.fz2 = v.fz3 = v.fz4 = 0.0f;
            out.push_back(v);
        }
        // Left Face
        if (vis & LEFT_VISIBLE) {
            v.x1 = gl_x + -0.5f; v.y1 = gl_y + -0.5f; v.z1 = gl_z + -0.5f;  // Bottom left
            v.x2 = gl_x + -0.5f; v.y2 = gl_y + -0.5f; v.z2 = gl_z +  0.5f;  // Bottom right
            v.x3 = gl_x + -0.5f; v.y3 = gl_y +  0.5f; v.z3 = gl_z +  0.5f;  // Top right
            v.x4 = gl_x + -0.5f; v.y4 = gl_y +  0.5f; v.z4 = gl_z + -0.5f;  // Top left
            v.fx1 = v.fx2 = v.fx3 = v.fx4 = -1.0f;
            v.fy1 = v.fy2 = v.fy3 = v.fy4 = 0.0f;
            v.fz1 = v.fz2 = v.fz3 = v.fz4 = 0.0f;
            out.push_back(v);
        }
        i += 1;
    }}}

    display->vertex_count = out.size();
    unsigned int size = display->vertex_count * sizeof(Quad);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, display->vbo);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, (void*)(&out[0]),
        GL_STATIC_DRAW_ARB);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    return display;
}