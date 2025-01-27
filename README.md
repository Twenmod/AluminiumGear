This project was made for block B of Y1 Programming of IGAD/CMGT at Breda University of Applied Sciences

This project's original target sytem is the Raspberry Pi 4
It also builds on windows

Pi build may break if I update this project



# Running
1) Open "CrossPiProject.sln"
2) make sure you select the correct startup project for either Rasp4Project for PI, or Windows for windows/pc
3) Select the project architecture i.e x64 for pc and arm64 for pi 4
4) build
## libraries
Some external libraries are required for this project
Windows specific
- GLFW
- glad
Linux/Pi specific
- X11
- GLES31
Shared
- glm
- stb_image
- libdl (for miniaudio.h)


# Gameplay
your goal is to get to the exit of the facility,
find items around the level to help you.
Do not get spotted by enemies or take them out before they can.

view the cheat sheet image next to this file to see where all items in the level are.

## Controls
mouse for menus
[ESCAPE] for pause menu
[WASD] to move
[SHIFT] to run
arrows for inventory navigation
[SPACE] use item
[LEFT_MOUSE] Weapon primary
[RIGHT_MOUSE] Weapon secondary (toggle)

# settings/configurables
Most easy to configure constants like render resolution or reflections can be configured inside the "Common/SharedItems/Common.h" file
Some in game debug options like physics debug drawing are available via the debug menu accessable in the main menu or pause menu in the bottom left. 
