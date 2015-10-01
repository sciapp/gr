#!/bin/bash

opts="--static-extras"
if [ `uname` = 'Darwin' ]
then
  opts="${opts} --disable-gtk --disable-x11 --disable-wx"
fi

$PYTHON setup.py build_ext ${opts} install

# Add more build steps here, if they are necessary.

# See
# http://docs.continuum.io/conda/build.html
# for a list of environment variables that are set during the build process.
