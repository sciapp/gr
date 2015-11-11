#!/bin/sh
version=`git ls-remote --tags https://github.com/jheinen/gr| grep -o 'v[0-9]*\.[0-9]*\.[0-9]*' | sort -t. -k 1,1n -k 2,2n -k 3,3n | tail -1`
git clone -b ${version} https://github.com/jheinen/gr
PYTHON=/usr/local/bin/python
cd gr
${PYTHON} setup.py build_ext --static-extras --disable-gtk --disable-xt --disable-wx bdist_wheel
