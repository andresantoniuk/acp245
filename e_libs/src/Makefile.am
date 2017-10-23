MAINTAINERCLEANFILES = \
	Makefile.in

CLEANFILES = \
	*.gcno \
	*.gcda

AM_CPPFLAGS = -Ilinux-gnu -I../

#
# Shared libraries
#
lib_LTLIBRARIES = libe_libs.la libe_libs_buff.la libe_libs_utils.la libe_libs_conf.la libe_libs_crc.la

if E_LIBS_NET_ENABLE
lib_LTLIBRARIES += libe_libs_net.la
endif

libe_libs_c_sources = \
	e_port.c \
	linux-gnu/e_mem.c	\
	linux-gnu/e_io.c	\
	linux-gnu/e_time.c	\
	e_time_common.c \
	e_log.c
libe_libs_h_sources = \
	linux-gnu/e_port.h \
	linux-gnu/e_types.h \
	e_mem.h	\
	e_io.h \
	e_time.h \
	e_log.h

libe_libs_buff_c_sources = \
	e_buff.c

libe_libs_buff_h_sources = \
	e_buff.h

libe_libs_utils_c_sources = \
	linux-gnu/e_gps.c	\
	linux-gnu/e_gprs.c	\
	e_util.c \
	e_queue.c \
	e_units.c

libe_libs_utils_h_sources = \
	e_util.h \
	e_queue.h \
	e_units.h \
	e_gps.h \
	e_gprs.h

if E_LIBS_TIMER_ENABLE
libe_libs_utils_c_sources += linux-gnu/e_timer.c
libe_libs_utils_c_sources += e_timer.h
endif

if E_LIBS_STARTUP_ENABLE
libe_libs_utils_c_sources += linux-gnu/e_startup.c
libe_libs_utils_h_sources += linux-gnu/e_startup.h
endif

libe_libs_conf_c_sources = \
	e_conf.c
libe_libs_conf_h_sources = \
	e_conf.h

libe_libs_crc_c_sources = \
	e_crc.c
libe_libs_crc_h_sources = \
	e_crc.h


libe_libs_la_SOURCES = $(libe_libs_c_sources) $(libe_libs_h_sources)
libe_libs_la_LDFLAGS = -no-undefined -version-info $(VERSION_INFO) -release $(VERSION) $(TARGET_LD_OPTS)
libe_libs_buff_la_SOURCES = $(libe_libs_buff_c_sources) $(libe_libs_buff_h_sources)
libe_libs_buff_la_LIBADD = libe_libs.la
libe_libs_buff_la_LDFLAGS = -no-undefined -version-info $(VERSION_INFO) -release $(VERSION) $(TARGET_LD_OPTS)
libe_libs_utils_la_SOURCES = $(libe_libs_utils_c_sources) $(libe_libs_utils_h_sources)
libe_libs_utils_la_LIBADD = libe_libs.la $(event_LIBS)
libe_libs_utils_la_LDFLAGS = -no-undefined -version-info $(VERSION_INFO) -release $(VERSION) $(TARGET_LD_OPTS)
libe_libs_conf_la_SOURCES = $(libe_libs_conf_c_sources) $(libe_libs_conf_h_sources)
libe_libs_conf_la_LIBADD = libe_libs.la
libe_libs_conf_la_LDFLAGS = -no-undefined -version-info $(VERSION_INFO) -release $(VERSION) $(TARGET_LD_OPTS)
libe_libs_crc_la_SOURCES = $(libe_libs_crc_c_sources) $(libe_libs_crc_h_sources)
libe_libs_crc_la_LIBADD = libe_libs.la
libe_libs_crc_la_LDFLAGS = -no-undefined -version-info $(VERSION_INFO) -release $(VERSION) $(TARGET_LD_OPTS)

if E_LIBS_NET_ENABLE
libe_libs_net_c_sources = \
	e_client_addr.c \
	linux-gnu/e_client.c \
	e_syslog.c
libe_libs_net_h_sources = \
	e_client.h \
	e_syslog.h
libe_libs_net_la_SOURCES = $(libe_libs_net_c_sources) $(libe_libs_net_h_sources)
libe_libs_net_la_LIBADD = libe_libs.la $(event_LIBS) $(wsock32_LIBS)
libe_libs_net_la_LDFLAGS = -no-undefined -version-info $(VERSION_INFO) -release $(VERSION) $(TARGET_LD_OPTS)
endif

includedir = ${prefix}/include/e_libs
include_HEADERS= $(srcdir)/../e_libs.h $(libe_libs_h_sources) $(libe_libs_buff_h_sources) $(libe_libs_utils_h_sources) $(libe_libs_conf_h_sources) $(libe_libs_net_h_sources) $(libe_libs_crc_h_sources) $(srcdir)/e_splint_macros.h

# Targets

SPLINTOPTS = $(CPPFLAGS) -weak +posixlib
splint:
	@echo "Running splint..."
	-for x in *.c *.h linux-gnu/*.c linux-gnu/*.h; do splint $(SPLINTOPTS) $$x; done
