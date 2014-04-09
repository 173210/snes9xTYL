----- Dev Stuff
All included files are licensed under the BSD license, see LICENSE in PSPSDK root for details.


danzeff is an OSK, as such it has alot of code dependant on what your current rendering system is.

I have tryed my best to seperate the rendering code out from the thinking code, so that additionally rendering targets can be added.
Currently there is only an implementation for SDL, but if you make an implementation for another renderer then please send it to me (danzelatlocalhostdotgeekdotnz) and I'll add it.

The pspctrl_emu files are for the SDL implementation.
If you are using SDL to take input (SDL_Joystick) then you can use the function provided in pspctrl_emu to convert the SDL_Joystick to a SceCtrlData for use with the OSK functions (I have also used it for psprint and it worked fine).


Code Usage:
Call danzeff_load(); to load up the keyboard (loads images from the memory stick)
Check that it managed to load the images with danzeff_isinitialized(); If it failed then either: There's not enough memory or the images aren't there.
You can now move it to where on the screen you want it to be using danzeff_moveTo(X,Y);

The 3 functions you can now use are:
int danzeff_dirty(); // <- returns true if the OSK would render differently to the last time it was rendered.
unsigned int danzeff_readInput(SceCtrlData pspctrl); // <- returns a character if one is pressed
void danzeff_render(); // <- draws the OSK


And the keyboard usage instructions from AFKIM:

----- USAGE
The danzeff keyboard is an OSK, mainly controlled by the analog stick.

Use the analog stick to select which square to chose from, and then press the button (X O [] /\) in the direction of the letter you want to input.

To switch into numbers mode Tap the L Shoulder.
To get capital Letters hold the R shoulder while in letters input.
To get a complete set of extra characters, hold down R shoulder while in numbers mode.

Other special keys:
Digital down  -> enter
Digital up    -> delete
Digital left  -> move left
Digital right -> move right
Select        -> ???
Start         -> ???