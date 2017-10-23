#!/bin/sh

CONF_OPTS="$@ --prefix=$HOME/tmp_install/"

# Default pkgconfigdir used by Edantech libraries.
export PKG_CONFIG_PATH=$PKG_CONF_PATH:$HOME/tmp_install/lib/pkgconfig/

autoreconf --force --install && ./configure $CONF_OPTS
