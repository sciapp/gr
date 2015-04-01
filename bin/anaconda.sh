#!/bin/sh
if [ "$1" = "-o" ]
then
  shift
  if [ "`echo $1 | grep '\.'`" != "" ]
  then
    type=`echo $1 | awk -F. '{print $NF}'`
    export GKS_FILEPATH="$1"
    export GKS_WSTYPE="$type"
  else
    export GKS_WSTYPE="$1"
  fi
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
opts=""
if [ "${MPLBACKEND}" == "gr" ]
then
  opts="-dmodule://gr.matplotlib.backend_gr"
fi
if [ -d ${HOME}/anaconda ]
then
  PYTHONHOME=${HOME}/anaconda
elif [ -d /usr/local/anaconda ]
then
  PYTHONHOME=/usr/local/anaconda
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
exec ${PYTHONHOME}/bin/python ${args} ${opts}
