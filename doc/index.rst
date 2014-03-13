.. gr documentation master file, created by
   sphinx-quickstart on Fri Feb 21 15:25:51 2014.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Introduction
------------

*GR* is a universal framework for cross-platform visualization applications.
It offers developers a compact, portable and consistent graphics library for
their programs. Applications range from publication quality 2D graphs to the
representation of complex 3D scenes.

.. image:: media/screenshots.png

----

*GR* is essentially based on an implementation of a Graphical Kernel System (GKS)
and OpenGL. As a self-contained system it can quickly and easily be integrated
into existing applications (i.e. using the ctypes mechanism in Python).
The *GR* framework can be used in imperative programming systems or integrated
into modern object-oriented systems, in particular those based on GUI toolkits.
*GR* is characterized by its high interoperability and can be used with modern
web technologies and mobile devices. The *GR* framework is especially suitable
for real-time environments.

.. image:: media/gr-structure.png

----

*GR* was developed by the `Scientific IT-Systems <https://iffwww.iff.kfa-juelich.de>`_ group at the `Peter Grünberg Institute <http://www.fz-juelich.de/pgi>`_
at `Forschunsgzentrum Jülich <http://www.fz-juelich.de>`_.
The main development has been done by Josef Heinen who currently maintains
the software.

*GR3* is a software library for simple visualization of 3D scenes.
It was developed by Florian Rhiem as part of his bachelor's thesis.
*GR3* is written in C and can also be used from Python through a wrapper
module. 

Getting Started
---------------

If you want to get the very latest version of *GR* direct from the
source code repository then you can use git::

    git clone git://ifflinux.iff.kfa-juelich.de/gr

The *GR* framewok is also available on PyPI package index:
`https://pypi.python.org/pypi/gr <https://pypi.python.org/pypi/gr/>`_

The installation method depends on the environment in which GR will
be used. For further information please refer to the installation
documentation.

Once you have installed the GR framework you simply need to type::

    from gr import pygr
    pygr.plot([0,1,4],[3,2,5])

At this point, you should browse the gallery to get an impression
of GR's capabilities.

Contents
--------

.. toctree::
   :maxdepth: 1

   installation.rst
   gr.rst
   Examples <examples/index.rst>
   gr3.rst
   tutorials/index.rst
   links.rst
   thanks.rst
   imprint.rst


Indices and tables
------------------

* :ref:`genindex`
* :ref:`search`

