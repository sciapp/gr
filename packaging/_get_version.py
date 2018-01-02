#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import re
import os
import subprocess
import sys

REL_VERSION_SCRIPT_PATH = 'lib/Version'


def get_version(gr_source_directory_path):
    version_script_path = os.path.join(gr_source_directory_path, REL_VERSION_SCRIPT_PATH)
    if not os.path.isfile(version_script_path):
        return None
    try:
        version_script_output = subprocess.check_output(version_script_path)
    except subprocess.CalledProcessError:
        return None
    version_script_output_lines = version_script_output.split('\n')
    for line in version_script_output_lines:
        match_obj = re.match(r'^\s*__version__\s*=\s*[\'"]([^\'"]+)[\'"]\s*$', line)
        if match_obj is not None:
            version_string = match_obj.group(1)
            return version_string
    return None


def main():
    if len(sys.argv) < 2:
        print('No GR source directory path specified!', file=sys.stderr)
        sys.exit(2)
    gr_source_directory_path = sys.argv[1]
    version = get_version(gr_source_directory_path)
    if version is not None:
        print(version)
        sys.exit(0)
    else:
        print('No GR version found! (Did you specify the correct GR source directory location?)', file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
