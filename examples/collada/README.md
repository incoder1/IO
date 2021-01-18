# COLLADA 3D model viewer example

This demonstration program is 3D model viewer which can load 3D assets from [COLLADA](https://www.khronos.org/collada/) format files.
COLLADA is XML based fromat, this parser is created using IO XML StAX parser.

This is not full featured COLLADA asset loader! It is developed to demonstrate ability of IO XML parser practical usage.

## OpenGL 4.2+
In order to run this example your system should suport at least [OpenGL 4.2 +](https://www.khronos.org/opengl/) preferrably hardware accelerated. 
Make sure you have installed video card drivers with hardware accelerated 3D graphics. Otherwise MESA 3D in software mode can be used (slow).
> Note: Unix/Linux more likelly you'll need proprietary video card drivers from your card vendor. Check your distoro documentation.

## Additional libraries you'll need for 3D graphics engine

| Library  | Description | Minimal required version | Platform |
 --- | --- | --- | --- 
| [glwf](https://www.glfw.org/) | API for creating windows, contexts and surfaces, receiving input and events. | 3.0 | ALL |
| [glew](http://glew.sourceforge.net/)| GLEW provides efficient run-time mechanisms for determining which OpenGL extensions are supported on the target platform) | 2.0 | ALL |
| [GLM](https://github.com/g-truc/glm) | C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specifications | 0.9.9.8 | ALL |
| [libpng](http://www.libpng.org/pub/png/libpng.html) | Official PNG reference library | 1.6 | UNIX like (Linux,Mac,FreeBSD etc.) |

> For Windows MSYS2 install them with:
> pacman -S mingw-w64-x86_64-glfw mingw-w64-x86_64-glew mingw-w64-x86_64-glm

## Usage
Before running build application make sure that content of `gpu` folder and collada executable located in the same directory. `models` folder contains DAE files can be viewed.
Windows version shows open file dialog to open a DAE file, for UNIX version use command line like 
> `./collada <COLLADA 3D MODEL File>` 
for example 
> ./collada models/RubiksCube/rubik_cube.dae

- Drag mouse - to rotate model
- Use Left, Up, Right, Bottom, W, S and keys to move light source
- Use space to change light type