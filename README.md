# fraccert
This project is not yet finished! If the fracfast API is conform to the thesis/documentation, version 1.0 will be released.

# Requirements
sdl2 and gmp.  
A C++ compiler with OpenMP support (e.g. g++ >9)

On Arch:  
`sudo pacman -S sdl2 gmp`

On Ubuntu:  
`sudo apt install libsdl2-dev libgmp-dev`


# Build and run instructions
Run from the command line in the root directory of the project:  
`make`  
`./fraccert`

This project consists of two parts, a fractal library (fracfast) and a viewer (fraccert).

See the thesis folder for information and documentation about this project.

Run "./fraccert --help" for information about the controls.
If Fraccert is started from a terminal, this becomes a console for Fraccert. Use "help" in this console for information about the available commands.
