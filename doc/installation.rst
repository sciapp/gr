Installation
------------

Prerequisites
^^^^^^^^^^^^^

The following table show which packages are required to support the different
features:

+---------------+----------------------+---------------------+
|               |- Debian / Rasbian    |- Fedora             |
|               |- Ubuntu              |- RedHat             |
+===============+======================+=====================+ 
|**required:**                                               |
+---------------+----------------------+---------------------+
|               |                      |Development Tools    |
+---------------+----------------------+---------------------+
|               |- libx11-dev          |Basic X Window System|
|               |- libxft-dev          |                     |
|               |- libxt-dev           |                     |
+---------------+----------------------+---------------------+
|Python support |python2.7-dev         |python-devel         |
+---------------+----------------------+---------------------+
|**optional:**                                               |
+---------------+----------------------+---------------------+
|LaTeX support  |- texlive-latex3      |- texlive            |
|               |- dvipng              |- dvipng             |
+---------------+----------------------+---------------------+
|Image output   |libgs-dev             |ghostscript-devel    |
+---------------+----------------------+---------------------+
|Qt embedding   |- qt4-dev-tools       |- qt4-devel          |
|               |- pyqt4-dev-tools     |- PyQt4              |
+---------------+----------------------+---------------------+
|wxWidgets      |- libgtk2.0-devs      |- wxBase.x86_64      |
|               |- libwxgtk2.8-dev     |- wxGTK-devel.x86_64 |
|               |- python-wxgtk2.8     |- wxPython           |
+---------------+----------------------+---------------------+
|OpenGL         |- python-opengl       |PyOpenGL             |
|               |- libgl1-mesa-dev     |                     |
+---------------+----------------------+---------------------+
|GLFW           |                      |glfw-devel           |
+---------------+----------------------+---------------------+
|Audio demos    |python-pyaudio        |pyaudio              |
+---------------+----------------------+---------------------+

Instead of manually installing Python and required modules it's highly
recommended to use a python bundle, eg. Anaconda or Enthought, instead.
Those distributions provide more packages that you can think you will ever
need and they are very easy to update using package managers.


Source install from git
^^^^^^^^^^^^^^^^^^^^^^^

Clone the main source using::

    git clone git://ifflinux.iff.kfa-juelich.de/gr

and build and install as usual with::

    cd gr
    make
    make install
    make clean
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

