
@echo off
mkdir bin
pushd src
cl -Zi ^
   /I C:\projects\cpp\libs\glfw\include ^
    main.cpp ^
    C:\projects\cpp\libs\glfw\lib-vc2022\glfw3dll.lib ^
    C:\projects\cpp\libs\glfw\lib-vc2022\glfw3_mt.lib ^
    C:\projects\cpp\libs\glew\lib\Release\x64\glew32.lib ^
    opengl32.lib ^
   -o ../bin/main.exe
popd

xcopy C:\projects\cpp\libs\glfw\lib-vc2022\glfw3.dll bin /Y
xcopy assets bin\assets /E /H /C /I /Y
