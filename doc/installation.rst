Installation
------------

Prerequisites
^^^^^^^^^^^^^

The following table shows which packages are required to support the different
features:

+---------------+----------------------+----------------------+
|               |- Debian / Rasbian    |- Fedora              |
|               |- Ubuntu              |- RedHat              |
+===============+======================+======================+
|**required:**                                                |
+---------------+----------------------+----------------------+
|               |                      |Development Tools     |
+---------------+----------------------+----------------------+
|               |- libx11-dev          |Basic X Window System |
|               |- libxft-dev          |                      |
|               |- libxt-dev           |                      |
+---------------+----------------------+----------------------+
|Python support |- python2.7-dev       |- python-devel        |
|               |- numpy               |- numpy               |
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
|PDF import     |mupdf-dev             |mupdf-devel           |
+---------------+----------------------+----------------------+
|MOV export     |        ffmpeg-2.x (source build)            |
+---------------+----------------------+----------------------+
|wxWidgets      |- libgtk2.0-devs      |- wxBase.x86_64       |
|               |- libwxgtk2.8-dev     |- wxGTK-devel.x86_64  |
|               |- python-wxgtk2.8     |- wxPython            |
+---------------+----------------------+----------------------+
|GLFW           |                      |glfw-devel            |
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

and build and install as usual with::

    cd gr
    make
    make install
    make clean
    export PYTHONPATH=${PYTHONPATH}:/usr/local/gr/lib/python

This will install the GR framework into the directory ``/usr/local/gr``. You can
choose any other destination by specifying the ``GRDIR`` variable, e.g.::

    make GRDIR=/opt/gr

Install from PyPi
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

