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

import sys
import math
import pyglet

# make sure we have a context
from pyglet.window import Window, key
pyglet.options['shadow'] = False
config = pyglet.gl.Config(buffers=2, samples=4)
window = pyglet.window.Window(resizable = True, config = config, vsync = False,
    visible = True, width = 800, height = 600)
window.invalid = False
window.set_exclusive_mouse(True)

from pyglet.gl import *
from aoslib import kv6
from aoslib.shaders import *
        
glEnable(GL_DEPTH_TEST)
glDepthFunc(GL_LEQUAL)
glEnable(GL_CULL_FACE)
glShadeModel(GL_SMOOTH)
glClearDepth(1.0)
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE)
glEnable(GL_BLEND)
glEnable(GL_LIGHT0)
glEnable(GL_LIGHTING)
glEnable(GL_NORMALIZE)
glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST)
glEnable(GL_FOG)
glFogi(GL_FOG_MODE, GL_LINEAR)
glFogf(GL_FOG_DENSITY, 0.009)
glHint(GL_FOG_HINT, GL_NICEST)
glFogi(GL_FOG_START, 100)
glFogi(GL_FOG_END, 128)

keyboard = key.KeyStateHandler()

class Model(object):
    def __init__(self, name):
        self.display = kv6.KV6(name).get_display()
    
    def draw(self):
        MODEL_SHADER.bind()
        self.display.draw()
        MODEL_SHADER.unbind()

POSITION_NORMALIZE = 10.0
VIEW_NORMALIZE = 5.0
SPEED_NORMALIZE = 10.0

def interpolate(old, new, div):
    return old + (new - old) / float(div)

class Camera(object):
    travel_speed = 30.0
    sensitivity = 0.2
    
    current_travel_speed = 0
    o_x = o_y = o_z = 0
    x, y, z = 0, 0, -20
    or_x = or_y = or_z = 0
    r_x = r_y = r_z = 0
    view_x = view_y = view_z = view_x2 = view_z2 = 0

    def __init__(self):
        pass
    
    def add_mouse_motion(self, dx, dy):
        dx *= self.sensitivity
        dy *= self.sensitivity
        self.r_x = min(90, max(-90, self.r_x + dy)) # pitch
        self.r_y -= dx # yaw
        val1 = math.cos(math.radians(self.r_y + 90.0))
        val2 = -math.sin(math.radians(self.r_y + 90.0))
        cosX = math.cos(math.radians(self.r_x))
        self.view_x = val1 * cosX
        self.view_z = val2 * cosX
        self.view_x2 = val1 * 0.7
        self.view_z2 = val2 * 0.7
        self.view_y = math.sin(math.radians(self.r_x))
    
    def transform(self):
        self.or_x = interpolate(self.or_x, self.r_x, VIEW_NORMALIZE)
        self.or_y = interpolate(self.or_y, self.r_y, VIEW_NORMALIZE)
        self.or_z = interpolate(self.or_z, self.r_z, VIEW_NORMALIZE)
        self.o_x = interpolate(self.o_x, self.x, POSITION_NORMALIZE)
        self.o_y = interpolate(self.o_y, self.y, POSITION_NORMALIZE)
        self.o_z = interpolate(self.o_z, self.z, POSITION_NORMALIZE)
        glRotatef(-self.or_x, 1.0, 0.0, 0.0)
        glRotatef(-self.or_y, 0.0, 1.0, 0.0)
        glRotatef(-self.or_z, 0.0, 0.0, 1.0)
        glTranslatef(self.o_x, self.o_y, self.o_z)
    
    def update(self, dt):
        forward = keyboard[key.W]
        backward = keyboard[key.S]
        left = keyboard[key.A]
        right = keyboard[key.D]
        up = keyboard[key.SPACE]
        down = keyboard[key.RCTRL] or keyboard[key.LCTRL]
        if any((forward, backward, left, right, up, down)):
            dest_speed = self.travel_speed
        else:
            dest_speed = 0
            
        self.current_travel_speed = interpolate(self.current_travel_speed,
            dest_speed, SPEED_NORMALIZE)
            
        speed = self.current_travel_speed * dt
        if forward:
            self.x += self.view_x * -speed
            self.y += self.view_y * -speed
            self.z += self.view_z * -speed
        if backward:
            self.x += self.view_x * speed
            self.y += self.view_y * speed
            self.z += self.view_z * speed
        if left:
            self.x += self.view_z2 * -speed
            self.z += -self.view_x2 * -speed
        if right:
            self.x += self.view_z2 * speed
            self.z += -self.view_x2 * speed
        if up:
            self.y -= speed * 0.7
        if down:
            self.y += speed * 0.7

camera = Camera()
model = Model('test.kv6')

@window.event
def on_key_press(button, modifiers):
    keyboard.on_key_press(button, modifiers)

@window.event
def on_key_release(button, modifiers):
    keyboard.on_key_release(button, modifiers)

@window.event
def on_mouse_motion(x, y, dx, dy):
    camera.add_mouse_motion(dx, dy)

@window.event
def on_resize(width, height):
    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    fov = 75.0
    gluPerspective(fov, float(width)/(height or 1.0), 0.1, 128)
    glMatrixMode(GL_MODELVIEW)

VECTOR_4 = (GLfloat * 4)

def draw(dt):
    window.switch_to()
    on_resize(window.width, window.height)
    glClearColor(1.0, 0.0, 1.0, 1.0)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
    glLoadIdentity()
    camera.transform()
    glLightfv(GL_LIGHT0, GL_POSITION, VECTOR_4(1.0, 1.0, 0.0, 0.0))
    model.draw()
    window.flip()

def update(dt):
    camera.update(dt)

pyglet.clock.schedule_interval(update, 1/60.0)
pyglet.clock.schedule_interval(draw, 1/60.0)
pyglet.app.run()
