# Fedora 35 introduced extra rpath checks which don't allow
# `/usr/lib/gr` as a valid rpath -> disable the check
# See <https://fedoraproject.org/wiki/Changes/Broken_RPATH_will_fail_rpmbuild> for details
%if 0%{?fedora_version} >= 35
%global __brp_check_rpaths %{nil}
%endif

%define THIRDPARTY 3rdparty
%define THIRDPARTY_SRC %{THIRDPARTY}/build/src

%if 0%{?mlz}
%define fixedversion %{version}
%else
# use fixedversion for builds on build.opensuse.org - needed for deb builds.
%define fixedversion fixed
%define compression gz
%endif

%define grdir %{_prefix}/gr

Name:				gr
Summary:			GR, a universal framework for visualization applications
Version:			0.63.0
Release:			3%{?dist}
License:			MIT
Group:				Development/Libraries
Source:				gr-%{fixedversion}.tar%{?compression:.%{compression}}
Source1:			https://gr-framework.org/downloads/3rdparty/libogg-1.3.2.tar.gz
Source2:			https://gr-framework.org/downloads/3rdparty/libtheora-1.1.1.tar.gz
Source3:			https://gr-framework.org/downloads/3rdparty/libvpx-1.4.0.tar.bz2
Source4:			https://gr-framework.org/downloads/3rdparty/ffmpeg-5.1.4.tar.gz
Source5:			https://gr-framework.org/downloads/3rdparty/glfw-3.3.3.tar.gz
Source6:			https://gr-framework.org/downloads/3rdparty/zeromq-4.3.4.tar.gz
Source7:			https://gr-framework.org/downloads/3rdparty/cmake-3.23.0-linux-x86_64.tar.gz
Source8:			https://gr-framework.org/downloads/3rdparty/cairo-1.16.0.tar.xz
Source9:			https://gr-framework.org/downloads/3rdparty/pixman-0.42.2.tar.gz
Source10:			https://gr-framework.org/downloads/3rdparty/tiff-4.7.0.tar.gz
Source11:			https://gr-framework.org/downloads/3rdparty/libopenh264-2.0.0.tar.gz
Source12:			https://gr-framework.org/downloads/3rdparty/xerces-c-3.2.4.tar.gz
Source13:			https://gr-framework.org/downloads/3rdparty/icu4c-74.2.tar.gz
BuildRequires:		git
BuildRequires:		gcc-c++
BuildRequires:		libX11-devel
BuildRequires:		libXt-devel
BuildRequires:		libXft-devel
BuildRequires:		gtk2-devel
%if 0%{?rhel} < 8
BuildRequires: qt-devel
%endif

%if 0%{?suse_version}
BuildRequires:		xorg-x11-devel
BuildRequires:		Mesa-libGL-devel
BuildRequires:		libqt4-devel
BuildRequires:		libqt5-qtbase-devel
%else
%if 0%{?rhel} < 10
BuildRequires:		xorg-x11-server-devel
BuildRequires:		mesa-libGL-devel
%endif
%endif

%if 0%{?rhel_version}
BuildRequires:		ghostscript
%else
# RHEL: unresolvable package(s)
BuildRequires:		ghostscript-devel
%endif

# Scientific Linux 6: have choice for libjpeg-devel needed by qt-devel: libjpeg-devel libjpeg-turbo-devel
%if 0%{?scientificlinux_version} == 600
BuildRequires:		libjpeg-turbo-devel
%endif

# RHEL, CentOS and Scientific Linux 7 have too old cmake version (use prebuild)
%if 0%{?rhel} == 7
%else
BuildRequires:		cmake
%endif

# wxWidgets BuildRequires
%if 0%{?fedora_version}
%if 0%{?fedora_version} >= 30 && 0%{?fedora_version} <= 38
BuildRequires:		wxGTK3-devel
%else
BuildRequires:		wxGTK-devel
%endif
%endif
%if 0%{?suse_version}
BuildRequires:		wxWidgets-devel
%endif

# Qt5 BuildRequires
%if 0%{?fedora_version} >= 23 || 0%{?rhel} >= 8
BuildRequires:		qt5-qtbase-devel
%endif

# Qt6 BuildRequires
%if 0%{?fedora_version} >= 34 || 0%{?rhel} >= 9
BuildRequires:		qt6-qtbase-devel
%endif

%description
GR, a universal framework for visualization applications

%prep
%setup -n gr-%{fixedversion}
mkdir -p %{THIRDPARTY_SRC}
%{__cp} %{SOURCE1} %{THIRDPARTY_SRC}
%{__cp} %{SOURCE2} %{THIRDPARTY_SRC}
%{__cp} %{SOURCE3} %{THIRDPARTY_SRC}
%{__cp} %{SOURCE4} %{THIRDPARTY_SRC}
%{__cp} %{SOURCE5} %{THIRDPARTY_SRC}
%{__cp} %{SOURCE6} %{THIRDPARTY_SRC}
%{__cp} %{SOURCE7} %{THIRDPARTY_SRC}
%{__cp} %{SOURCE8} %{THIRDPARTY_SRC}
%{__cp} %{SOURCE9} %{THIRDPARTY_SRC}
%{__cp} %{SOURCE10} %{THIRDPARTY_SRC}
%{__cp} %{SOURCE11} %{THIRDPARTY_SRC}
%{__cp} %{SOURCE12} %{THIRDPARTY_SRC}
%{__cp} %{SOURCE13} %{THIRDPARTY_SRC}
%{__tar} -C %{THIRDPARTY} -xf %{SOURCE7}

%build
# Honor flags from Linux distribution
export CFLAGS="%{optflags}"
export CXXFLAGS="%{optflags}"
export LDFLAGS="%{build_ldflags}"
# RHEL, CentOS and Scientific Linux 7 have too old cmake version (use prebuild)
%if 0%{?rhel} == 7
export PATH=`pwd`/%{THIRDPARTY}/cmake-3.23.0-linux-x86_64/bin:$PATH
%endif
make -C 3rdparty default extras
cmake \
  -DCMAKE_INSTALL_PREFIX=%{grdir} \
  -DCMAKE_BUILD_TYPE=Release \
  -DGR_USE_BUNDLED_LIBRARIES=ON \
  -S . \
  -B build
cmake --build build

%install
# RHEL, CentOS and Scientific Linux 7 have too old cmake version (use prebuild)
%if 0%{?rhel} == 7
export PATH=`pwd`/%{THIRDPARTY}/cmake-3.23.0-linux-x86_64/bin:$PATH
%endif
%{__install} -m 755 -d $RPM_BUILD_ROOT%{grdir}
DESTDIR=${RPM_BUILD_ROOT} cmake --install build

%clean
make realclean

%files
%defattr(-,root,root)
%{grdir}
