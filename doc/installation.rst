Installation
------------

Prerequisites
^^^^^^^^^^^^^

* Debian:
    - libx11-dev
    - libxft-dev
    - libxt-dev
    - python2.7-dev
    - libgl1-mesa-dev
    - libgs-dev
    - texlive-latex3
    - dvipng
    - qt4-dev-tools
    - libgtk2.0-dev
    - pyqt4-dev-tools
    - libwxgtk2.8-dev
    - python-wxgtk2.8
    - python-pyaudio
    - python-opengl

* Fedora:
    - Development Tools
    - Basic X Window System
    - qt4-devel
    - wxBase.x86_64
    - wxGTK-devel.x86_64
    - ghostscript-devel
    - python-devel
    - glfw-devel
    - texlive
    - dvipng
    - PyQt4
    - wxPython
    - pyaudio

Source install from git
^^^^^^^^^^^^^^^^^^^^^^^

Clone the main source using::

    git clone git://ifflinux.iff.kfa-juelich.de/gr

and build and install as usual with::

    cd gr
    make; make install; make clean
    export PYTHONPATH=${PYTHONPATH}:/usr/local/gr/lib/python

Install from PyPi
^^^^^^^^^^^^^^^^^

::

    pip install gr

Once you have installed the GR framework you should try whether you can import
the gr module::

    > python
    Python 2.7.6 (default, Nov 14 2013, 07:31:25) 
    [GCC 4.2.1 (Based on Apple Inc. build 5658) (LLVM build 2336.1.00)] on darwin
    Type "help", "copyright", "credits" or "license" for more information.
    >>> import gr

