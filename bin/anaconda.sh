#!/bin/sh
if [ "$1" = "-o" ]
then
  shift
  export GKS_WSTYPE="$1"
  shift
fi
if [ $# -gt 0 ]
then
  args="$*"
else
  args="-"
fi
cwd="${PWD}"
cd "$(dirname "${0}")"
dir=$(readlink "$(basename "${0}")")
while [ "${dir}" ]; do
  cd "$(dirname "${dir}")"
  dir=$(readlink "$(basename "${0}")")
done
name="${PWD}/$(basename "${0}")"
cd "${cwd}"
GRDIR=`dirname ${name} | sed -e 's;/bin;;'`
if [ "${MPLBACKEND}" == "gr" ]
then
  opts="-dmodule://gr.matplotlib.backend_gr"
else
  opts=""
fi
if [ -d /opt/anaconda ]
then
  PYTHONHOME=/opt/anaconda
else
  PYTHONHOME=/usr/local/anaconda
fi
if [ `uname` = "Darwin" ]
then
    export DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}:${GRDIR}/lib
else
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${GRDIR}/lib
fi
export PYTHONPATH=${PYTHONPATH}:${GRDIR}/lib/python
exec ${PYTHONHOME}/bin/python ${args} ${opts}
