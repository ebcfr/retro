##############################################################################################

PROJ=xeuler

TARGET  =
CC      = $(TARGET)gcc
OBJCOPY = $(TARGET)objcopy
AS      = $(TARGET)gcc -x assembler-with-cpp -c
SIZE    = $(TARGET)size
OBJDUMP = $(TARGET)objdump

# List all default C defines here
DDEFS = -g -O0 -DFONT=\"8x13\" -DGFONT=\"8x13\"

# List all default directories to look for include files here
DINCDIR = . src

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS = -lm -lX11

# C source files here
SRC  = src/edit.c src/extend.c src/graphics.c src/mainloop.c \
       src/polynom.c src/express.c src/funcs.c \
       src/helpf.c src/matheh.c src/sysdepx.c

#
# End of user defines
#############################################################################

INCDIR  = $(patsubst %,-I%,$(DINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR))
DEFS    = $(DDEFS)
LIBS    = $(DLIBS)

OBJS    = $(SRC:%.c=%.o)

CFLAGS  = $(INCDIR) $(MCFLAGS) $(DEBUG) $(OPT) -fomit-frame-pointer -Wall -std=c99 -D_POSIX_C_SOURCE=200809L $(DEFS)
LDFLAGS = -Wl,-Map=$@.map,--gc-sections $(LIBDIR)

# Generate dependency information
CFLAGS += -MD -MP -MF .dep/$(@F).d

#
# makefile rules
#
all: $(PROJ)

$(PROJ): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	-rm -f $(OBJS)
	-rm -f $(PROJ)
	-rm -f *.map
	-rm -fR .dep

# 
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

.PHONY: clean all

