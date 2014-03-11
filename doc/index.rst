.. gr documentation master file, created by
   sphinx-quickstart on Fri Feb 21 15:25:51 2014.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

GR framework documentation
--------------------------

*GR* is a universal framework for cross-platform visualization applications.
It offers developers a compact, portable and consistent graphics library for
their programs. Applications range from publication quality 2D graphs to the
representation of complex 3D scenes.

*GR* is essentially based on an implementation of a Graphical Kernel System (GKS)
and OpenGL. As a self-contained system it can quickly and easily be integrated
into existing applications (i.e. using the ctypes mechanism in Python).
The *GR* framework can be used in imperative programming systems or integrated
into modern object-oriented systems, in particular those based on GUI toolkits.
*GR* is characterized by its high interoperability and can be used with modern
web technologies and mobile devices. The *GR* framework is especially suitable
for real-time environments.

*GR3* is a software library for simple visualization of 3D scenes.
It was developed by Florian Rhiem as part of his bachelor's thesis.
*GR3* is written in C and can also be used from Python through a wrapper
module. 

Git repository
--------------

If you want to get the very latest version of *GR* direct from the
source code repository then you can use git::

    git clone git://ifflinux.iff.kfa-juelich.de/gr

The *GR* framewok is also available on PyPI package index:
`https://pypi.python.org/pypi/gr <https://pypi.python.org/pypi/gr/>`_


**Contents:**

.. toctree::
   :maxdepth: 1

   installation
   gr
   Examples <examples/examples.rst>
   gr3
   links
   imprint


Thanks
------

* Marcel Dück for the Java code and the iGR iOS App
* Christian Felder for the qtgr module and the GKS iOS App
* Marvin Goblet for the initial GKSTerm implementation
* Ingo Heimbach for implementing the GKS wxWidgets plugin
* David Knodt for the video converter routines
* Robert Nesselrath for the glgr application
* Florian Rhiem for the design and implementation of *GR3*
* Elmar Westphal for the gr_textex routine
* Jörg Winkler for the GKS OpenGL device driver


Indices and tables
------------------

* :ref:`genindex`
* :ref:`search`


