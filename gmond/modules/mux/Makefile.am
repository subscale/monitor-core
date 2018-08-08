AM_CFLAGS  = -I$(top_builddir)/include -I$(top_builddir)/lib 

if STATIC_BUILD
noinst_LTLIBRARIES    = libmodmux.la
libmodmux_la_SOURCES = mod_mux.c 
libmodmux_la_LDFLAGS = -export-all-symbols
else
pkglib_LTLIBRARIES    = modmux.la
modmux_la_SOURCES = mod_mux.c 
modmux_la_LDFLAGS = -module -avoid-version

#EXTRA_DIST = ../conf.d/mux.conf
endif

INCLUDES = @APR_INCLUDES@
