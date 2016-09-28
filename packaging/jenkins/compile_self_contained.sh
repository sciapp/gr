if grep "CentOS Linux 7" /etc/*-release; then
if [ -e "/usr/local/gr" ]; then
sudo yum install -y gcc gcc-c++ mesa-libGL-devel
else
sudo yum groupinstall -y "Development Tools"
sudo yum install -y gcc gcc-c++ mesa-libGL-devel libXt-devel libX11-devel libXrender-devel
fi
fi

if grep "CentOS release 6" /etc/*-release; then
if [ -e "/usr/local/gr" ]; then
sudo yum install -y gcc gcc-c++ mesa-libGL-devel
else
sudo yum groupinstall -y "Development Tools"
sudo yum groupinstall -y "X Window System"
sudo yum install -y gcc gcc-c++ mesa-libGL-devel libXt-devel libX11-devel libXrender-devel
fi
fi

if grep "Debian" /etc/*-release; then
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y update
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y install make gcc g++
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y install libgl1-mesa-dev libglu1-mesa-dev xorg-dev
fi

if grep "Ubuntu" /etc/*-release; then
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y update
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y install make gcc g++
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y install libgl1-mesa-dev libglu1-mesa-dev xorg-dev
fi

if grep "Fedora 23" /etc/*-release; then
sudo dnf -y install gcc gcc-c++ mesa-libGL-devel
fi

make self GRDIR=/tmp/gr

