#!/bin/sh
name=`readlink -f ${0}`
GRDIR=`dirname "${name}" | sed -e 's;/bin;;'`
if [ -f /usr/local/bin/python ]
then
  PYTHONHOME=/usr/local
else
  PYTHONHOME=/usr
fi
if [ `uname` = "Darwin" ]
then
    export DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${GRDIR}/lib
else
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${GRDIR}/lib
fi
export PYTHONPATH=${PYTHONPATH}:${GRDIR}/lib/python
exec ${PYTHONHOME}/bin/python $*
