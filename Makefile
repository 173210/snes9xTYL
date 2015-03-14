TARGET = snes9x
INCDIR = . psp psp/openspc++ psp/adhoc zlib danzeff

# Comment these out. Don't just set to 0.
ME_ACTIVE = 1
HOME_HOOK_ON = 1

CFLAGS=  -O3 -frename-registers -ffast-math -fomit-frame-pointer -G0 -Wall -g -DFW3X -DPSP -D__PSP__ -DRELEASE

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c

LIBDIR =
LDFLAGS =

LIBS = -lpng -lm -lpspaudio -lpspgu -lpsppower -ljpeg -lpsputility -lstdc++ -lpspwlan -lpspgum_vfpu  -lpspvfpu -lpspgu  -lpspusb -lpspusbstor -lpspnet_adhoc -lpspnet_adhocctl -lpspnet_adhocmatching

PSP_EBOOT_ICON= "resources/ICON0.png"
PSP_EBOOT_PIC1= "resources/PIC1.png"
PSP_EBOOT_SND0 = "resources/SND0.AT3"

BUILD_PRX = 1
PSP_FW_VERSION = 505
PSP_LARGE_MEMORY = 1

PSP_EBOOT_TITLE = SNES9X Euphoria R5
EXTRA_TARGETS = EBOOT.PBP

# Object files
PSP_OBJECTS=	apu.o apudebug.o c4.o c4emu.o cheats.o cheats2.o clip.o cpu.o cpuexec_opti.o cpuops.o data.o \
				debug.o dma.o dsp1.o fxdbg.o fxemu.o fxinst.o gfx_mode7.o gfx.o gfx_org.o gfx_psp.o globals.o loadzip.o memmap.o \
				ppu.o sa1.o sa1cpu.o sdd1.o sdd1emu.o snapshot.o snes9x.o spc700.o srtc.o soundux.o\
				tile.o tile_org.o tile_sw.o tile_psp.o unzip.o cheats.o cheats2.o

PLATFORMOBJ = 	psp/psp.o psp/vfpu.o psp/psp_ui.o psp/config.o psp/psplib.o psp/pg.o psp/psp_msg.o \
				psp/filer.o psp/menu.o psp/imageio.o psp/blitter.o psp/decrypt.o psp/img_jpeg.o psp/ram.o psp/psp_net.o \
				psp/menu_fx.o psp/mem64.o

MEOBJ =	psp/MediaEngine.o psp/mediaengineprx/me.o

HOMEHOOKOBJ = psp/homehook.o

SPCOBJ = psp/openspc++/dsp.o psp/openspc++/spc.o psp/openspc++/ospcmain.o psp/openspc++/os9xZ_openspc.o

NETOBJ = psp/adhoc/pspadhoc.o psp/adhoc/selectorMenu.o

ZLIBOBJ = zlib/adler32.o zlib/compress.o zlib/crc32.o zlib/gzio.o\
		zlib/uncompr.o zlib/deflate.o zlib/trees.o zlib/zutil.o\
		zlib/inflate.o zlib/infblock.o zlib/inftrees.o zlib/infcodes.o\
		zlib/infutil.o zlib/inffast.o

DANZOBJ = danzeff/danzeff.o

OBJS= $(PLATFORMOBJ) $(PSP_OBJECTS) $(ZLIBOBJ) $(SPCOBJ) $(NETOBJ) $(DANZOBJ)

ifdef ME_ACTIVE
INCDIR+= psp/mediaengineprx
OBJS+= $(MEOBJ)
CFLAGS+= -DME_SOUND
endif

ifdef HOME_HOOK_ON
INCDIR+= psp/homehookprx
MEOBJ+= $(HOMEHOOKOBJ)
CFLAGS+= -DHOME_HOOK
endif

CFLAGS+= -DNOKERNEL

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

# Dependecies
tile_psp.o: tile_psp.h
gfx_psp.o: tile_psp.h
clean_snes:
	rm $(PSP_OBJECTS) $(PLATFORMOBJ)

