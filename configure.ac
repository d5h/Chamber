#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(chamber, 0.8.7)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/gamemode.c])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
SDL_VERSION=1.2.5
AM_PATH_SDL($SDL_VERSION,
            :,
            AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))

# Check for SDL_image.  Adapted from GCL's readline check.
AC_CHECK_HEADER(SDL/SDL_image.h,
	AC_CHECK_LIB(SDL_image, IMG_Load,
		AC_DEFINE(HAVE_SDL_IMAGE)
		SDL_LIBS="$SDL_LIBS -lSDL_image"
		,, $SDL_LIBS))

# Checks for header files.
# AC_HEADER_STDC
# AC_CHECK_HEADERS([float.h limits.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
# AC_HEADER_STDBOOL
# AC_C_CONST
# AC_TYPE_SIZE_T

# Checks for library functions.
# AC_FUNC_MALLOC
# AC_FUNC_VPRINTF
# AC_CHECK_FUNCS([atexit memmove memset modf sqrt strchr strerror strrchr strtoul])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 util/Makefile])
AC_OUTPUT
