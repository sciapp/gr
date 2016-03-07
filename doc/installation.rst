Installation
------------

Linux packages
^^^^^^^^^^^^^^

Since GR v0.17.2 we provide `python-gr` `.rpm` and `.deb` packages for various
Linux distributions using
`openSUSE Build Service <http://build.opensuse.org>`_. Your operating systems
package manager will cope with package dependencies.

Please follow the installation instructions for your operating system described
`here <http://software.opensuse.org/download.html?project=science:gr-framework&package=python-gr>`_.


Prerequisites
^^^^^^^^^^^^^

The GR distribution contains a script (``build_deps``) which may help
you to prepare your system for the installation of the GR framework::

    % lib/build-deps

          OS name: Linux
     Distribution: Red Hat
      Description: Fedora 20 (Heisenbug)
          Release: 20
           Kernel: 3.19.8-100.fc20.x86_64
     Architecture: x86_64

    The following pre-installation steps are required for a GR framework
    software installation:

    yum install gcc gcc-c++ gcc-gfortran PyQt4-devel ghostscript-devel \
    texlive-collection-latex texlive-dvipng cmake patch


The following table shows which packages are required to support the different
features:

+---------------+----------------------+----------------------+
|               |- Debian / Rasbian    |- Fedora              |
|               |- Ubuntu              |- RedHat              |
+===============+======================+======================+
|**required:**                                                |
+---------------+----------------------+----------------------+
|               |- make                |Development Tools     |
|               |- gcc                 |                      |
|               |- g++                 |                      |
+---------------+----------------------+----------------------+
|               |- libx11-dev          |Basic X Window System |
|               |- libxft-dev          |                      |
|               |- libxt-dev           |                      |
+---------------+----------------------+----------------------+
|Python support |- python2.7-dev       |- python-devel        |
|               |- python-numpy        |- numpy               |
+---------------+----------------------+----------------------+
|**recommended:**                                             |
+---------------+----------------------+----------------------+
|LaTeX support  |- texlive-latex3      |- texlive             |
|               |- dvipng              |- dvipng              |
+---------------+----------------------+----------------------+
|OpenGL         |- python-opengl       |PyOpenGL              |
|               |- libgl1-mesa-dev     |                      |
+---------------+----------------------+----------------------+
|Qt embedding   |- qt4-dev-tools       |- qt4-devel           |
|               |- pyqt4-dev-tools     |- PyQt4               |
+---------------+----------------------+----------------------+
|**optional:**                                                |
+---------------+----------------------+----------------------+
|Image output   |libgs-dev             |ghostscript-devel     |
+---------------+----------------------+----------------------+
|PDF import     |libmupdf-dev          |mupdf-devel           |
+---------------+----------------------+----------------------+
|MOV export     |        ffmpeg-2.x (source build)            |
+---------------+----------------------+----------------------+
|wxWidgets      |- libgtk2.0-dev       |- wxBase.x86_64       |
|               |- libwxgtk2.8-dev     |- wxGTK-devel.x86_64  |
|               |- python-wxgtk2.8     |- wxPython            |
+---------------+----------------------+----------------------+
|GLFW           |libglfw3-dev          |glfw-devel            |
+---------------+----------------------+----------------------+
|ØMQ            |libzmq3-dev           |zeromq3-devel         |
+---------------+----------------------+----------------------+
|**for the demos:**                                           |
+---------------+----------------------+----------------------+
|Audio demos    |python-pyaudio        |pyaudio               |
+---------------+----------------------+----------------------+

Instead of manually installing Python and required modules it's highly
recommended to use a python bundle, eg.
`Anaconda <http://continuum.io/downloads>`_, instead. This is
especially true for Windows and OS X.

Those distributions provide more packages that you can think you will ever
need and they are very easy to update using package managers.

.. note::
    We are providing binary packages for Anaconda, which can be
    obtained from `conda.anaconda.org <http://conda.anaconda.org>`_ and
    then be installed with the ``conda install`` command.

For Linux, Windows or OS X you can download and install ready-to-use
packages with a single command::

    conda install -c https://conda.anaconda.org/jheinen gr
    python

If you don't have Miniconda or Anaconda installed, you can download
them from `Continuum <http://continuum.io/downloads>`_ and use a
command-line installer. For Miniconda on OS X, in the shell execute::

    bash Miniconda-3.x.x-MacOSX-x86_64.sh
    export PATH=~/miniconda/bin:$PATH
    conda install -c https://conda.anaconda.org/jheinen gr
    python

Alternatively, to use the GR framework with Anaconda, you simply have
to extend the Python path::

    export GRDIR=/usr/local/gr
    export PYTHONPATH=${PYTHONPATH}:${GRDIR}/lib/python
    anaconda


Source install from git
^^^^^^^^^^^^^^^^^^^^^^^

Clone the main source using::

    git clone https://github.com/jheinen/gr

Installation as python package using `setup.py`::

    python setup.py install

The `setup.py` script will perform some prerequisite checks and start the
build process. In order to check which plugins can be build before starting
the build process use the following command::

    python setup.py check_ext

This should printout something similar like this:

