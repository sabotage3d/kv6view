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

from pyglet.gl import *
from ctypes import *

class CompileError(SystemExit):
    pass

class Shader(object):
    initialized = False
    def __init__(self, vert = [], frag = []):
        self.vert = vert
        self.frag = frag
    
    def initialize(self):
        self.handle = glCreateProgram()
        
        for item in self.vert:
            self.create_shader(item, GL_VERTEX_SHADER)
        
        for item in self.frag:
            self.create_shader(item, GL_FRAGMENT_SHADER)

    def create_shader(self, item, type):
        shader = glCreateShader(type)
        src = (c_char_p * 1)(item)
        glShaderSource(shader, 1, cast(pointer(src), POINTER(POINTER(c_char))), None)
        glCompileShader(shader)
        temp = c_int(0)
        glGetShaderiv(shader, GL_COMPILE_STATUS, byref(temp))

        if not temp:
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, byref(temp))
            buffer = create_string_buffer(temp.value)
            glGetShaderInfoLog(shader, temp, None, buffer)
            print 'Shader error: %s' % buffer.value
        else:
            glAttachShader(self.handle, shader);

    def link(self):
        glLinkProgram(self.handle)

        temp = c_int(0)
        glGetProgramiv(self.handle, GL_LINK_STATUS, byref(temp))

        if not temp:
            glGetProgramiv(self.handle, GL_INFO_LOG_LENGTH, byref(temp))
            buffer = create_string_buffer(temp.value)
            glGetProgramInfoLog(self.handle, temp, None, buffer)
            raise CompileError(buffer.value)

    def bind(self, size = None):
        glUseProgram(self.handle)
    
    @staticmethod
    def unbind():
        glUseProgram(0)

    def uniformf(self, name, *vals):
        if len(vals) in range(1, 5):
            loc = glGetUniformLocation(self.handle, name)
            { 1 : glUniform1f,
                2 : glUniform2f,
                3 : glUniform3f,
                4 : glUniform4f
            }[len(vals)](loc, *vals)

    def uniformi(self, name, *vals):
        if len(vals) in range(1, 5):
            loc = glGetUniformLocation(self.handle, name)
            { 
                1 : glUniform1i,
                2 : glUniform2i,
                3 : glUniform3i,
                4 : glUniform4i
            }[len(vals)](loc, *vals)
            
    def uniform_vec2(self, name, mat):
        loc = glGetUniformLocation(self.handle, name)
        glUniformMatrix2fv(loc, 1, False, (c_float * 4)(*mat))

    def uniform_vec3(self, name, mat):
        loc = glGetUniformLocation(self.handle, name)
        glUniformMatrix3fv(loc, 1, False, (c_float * 9)(*mat))
        
    def uniform_vec4(self, name, mat):
        loc = glGetUniformLocation(self.handle, name)
        glUniformMatrix4fv(loc, 1, False, (c_float * 16)(*mat))