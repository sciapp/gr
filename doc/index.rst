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

Starting with release 0.6 *GR* can be used as a backend
for `Matplotlib <http://matplotlib.org>`_ and significantly improve
the performance of existing Matplotlib applications.
In :doc:`this <tutorials/matplotlib>` tutorial section you can find
some examples.

Getting Started
---------------

If you want to get the very latest version can clone the
`GR repository <https://github.com/jheinen/gr>`_ from GitHub::

    git clone https://github.com/jheinen/gr

The *GR* framewok is also available on PyPI:
`https://pypi.python.org/pypi/gr <https://pypi.python.org/pypi/gr/>`_

The installation method depends on the environment in which GR will
be used. For further information please refer to the :doc:`installation documentation <installation>`.

Once you have installed the GR framework you simply need to type::

    from gr import pygr
    pygr.plot([0,1,4],[3,2,5])

At this point, you should browse the gallery to get an impression
of GR's capabilities.

Documentation
-------------

This is the documentation for the GR framework.

*  :doc:`GR Reference <gr>`
*  :doc:`GR3 Reference <gr3>`
*  :doc:`Tutorials <tutorials/index>`

.. toctree::
   :hidden:

   installation.rst
   gr.rst
   gr3.rst
   Examples <examples/index.rst>
   tutorials/index.rst
   credits.rst
   references.rst
   imprint.rst

**Other resources**

* The GR framework has already been presented in a talk at PyCon DE `2012 <https://2012.de.pycon.org/programm/schedule/sessions/54>`_ and `2013 <https://2013.de.pycon.org/schedule/sessions/45/>`_, during a `poster session <https://us.pycon.org/2013/schedule/presentation/158/>`_ at PyCon US 2013, at `PythonCamps 2013 <http://josefheinen.de/rasberry-pi.html>`_ in Cologne and at `EuroPython 2014 <https://ep2014.europython.eu/en/schedule/sessions/86/>`_ in Berlin. The are also HTML versions of the talks at `PyCon.DE 2013 <http://iffwww.iff.kfa-juelich.de/pub/doc/PyCon_DE_2013>`_ and `EP14 <http://iffwww.iff.kfa-juelich.de/pub/doc/EP14>`_.

* There is an active developer group and :doc:`list <credits>` of people who have made significant contributions. More information can be found :doc:`here <references>`.

Indices and tables
------------------

* :ref:`genindex`
* :ref:`search`

