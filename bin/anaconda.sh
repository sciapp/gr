#!/bin/sh
if [ `uname` = "Darwin" ]
then
  export DYLD_LIBRARY_PATH=/usr/local/gr/lib
else
  export LD_LIBRARY_PATH=/usr/local/gr/lib
fi
export PYTHONPATH=/usr/local/gr/lib/python
if [ -d /opt/anaconda ]
then
  ANACONDAPATH=/opt/anaconda
else
  ANACONDAPATH=/usr/local/anaconda
fi
exec ${ANACONDAPATH}/bin/python $*
