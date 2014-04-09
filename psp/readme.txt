 ----------------
/snes9xTYL 0.4.2/
---------------

Since v0.3, there's 2 versions :
- standard, running in user mode, allowing firmware 2.0+ psp.
- me, running in kernel mode and using the Media Engine for sound mixing, only working on firmware 1.0 & 1.5 psp.

+++++++++++++++++++++++++++++
[INTRO]
--------------------------->

snes9xTYL is based on :

Little John for PalmOS - SNES module
--> http://yoyofr92.free.fr

which is based on Snes9x 1.39
--> http://www.snes9x.com

unofficial PSPSDK from psp2dev community
--> http://www.ps2dev.org
great work!

speedhacks database from SnesAdvance
--> http://www.snesadvance.org
you rock!

Nice icon/PSP menu background by various artists and Pochi
--> http://pochistyle.pspwire.net/

Danzeff OSK from Danzel
--> danzelatlocalhostdotgeekdotnz

Personnal thanks to : smiths, chp, bifuteki.

+++++++++++++++++++++++++++++
[FEATURES]
--------------------------->
- 5 Rendering mode : 
	-Mode0 : Optimized Snes9x, 
	-Mode1 : Original Snes9x 
	-Mode2 : Hardware accelerated (using PSP's GU)
	-Mode3 : Adaptive rendering Mode 2 + Mode 1 (default)
	-Mode4 : Adaptive rendering Mode 2 + Mode 0
Yes you read well, this version support nearly all the graphics effects of the Snes accelerated with PSP hardware.
Except : offset mode & mode 7.Priority and blending are fully emulated.
For the moment a few graphic glitches remain, we did our best for the moment in our knowledge. Please understand.
-Zipped ROM support.
-IPS patch file (have to be the same name as game with .IPS extension : SOE.ZIP => SOE.IPS)
-Compressed Savestate with small screenshot.
-Auto save of SRAM so you should never lose them ;-).
-Autoskip.
-Gamma correction.
-VSync support.
-SDD1 encrypted roms (Star Ocean, Street Fight Alpha 2,...)
-SA1 (slow), SuperFX (slow), C4, DSP1 support.
-Multiple sound frequencies : 11Khz, 22Khz, 32Khz, 44Khz.
-Multiple stretching mode with or without smoothing
-Detailed Battery informations.
-222,266 & 333Mhz PSP frequency.
-Snapshot based Icon per game. Take a snapshot while ingame (using GUI) & then you'll have it in file browser.
-Import savestate from uosnes9x & Zsnes.
-Per game settings.
-Multiple hacks to speed up emulation.
-Inline multilanguage help (english & partial french, japanese).
-Memstick Secured handling : do not write anything if battery is low + visual alarm.
-Configurable inputs
-Screen calibration
-Full sound emulation on Media Engine for PSP Firmware 1.0 & 1.5
-2 players adhoc netplay.
-SPC playback.
-Cheats code.
-Minimalist file viewer with search function (useful for FAQ).
-Customizable background.
-Export savestate to snes9x.
-Funny intro.
-...
 

+++++++++++++++++++++++++++++
[INSTALL]
--------------------------->

copy the 2 directories corresponding to your PSP version (1.0 / 1.5) in the
"/PSP/GAME/" folder

for 2.00+ owner, check fanjita website for detailed informations. (http://www.fanjita.org)

You can put the rom files anywhere you want, a "SAVES/" subdirectory will be created in you install
directory. 
All savestate files, sram file & jpg snapshot will be written in the "SAVES" subdir.

FAQ and TXT files should go in the FAQS subdirectory.
Settings are saved in the PROFILES subdirectory.
DATA subdirectory is composed of :
 - music.zip : put your own .spc file in this archive to have them played in GUI.
 - logo.zip : put your own .jpg file in this archive to have your own backgrounds in GUI.
 - snesadvance.dat : the speedhack, can be customized if you known what you're doing.
 - msg.ini : start of an internationalization, not used yet.
 - graphics : png files for danzeff OSK.
           
+++++++++++++++++++++++++++++
[PLAY]
--------------------------->

Default controls :

|----------------------------|
|PSP            | SNES       |
|----------------------------|
|pad            | pad        |
|analog         | pad        |
|L + R          | GUI        |
|CROSS          | A          |
|CIRCLE         | B          |
|SQUARE         | X          |
|TRIANGLE       | Y          |
|START          | START      |
|SELECT         | SELECT     |
|L              | L          |
|R              | R          |
|----------------------------|

+++++++++++++++++++++++++++++
[HISTORY]
see changes.txt for details
-
0.4.2  - 2006/05/15
-
0.4.1  - 2006/05/08
-
0.4  - 2006/05/07
-
0.3  - 2006/02/06
-
0.2c - 2005/08/23
-
0.2b - 2005/08/23
-
0.2 - 2005/08/23
-
0.1 - 2005/08/02 (first release)
-

+++++++++++++++++++++++++++++
[EXIT]
--------------------------->

Have fun!

---------------------->
http://yoyofr92.free.fr
------>

yoyofr