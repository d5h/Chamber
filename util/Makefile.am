
noinst_PROGRAMS = bmp2c ttf2c txt2c dbgpgr

bmp2c_SOURCES = bmp2c.c common.c common.h save_surf.c save_surf.h
bmp2c_CFLAGS = @SDL_CFLAGS@ ${AM_CFLAGS}
bmp2c_LDADD = @SDL_LIBS@

ttf2c_SOURCES = ttf2c.c common.c common.h save_surf.c save_surf.h
ttf2c_CFLAGS = @SDL_CFLAGS@ ${AM_CFLAGS}
ttf2c_LDADD = -lSDL_ttf @SDL_LIBS@

txt2c_SOURCES = txt2c.c common.c common.h
txt2c_CFLAGS = ${AM_CFLAGS}

dbgpgr_SOURCES = dbgpgr.c
dbgpgr_CFLAGS = ${AM_CFLAGS}

pretty:
	indent *.[ch]

stats:
	@wc *.[ch]
