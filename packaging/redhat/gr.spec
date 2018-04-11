%define THIRDPARTY build/3rdparty
%define THIRDPARTY_SRC %{THIRDPARTY}/src
%define THIRDPARTY_INC %{THIRDPARTY}/include
%define THIRDPARTY_LIB %{THIRDPARTY}/lib

%if 0%{?__jcns}
%define fixedversion %{version}
%else
# use fixedversion for builds on build.opensuse.org - needed for deb builds.
%define fixedversion fixed
%define compression gz
%endif

%define qmake_qt4 qmake-qt4
%define grdir %{_prefix}/gr

Name:				gr
Summary:			GR, a universal framework for visualization applications
Version:			0.28.1
Release:			3%{?dist}
License:			MIT
Group:				Development/Libraries
Source:				gr-%{fixedversion}.tar%{?compression:.%{compression}}
Source1:			https://gr-framework.org/downloads/3rdparty/mupdf-1.6-source.tar.gz
Source2:			https://gr-framework.org/downloads/3rdparty/libogg-1.3.2.tar.gz
Source3:			https://gr-framework.org/downloads/3rdparty/libtheora-1.1.1.tar.gz
Source4:			https://gr-framework.org/downloads/3rdparty/libvpx-1.4.0.tar.bz2
Source5:			https://gr-framework.org/downloads/3rdparty/ffmpeg-2.1.4.tar.gz
Source6:			https://gr-framework.org/downloads/3rdparty/glfw-3.1.1.tar.gz
Source7:			https://gr-framework.org/downloads/3rdparty/zeromq-4.0.4.tar.gz
Source8:			https://gr-framework.org/downloads/3rdparty/openjpeg-2.0.0.tar.gz
Source9:			https://gr-framework.org/downloads/3rdparty/cmake-2.8.12.2.tar.gz
Source10:			https://gr-framework.org/downloads/3rdparty/cairo-1.14.6.tar.xz
Source11:			https://gr-framework.org/downloads/3rdparty/pixman-0.34.0.tar.gz
# for vcversioner
BuildRequires:		git
BuildRequires:		gcc-c++
BuildRequires:		libX11-devel
BuildRequires:		libXt-devel
BuildRequires:		libXft-devel
BuildRequires:		gtk2-devel
%if 0%{?__jcns}
%define debug_package %{nil}
%define qmake_qt4 %{_prefix}/qt4/bin/qmake
%define qmake_qt5 %{_prefix}/qt5/bin/qmake
BuildRequires: qt4-local
BuildRequires: qt5-local
BuildRequires: gcc-local
BuildRequires: cmake-local
%else
BuildRequires: qt-devel
%endif

%if 0%{?suse_version}
%define qmake_qt4 qmake
%define qmake_qt5 qmake-qt5
BuildRequires:		Mesa-libGL-devel
BuildRequires:		libqt4-devel
BuildRequires:		libqt5-qtbase-devel
%else
BuildRequires:		mesa-libGL-devel
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

# RHEL 6 and Scientific Linux 6 have too old cmake version (build internal)
%if 0%{?rhel_version} == 600 || 0%{?scientificlinux_version} == 600
%else
BuildRequires:		cmake
%endif

# wxWidgets BuildRequires
%if 0%{?fedora_version}
BuildRequires:		wxGTK-devel
%endif
%if 0%{?suse_version}
BuildRequires:		wxWidgets-devel
%endif

# Qt5 BuildRequires for Fedora
%if 0%{?fedora_version} >= 23
%define qmake_qt5 qmake-qt5
BuildRequires:		qt5-qtbase-devel
%endif


%description
GR, a universal framework for visualization applications

%prep
%setup -n gr-%{fixedversion}
mkdir -p %{THIRDPARTY_SRC}
tar -C %{THIRDPARTY_SRC} -xf %{SOURCE1}
tar -C %{THIRDPARTY_SRC} -xf %{SOURCE2}
tar -C %{THIRDPARTY_SRC} -xf %{SOURCE3}
tar -C %{THIRDPARTY_SRC} -xf %{SOURCE4}
tar -C %{THIRDPARTY_SRC} -xf %{SOURCE5}
tar -C %{THIRDPARTY_SRC} -xf %{SOURCE6}
tar -C %{THIRDPARTY_SRC} -xf %{SOURCE7}
tar -C %{THIRDPARTY_SRC} -xf %{SOURCE8}
tar -C %{THIRDPARTY_SRC} -xf %{SOURCE9}
tar -C %{THIRDPARTY_SRC} -xf %{SOURCE10}
tar -C %{THIRDPARTY_SRC} -xf %{SOURCE11}

%build
make -C 3rdparty GRDIR=%{grdir} DIR=`pwd`/%{THIRDPARTY}
make -C 3rdparty extras GRDIR=%{grdir} DIR=`pwd`/%{THIRDPARTY}
%if 0%{?__jcns}
export CC=/usr/local/gcc/bin/gcc49
export CXX=/usr/local/gcc/bin/g++49
%endif
make GRDIR=%{grdir} \
     EXTRA_CFLAGS=-I`pwd`/%{THIRDPARTY_INC} \
     EXTRA_CXXFLAGS=-I`pwd`/%{THIRDPARTY_INC} \
     EXTRA_LDFLAGS=-L`pwd`/%{THIRDPARTY_LIB} \
     THIRDPARTYDIR=`pwd`/%{THIRDPARTY} \
     %{?qmake_qt4:QT4_QMAKE=%{qmake_qt4}} \
     %{?qmake_qt5:QT5_QMAKE=%{qmake_qt5}}

%install
%{__install} -m 755 -d $RPM_BUILD_ROOT%{grdir}
make install GRDIR=%{grdir} DESTDIR=${RPM_BUILD_ROOT}

%clean
make realclean

%files
%defattr(-,root,root)
%{grdir}
