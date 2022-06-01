# Shadertoy OpenGL Wrapper
Copy the code from shadertoy in a file and run it native.

## Not supported features
Textures, sounds and anything that you load using channels.

## Required libs
Only uses GLFW so if you got it installed you are good to go.

## Build in linux

You need to instal glfw3 on your computer.

if you are in ubuntu, run the following command.

```
sudo apt-get install libglfw3
sudo apt-get install libglfw3-dev
```
If you are not in ubuntu, you need to check what methods are available for your distribution.

Then, get into the project folder and run

```
make run_main
```

## Build on windows

I'm not a windows guy, but I did my best to make possible to compile in windows. But problems may hapen.

You have to create a folder structure similar like this.

c:\shadertoy
```
->libs
-->glew
--->bin
--->include
-->glfw
--->include
--->lib-bc-2020 (can change depending on the version of visual studio you have)
->glfw-load-opengl-funcitons
```

1. First, create a folder for the project, can be c:\shadertoy or whatever you like.

2. Download the windows precompiled binaries form https://www.glfw.org/download.html, 64-bit version is recommended.

3. Decompress the file in the folder you created in 1.

4. You also need glew (this is not needed in linux (:shrug:)), downlad it from http://glew.sourceforge.net/

5. Decompress it in the folder you created in 1.

6. You need to open "x64 Native Tools Command Prompt for Visual Studio", should be installed with your visual studio, if not, you have to install it.

7. In the command prompt, type the following:

```
cd c:\shadertoy
git clone https://github.com/WEREMSOFT/glfw-load-opengl-funcitons.git
cd glfw-load-opengl-funcitons
build
```
That should be it. If not, the project is only a fistfull of files. You should be able to build it using premake or creating an empty project in visual studio and following the instructions on the glfw site.

## How to use it

Got to a shadertoy contribution that does not use channels, like https://www.shadertoy.com/view/fstyD4

![alt text](docsImages/1.png)

Copy the code and paste into the file assets/shader.fs.

Enjoy

![alt text](docsImages/2.png)