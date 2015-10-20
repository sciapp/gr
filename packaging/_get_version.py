#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import re
import os
import os.path
import sys


if 'GRDIR' in os.environ:
    GRDIR = os.environ['GRDIR']
else:
    GRDIR = '../tmp/gr'
REL_GR_PACKAGE_PATH = 'lib/python/gr'
VERSION_MODULE = '_version'


def get_version():
    sys.path.append(os.path.join(GRDIR, REL_GR_PACKAGE_PATH))
    version_module = __import__(VERSION_MODULE, globals(), locals(), [], -1)
    version_string = version_module.__version__
    sys.path.pop()
    match_obj = re.match('(\d+\.){2}\d+', version_string)
    if match_obj is not None:
        version_tuple_string = match_obj.group()
    else:
        version_tuple_string = ''
    return version_tuple_string


def main():
    print(get_version())


if __name__ == '__main__':
    main()
