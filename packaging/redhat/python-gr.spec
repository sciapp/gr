%{!?__python: %global __python %{_bindir}/python}
%{!?python_sitearch: %global python_sitearch %(%{__python} -c "from distutils.sysconfig import get_python_lib; print get_python_lib(True)")}
%{!?python_version: %global python_version %(%{__python} -c "import sys; sys.stdout.write(sys.version[:3])")}
%global python_platform %(%{__python} -c "from distutils.util import get_platform ; print get_platform()")
%define THIRDPARTY build/3rdparty.%{python_platform}-%{python_version}
%define THIRDPARTY_SRC %{THIRDPARTY}/src
%define THIRDPARTY_LIB %{THIRDPARTY}/lib

%if 0%{?__jcns}
%define fixedversion %{version}
Name:          python-gr-local
%else
# use fixedversion for builds on build.opensuse.org - needed for deb builds.
%define fixedversion fixed
Name:          python-gr
%endif

%define qmake_qt4 qmake-qt4
%define grdir %{_prefix}/gr

Summary:			GR, a universal framework for visualization applications
Version:			0.17.3.post55
Release:			2%{?dist}
License:			MIT
Group:				Development/Libraries
Source:				gr-%{fixedversion}.tar
Source1:			http://mupdf.com/downloads/archive/mupdf-1.6-source.tar.gz
Source2:			http://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.gz
Source3:			http://downloads.xiph.org/releases/theora/libtheora-1.1.1.tar.gz
Source4:			http://downloads.webmproject.org/releases/webm/libvpx-1.4.0.tar.bz2
Source5:			https://ffmpeg.org/releases/ffmpeg-2.1.4.tar.gz
Source6:			https://github.com/glfw/glfw/archive/3.1.1.tar.gz
Source7:			http://download.zeromq.org/zeromq-4.0.4.tar.gz
Source8:			https://pgi-jcns.fz-juelich.de/files/openjpeg-2.0.0.tar.gz
Source9:			https://cmake.org/files/v2.8/cmake-2.8.12.2.tar.gz
Source10:			https://cairographics.org/releases/cairo-1.14.6.tar.xz
Source11:			https://cairographics.org/releases/pixman-0.34.0.tar.gz
# for vcversioner
BuildRequires:		git
BuildRequires:		gcc-c++
BuildRequires:		libX11-devel
BuildRequires:		libXt-devel
BuildRequires:		libXft-devel
BuildRequires:		gtk2-devel
%if 0%{?__jcns}
%define qmake_qt4 %{_prefix}/qt4/bin/qmake
%define qmake_qt5 %{_prefix}/qt5/bin/qmake
BuildRequires: python-local
BuildRequires: python-setuptools-local
BuildRequires: qt4-local
BuildRequires: qt5-local
Requires:      python-local
Requires:      numpy-local
%else
BuildRequires: python-devel
BuildRequires: python-setuptools
BuildRequires: qt-devel
Requires:      python
Requires:      numpy
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


%package -n gr
Summary:			GR, a universal framework for visualization applications
%description -n gr
GR, a universal framework for visualization applications


%build
%{__python} setup.py build_ext --static-extras --qmake-qt4=%{qmake_qt4} \
                               %{?qmake_qt5:--qmake-qt5=%{-qmake_qt5}}
%{?qmake_qt4:export QT4_QMAKE=%{qmake_qt4}}
%{?qmake_qt5:export QT5_QMAKE=%{qmake_qt5}}
make -C 3rdparty GRDIR=%{grdir} DIR=%{THIRDPARTY}
make -C 3rdparty extras GRDIR=%{grdir} DIR=%{THIRDPARTY}
make GRDIR=%{grdir}

%install
%{__python} setup.py build_ext --static-extras --qmake-qt4=%{qmake_qt4} \
                               %{?qmake_qt5:--qmake-qt5=%{-qmake_qt5}} \
                     install --root=$RPM_BUILD_ROOT
make install GRDIR=%{grdir} DESTDIR=${RPM_BUILD_ROOT}

%clean
%{__python} setup.py clean --all

%files
%defattr(-,root,root)
%{python_sitearch}

%files -n gr
%defattr(-,root,root)
%{grdir}
