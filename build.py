import sys
from distutils.core import setup
from Cython.Distutils.extension import Extension
from Cython.Distutils import build_ext

macros = [('GLEW_STATIC', 1)]
ext_modules = []
include_dirs = ['.', './include', './aoslib/']
lib_dirs = ['./lib']

extra_link_args = []
extra_compile_args = []

opengl_libraries = ['glew32s']
opengl_link_args = extra_link_args + []

if sys.platform == 'win32':
    opengl_libraries.append('opengl32')
elif sys.platform == 'linux2':
    opengl_libraries.append('GL')
elif sys.platform == 'darwin':
    opengl_link_args.extend(['-framework', 'OpenGL'])

ext_modules.append(Extension('aoslib.kv6', 
    [
        './aoslib/kv6.pyx'
    ], include_dirs = include_dirs, libraries = opengl_libraries,
    library_dirs = lib_dirs, extra_link_args=opengl_link_args, 
    extra_compile_args = extra_compile_args, define_macros = macros,
    language = 'c++'))

setup(
    name = 'OpenGL port extensions',
    cmdclass = {'build_ext' : build_ext},
    ext_modules = ext_modules
)