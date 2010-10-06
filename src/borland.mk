##
# Copyright (C) Dan Hipschman 05/04/05
##

# -----------------------------------------------------------------------------
PROJECT = chamber.exe

PROGRAM = chamber
VERSION = 0.8.7
CONFIGFILE = D:/0_temp/chamber/configuration

OBJFILES = bg1.obj \
           bitmap.obj \
           blackout.obj \
           blueball.obj \
           bluepad.obj \
           cyanball.obj \
           cyanpad.obj \
           fastpad.obj \
           font.obj \
           gamemode.obj \
           gfxhelp.obj \
           greenball.obj \
           greenpad.obj \
           help.obj \
           initialize.obj \
           main.obj \
           medium_font.obj \
           misc.obj \
           purpleball.obj \
           purplepad.obj \
           redball.obj \
           redpad.obj \
           scoremodes.obj \
           slowpad.obj \
           symtab.obj \
           usage.obj \
           yellowball.obj \
           yellowpad.obj
# If you have SDL_image, add it here.
LIBFILES = SDLmain.lib SDL.lib
LIBDIR = -LD:\Borland\Lib\SDL
DEFFILES =

DEBUG = NO
SUBSYS = GUI
THREADS = MORE
PRODUCT = EXE

# If you have SDL_image, add -DHAVE_SDL_IMG=1 here.
CFLAGS = -DPROGRAM=\"${PROGRAM}\" \
         -DVERSION=\"${VERSION}\" \
         -DCONFIGFILE=\"${CONFIGFILE}\"
LFLAGS =


# No need to change anything past this line.
# -----------------------------------------------------------------------------
CC = bcc32
RC = brcc32
IL = ilink32
TL = tlib

# -----------------------------------------------------------------------------
!if $(DEBUG) == YES
  CDEBUGFLAGS = -w -v
  LDEBUGFLAGS = -v
!else
  CDEBUGFLAGS = -v- -k- -O2
  LDEBUGFLAGS = -v-
!endif

!if $(SUBSYS) == GUI
  CSSFLAGS = -W
  SUBSYSFLAGS = -aa
!else
  CSSFLAGS = -WC
  SUBSYSFLAGS = -ap
!endif

!if $(THREADS) == ONE
  CMTFLAG =
  RUNTIME = cw32.lib
!else
  CMTFLAG = -WM
  RUNTIME = cw32mt.lib
!endif

!if $(PRODUCT) == DLL
  CPFLAGS = -WD
  STARTUP = c0d32.obj
!elif $(SUBSYS) == GUI
  CPFLAGS =
  STARTUP = c0w32.obj
!else
  CPFLAGS =
  STARTUP = c0x32.obj
!endif

!if $(PRODUCT) == EXE
  LACTION = $(IL) $(LIBDIR) $(LDEBUGFLAGS) $(SUBSYSFLAGS) -Tpe -c -x -Gn $(LFLAGS) $(STARTUP) $(OBJFILES),$(PROJECT),,$(LIBFILES) import32.lib $(RUNTIME),$(DEFFILES),
!elif $(PRODUCT) == DLL
  LACTION = $(IL) $(LIBDIR) $(LDEBUGFLAGS) $(SUBSYSFLAGS) -Tpd -Gi -c -x -Gn $(LFLAGS) $(STARTUP) $(OBJFILES),$(PROJECT),,$(LIBFILES) import32.lib $(RUNTIME),$(DEFFILES),
!else
  LACTION = $(TL) $(PROJECT) /u $(OBJFILES),$(PROJECT:.lib=.lst)
!endif

# -----------------------------------------------------------------------------
$(PROJECT): $(OBJFILES) $(DEFFILES)
  $(LACTION)

# -----------------------------------------------------------------------------
.c.obj:
  $(CC) $(CDEBUGFLAGS) $(CMTFLAG) $(CSSFLAGS) $(CPFLAGS) $(CFLAGS) -c {$< }

.cpp.obj:
  $(CC) $(CDEBUGFLAGS) $(CMTFLAG) $(CSSFLAGS) $(CPFLAGS) $(CFLAGS) -c {$< }

