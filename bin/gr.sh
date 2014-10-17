#!/bin/sh
if [ `basename $0` = "igr" ]
then
  python=ipython
else
  python=python
fi
if [ "$1" = "-t" ]
then
  shift
  export GKS_WSTYPE="$1"
  shift
fi
if [ $# -gt 0 ]
then
  args="$*"
else
  if [ ${python} != "ipython" ]
  then
    args="-"
  else
    args=""
  fi
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
if [ -z "${GROPTS}" ]
then
  if [ ${python} != "ipython" ]
  then
    GROPTS="-dmodule://gr.matplotlib.backend_gr"
  fi
fi
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
if [ -f ${GRDIR}/etc/grrc ]
then
  . ${GRDIR}/etc/grrc
fi
exec ${PYTHONHOME}/bin/${python} ${args} ${GROPTS}
