
@echo off
mkdir bin
pushd src
cl -Zi ^
   /std:c++17 ^
   /I ..\..\libs\glfw\include ^
    main.cpp ^
    ..\..\libs\glfw\lib-vc2022\glfw3dll.lib ^
    ..\..\libs\glfw\lib-vc2022\glfw3_mt.lib ^
    ..\..\libs\glew\lib\Release\x64\glew32.lib ^
    opengl32.lib ^
   -o ../bin/main.exe
popd

xcopy ..\libs\glfw\lib-vc2022\glfw3.dll bin /Y
xcopy assets bin\assets /E /H /C /I /Y

pushd bin
main.exe
popd
