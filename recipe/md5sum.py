from __future__ import print_function

import hashlib
import sys

path = sys.argv[1]
m = hashlib.md5()
m.update(open(path, 'rb').read())

print(m.hexdigest())
