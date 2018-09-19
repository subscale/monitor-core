AM_CFLAGS  = -I$(top_builddir)/include -I$(top_builddir)/lib 

if STATIC_BUILD
noinst_LTLIBRARIES    = libmodcppmr.la
libmodcppmr_la_SOURCES = mod_cppmr.c 
libmodcppmr_la_LDFLAGS = -export-all-symbols
else
pkglib_LTLIBRARIES    = modcppmr.la
modcppmr_la_SOURCES = mod_cppmr.c 
modcppmr_la_LDFLAGS = -module -avoid-version

#EXTRA_DIST = ../conf.d/cppmr.conf
endif

INCLUDES = @APR_INCLUDES@
