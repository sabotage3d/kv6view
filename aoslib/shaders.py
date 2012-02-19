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

from shader import Shader
from pyglet.gl import glBindAttribLocationARB
from kv6 import FACE_NORMAL_INDEX

def load_shader(name, attributes = (), **defaults):
    vert = open('./shaders/%s.vert' % name, 'rb').read()
    frag = open('./shaders/%s.frag' % name, 'rb').read()
    shader = Shader([vert], [frag])
    shader.initialize()
    for (index, name) in attributes:
        glBindAttribLocationARB(shader.handle, index, name)
    shader.link()
    shader.bind()
    for k, v in defaults.iteritems():
        try:
            v = tuple(v)
        except TypeError:
            v = (v,)
        if isinstance(v[0], float):
            func = shader.uniformf
        elif isinstance(v[0], int):
            func = shader.uniformi
        func(k, *v)
    shader.unbind()
    return shader

MODEL_SHADER = load_shader('model', blend_color = (1.0, 1.0, 1.0, 1.0),
    default_color = (0.0, 0.0, 0.0), 
    attributes = [(FACE_NORMAL_INDEX, 'face_normal')])

__all__ = ['MODEL_SHADER']