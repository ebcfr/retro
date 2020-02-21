##############################################################################################

XPROJ=xretro
TPROJ=retro

INSTALL_DIR = /usr/local

TARGET  =
CC      = $(TARGET)gcc
OBJCOPY = $(TARGET)objcopy
AS      = $(TARGET)gcc -x assembler-with-cpp -c
SIZE    = $(TARGET)size
OBJDUMP = $(TARGET)objdump

# List all default C defines here
DDEFS = -g -O0 -DFONT=\"8x13\" -DGFONT=\"8x13\" -DINSTALL=\"$(INSTALL_DIR)\" -D_GNU_SOURCE -DDEBUG

# List all default directories to look for include files here
DINCDIR = . src

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS = -lm 

XLIBS = -lX11

# C source files here
SRC  = src/core.c src/edit.c src/extend.c src/funcs.c \
       src/graphics.c src/mainloop.c src/matheh.c src/spread.c

XSRC = src/sysdepx.c

TSRC = src/sysdept.c

#
# End of user defines
#############################################################################

INCDIR  = $(patsubst %,-I%,$(DINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR))
DEFS    = $(DDEFS)
LIBS    = $(DLIBS)

OBJS    = $(SRC:%.c=%.o)
XOBJS   = $(XSRC:%.c=%.o)
TOBJS   = $(TSRC:%.c=%.o)

CFLAGS  = $(INCDIR) $(MCFLAGS) $(DEBUG) $(OPT) -fomit-frame-pointer -Wall -std=c99 -D_POSIX_C_SOURCE=200809L $(DEFS)
LDFLAGS = -Wl,--gc-sections $(LIBDIR)

# Generate dependency information
CFLAGS += -MD -MP -MF .dep/$(@F).d

#
# makefile rules
#
all: $(XPROJ) $(TPROJ)

$(XPROJ): $(OBJS) $(XOBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS) $(XLIBS)

$(TPROJ): $(OBJS) $(TOBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
install:
	cp $(XPROJ) $(INSTALL_DIR)/bin/
	cp $(TPROJ) $(INSTALL_DIR)/bin/

clean:
	-rm -f $(OBJS) $(XOBJS) $(TOBJS)
	-rm -f $(XPROJ) $(TPROJ)
	-rm -f *.map
	-rm -fR .dep

# 
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

.PHONY: clean all

