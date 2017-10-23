MAINTAINERCLEANFILES = \
	Makefile.in

CLEANFILES = \
	*.gcno \
	*.gcda

AM_CPPFLAGS = $(e_libs_CFLAGS) $(e_libs_buff_CFLAGS) $(e_libs_utils_CFLAGS) -I$(host_os)

#
# Shared libraries
#
lib_LTLIBRARIES = libacp245.la

libacp245_c_sources = \
	acp_init.c \
	acp_license.c \
	acp_ie.c \
	acp_el.c \
	sha256.c \
	hmac_sha256.c \
	acp_key.c \
	acp_msg.c

libacp245_h_sources = \
	acp245.h \
	acp_types.h \
	acp_init.h \
	acp_license.h \
	acp_err.h \
	acp_ie.h \
	acp_el.h \
	sha256.h \
	hmac_sha256.h \
	acp_key.h \
	acp_msg.h

libacp245_la_SOURCES = $(libacp245_c_sources) $(libacp245_h_sources)
libacp245_la_LIBADD = $(e_libs_LIBS) $(e_libs_buff_CFLAGS) $(e_libs_utils_CFLAGS)
libacp245_la_LDFLAGS = -no-undefined -version-info $(VERSION_INFO) -release $(VERSION) $(TARGET_LD_OPTS)

includedir = ${prefix}/include/acp245
include_HEADERS= $(libacp245_h_sources)

# Targets

SPLINTOPTS = -I../ -standard -compdef -compdestroy -nullstate -predboolint -type -retvalother -mustfreeonly +impouts -predboolothers -boolops
splint:
	@echo "Running splint on libraries..."
	-splint $(SPLINTOPTS) -exportlocal $(libacp245_c_sources) $(AM_CPPFLAGS)