.. code-block:: python

    isLinuxOrDarwin:  True
            isLinux:  False
           isDarwin:  True
            isWin32:  False

         OSX target:  10.6

             x11lib:  ['/usr/X11R6/lib']
             x11inc:  ['/usr/X11R6/include']
            x11libs:  []
         x11ldflags:  ['-L/usr/X11R6/lib', '-lXt']
          x11cflags:  ['-I/usr/X11R6/include']

           wxconfig:  /usr/local/bin/wx-config
              wxdir:  None
              wxlib:  []
              wxinc:  []
             wxlibs:  []
          wxldflags:  ['-L/usr/local/wx/lib', '-framework', 'IOKit',
                       '-framework', 'Carbon', '-framework', 'Cocoa',
                       '-framework', 'AudioToolbox', '-framework', 'System',
                       '-framework', 'OpenGL', '-lwx_osx_cocoau-3.0']
              wxcxx:  ['-I/usr/local/wx/lib/wx/include/osx_cocoa-unicode-3.0',
                       '-I/usr/local/wx/include/wx-3.0',
                       '-D_FILE_OFFSET_BITS=64', '-DWXUSINGDLL',
                       '-D__WXMAC__', '-D__WXOSX__', '-D__WXOSX_COCOA__']

         gtkldflags:  ['-L/usr/local/lib', '-lgtk-quartz-2.0',
                       '-lgdk-quartz-2.0', '-lpangocairo-1.0', '-lpango-1.0',
                       '-latk-1.0', '-lcairo', '-lgdk_pixbuf-2.0', '-lgio-2.0',
                       '-lgobject-2.0', '-lglib-2.0', '-lintl']
          gtkcflags:  ['-D_REENTRANT', '-I/usr/local/include/gtk-2.0',
                       '-I/usr/local/lib/gtk-2.0/include',
                       '-I/usr/local/include/pango-1.0',
                       '-I/usr/local/include/atk-1.0',
                       '-I/usr/local/include/cairo',
                       '-I/usr/local/include/pixman-1', '-I/usr/local/include',
                       '-I/usr/local/include/freetype2',
                       '-I/usr/local/include/libpng16',
                       '-I/usr/local/include/gdk-pixbuf-2.0',
                       '-I/usr/local/include/libpng16',
                       '-I/usr/local/include/glib-2.0',
                       '-I/usr/local/lib/glib-2.0/include']

              qmake:  /usr/local/qt-4.8/bin/qmake
              qtdir:  /usr/local/qt-4.8
              qtinc:  ['/usr/local/qt-4.8/include']
              qtlib:  ['/usr/local/qt-4.8/lib']
             qtlibs:  ['QtGui', 'QtCore']
          qtldflags:  []
         Qt version:  [4, 8, 6]

              gsdir:  None
              gsinc:  ['/usr/local/include']
              gslib:  []
             gslibs:  ['gs', 'Xt', 'X11', 'iconv']
          gsldflags:  ['-L/usr/X11R6/lib', '-L/usr/local/lib', '-lgs', '-lXt',
                       '-lX11', '-liconv']

              grdir:  lib/python2.7/site-packages/gr-0.17.1.post35-py2.7-macosx-10.4-x86_64.egg/gr

    freetype-config:  /usr/local/bin/freetype-config
          ftldflags:  ['-L/usr/local/lib', '-lfreetype']
           ftcflags:  ['-I/usr/local/include/freetype2']

           mupdfinc:  ['/usr/local/include']
          mupdflibs:  ['mupdf', 'jbig2dec', 'jpeg', 'openjp2', 'z', 'm']
       mupdfldflags:  ['-L/usr/local/lib', '-L/usr/local/lib', '-lfreetype',
                       '-lmupdf', '-ljbig2dec', '-ljpeg', '-lopenjp2', '-lz',
                       '-lm', '-lmupdf', '-ljbig2dec', '-ljpeg', '-lopenjp2',
                       '-lz', '-lm', '-lmupdf', '-ljbig2dec', '-ljpeg',
                       '-lopenjp2', '-lz', '-lm']

         opengllibs:  []
       opengldflags:  ['-framework', 'OpenGL', '-framework', 'Cocoa']

        disable-x11:  False
         disable-xt:  False
        disable-xft:  False
         disable-wx:  False
         disable-qt:  False
        disable-gtk:  False
         disable-gs:  False
        disable-fig:  False
        disable-svg:  False
       disable-html:  False
        disable-pgf:  False
        disable-wmf:  False
        disable-mov:  False
     disable-opengl:  False
     disable-quartz:  False
   disable-freetype:  False
      disable-mupdf:  False

In order to build and install a self-contained gr package you can use the
following command::

    python setup.py build_ext --static-extras install

Installation into single directory using `Makefile`::

    cd gr
    make
    make install
    make clean
    export PYTHONPATH=${PYTHONPATH}:/usr/local/gr/lib/python

This will install the GR framework into the directory ``/usr/local/gr``. You can
choose any other destination by specifying the ``GRDIR`` variable, e.g.::

    make GRDIR=/opt/gr

To create a self-contained GR distribution you can use the ``self`` target::

    make self

On slow systems, you can have a coffee now, as the system will download
and build several static libraries.

The GR distribution provides some wrapper scripts for python and anaconda environments.
You can change the default locations for the corresponding python interpreter by
specifying the ``PYTHONBIN`` and ``ANACONDABIN`` variables, e.g.::

    make install GRDIR=/opt/gr PYTHONBIN=/usr/local/bin ANACONDABIN=/usr/local/anaconda2/bin

Install from PyPI
^^^^^^^^^^^^^^^^^

::

    pip install gr

Once you have installed the GR framework you should try whether you can import
the gr module::

    > python
    Python 2.7.8 (default, Jul  3 2014, 21:06:26)
    [GCC 4.2.1 (Based on Apple Inc. build 5658) (LLVM build 2336.1.00)] on darwin
    Type "help", "copyright", "credits" or "license" for more information.
    >>> import gr

