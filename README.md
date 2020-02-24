# 3dRaymarcherFramework

This contains ready to use windows builds as well as the raw code and project files for an individual build.

To use the existing build, copy the Release folder to your computer and add the glew32.dll to it. You'll find it here: https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip/download

You can edit the fragment shader text file with your favourite editor and simply save, the changes will then apply to the image shown.

NOTE: Your monitor resolution MUST BE 1920x1080 for the ready builds to work! If it is not, you need to change the screenheight variable in Window.cpp and build it yourself, good luck!

Building yourself: you need the includes and libraries for glfw3 and glew, VisualStudio 2013 or comparable and all files of the repo.
