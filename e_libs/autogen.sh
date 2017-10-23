#!/bin/sh
# based on autogen.sh from gpsd project

CONF_OPTS="--prefix=$HOME/tmp_install/ $@"

autoreconf --force --install && ./configure $CONF_OPTS
