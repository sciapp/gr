if grep "CentOS Linux 7" /etc/*-release; then
if [ -e "/usr/local/gr" ]; then
sudo yum update -y
sudo yum install -y gcc gcc-c++ mesa-libGL-devel
else
sudo yum update -y
sudo yum install -y gcc gcc-c++ cmake patch
sudo yum install -y ghostscript-devel texlive-collection-latex texlive-dvipng cairo-devel
fi
fi

if grep "CentOS release 6" /etc/*-release; then
if [ -e "/usr/local/gr" ]; then
sudo yum update -y
sudo yum install -y gcc gcc-c++ mesa-libGL-devel
else
sudo yum update -y
sudo yum install -y gcc gcc-c++ cmake patch
sudo yum install -y ghostscript-devel texlive-collection-latex texlive-dvipng cairo-devel
fi
fi

if grep "Debian" /etc/*-release; then
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y update
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y dist-upgrade
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y install make gcc g++
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y install libx11-dev libxft-dev libxt-dev python2.7
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y install texlive-latex3 dvipng libgl1-mesa-dev qt4-dev-tools libgs-dev
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y install libgtk2.0-dev libwxgtk3.0-dev libglfw3-dev libzmq3-dev cmake
fi

if grep "Ubuntu" /etc/*-release; then
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y update
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y dist-upgrade
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y install make gcc g++
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y install libx11-dev libxft-dev libxt-dev python2.7
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y install texlive-latex3 dvipng libgl1-mesa-dev qt4-dev-tools libgs-dev
sudo apt-get -qq -o=Dpkg::Use-Pty=0 -y install libgtk2.0-dev libwxgtk3.0-dev libglfw3-dev libzmq3-dev cmake
fi

if grep "Fedora 23" /etc/*-release; then
sudo dnf -y upgrade
sudo dnf -y install gcc gcc-c++
sudo dnf -y install ghostscript-devel texlive-collection-latex texlive-dvipng zeromq3-devel glfw-devel mupdf-devel wxGTK-devel gtk2-devel
fi

make install GRDIR=/tmp/gr
