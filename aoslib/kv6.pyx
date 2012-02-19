# Copyright (c) Mathias Kaerlev 2011-2012.

# This file is part of kv6view.

# kv6view is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# kv6view is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with kv6view.  If not, see <http://www.gnu.org/licenses/>.

cdef extern from "include_gl.h":
    ctypedef unsigned int GLuint

cdef extern from "kv6_c.cpp":
    enum:
        FACE_NORMAL_INDEX_C "FACE_NORMAL_INDEX"

    struct RGB:
        unsigned char r, g, b, a
    
    struct VOXType:
        RGB color
        unsigned short z
        unsigned char visibility, normal_index
        
    struct KV6Data:
        unsigned int x_size, y_size, z_size
        float x_piv, y_piv, z_piv
        unsigned int voxel_count
        VOXType * vox
        unsigned int * x_entries
        unsigned short * xy_entries
    
    struct KV6Display:
        pass

    KV6Data * load_kv6(char * filename)
    void save_kv6(KV6Data * data, char * filename)
    KV6Display * create_display(KV6Data * data)
    void draw_display(KV6Display * display)
    void initialize_c "initialize"()

initialize_c()

import random

MAGIC = 'Kvxl'
PALETTE_MAGIC = 'SPal'

FACE_NORMAL_INDEX = FACE_NORMAL_INDEX_C

cdef class Display:
    cdef:
        KV6Display * display
        KV6 data
    
    def __init__(self, KV6 data):
        self.data = data
        self.display = create_display(data.data)
    
    def draw(self):
        draw_display(self.display)

cdef class KV6:
    cdef:
        KV6Data * data
        
    def __init__(self, filename):
        self.data = load_kv6(filename)
        if self.data == NULL:
            raise NotImplementedError('bad kv6 file: %r' % filename)
    
    def save(self, filename):
        save_kv6(self.data, filename)
    
    def get_display(self):
        return Display(self)
    
    def offset_pivot_point(self, x, y, z):
        self.data.x_piv += x
        self.data.y_piv += y
        self.data.z_piv += z
    
    def get_pivot_point(self):
        cdef KV6Data * data = self.data
        return data.x_piv, data.y_piv, data.z_piv
    
    def get_points(self):
        cdef KV6Data * data = self.data
        cdef RGB color
        cdef VOXType point
        cdef int x, y, a
        cdef int i = 0
        cdef list points = []
        for x in range(data.x_size):
            for y in range(data.y_size):
                for a in range(data.xy_entries[x*data.y_size+y]):
                    point = data.vox[i]
                    color = point.color
                    points.append((x, y, point.z, (color.r, color.g, color.b)))
                    i += 1
        return points