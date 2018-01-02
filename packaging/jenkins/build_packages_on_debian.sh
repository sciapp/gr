# Install dependencies
sudo yum install -y gcc gcc-c++ mesa-libGL-devel gcc-gfortran ghostscript-devel texlive-collection-latex texlive-dvipng cmake patch python2.7 rpm-build ruby-devel rubygems
sudo gem install fpm

# Build gr and linux packages
make DISTROS=all linuxpackages

# Collect artifacts
mkdir -p artifacts/linux/centos
cp packaging/centos/gr-*-1.x86_64.rpm artifacts/linux/centos/
mkdir -p artifacts/linux/centos6
cp packaging/centos6/gr-*-1.x86_64.rpm artifacts/linux/centos6/
mkdir -p artifacts/linux/debian
cp packaging/debian/gr_*_amd64.deb artifacts/linux/debian/
mkdir -p artifacts/linux/suse
cp packaging/suse/gr-*-1.x86_64.rpm artifacts/linux/suse/
