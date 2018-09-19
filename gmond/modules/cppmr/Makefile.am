AM_CFLAGS  = -I$(top_builddir)/include -I$(top_builddir)/lib 

if STATIC_BUILD
noinst_LTLIBRARIES    = libmodcppmr.la
libmodmux_la_SOURCES = mod_cppmr.c 
libmodmux_la_LDFLAGS = -export-all-symbols
else
pkglib_LTLIBRARIES    = modcppmr.la
modmux_la_SOURCES = mod_cppmr.c 
modmux_la_LDFLAGS = -module -avoid-version

#EXTRA_DIST = ../conf.d/cppmr.conf
endif

INCLUDES = @APR_INCLUDES@
