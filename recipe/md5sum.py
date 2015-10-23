import md5
import sys
path = sys.argv[1]
m = md5.new()
m.update(open(path, 'rb').read())
print(m.hexdigest())
