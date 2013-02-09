TARGET = clock
OBJS = main.o 

INCDIR =
CFLAGS = -DFPM_MIPS -O2 -G0 -Wall -fno-pic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = $(TARGET).exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

INCDIR = ./include
LIBDIR = ./lib
LDFLAGS = -mno-crt0 -nostartfiles
LIBS = -lpspsdk
LIBS = -lpng -lm -lpspaudio -lpspgu -lpsppower -ljpeg -lpsputility -lstdc++ -lpspwlan -lpspgum_vfpu  -lpspvfpu -lpspgu  -lpspusb -lpspusbstor -lpspnet_adhoc -lpspnet_adhocctl -lpspnet_adhocmatching
LIBS += -lpspaudiocodec -lpspaudio -lpspkernel -lm -lpsplibc -lpspsystemctrl_kernel
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all:
	psp-build-exports -s $(PRX_EXPORTS)
	@cp $(TARGET).prx ../../
	@cp $(TARGET).S ../
	@rm -f $(TARGET).prx
	@rm -f $(TARGET).S
