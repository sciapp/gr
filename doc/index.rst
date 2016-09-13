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
and *OpenGL*. As a self-contained system it can quickly and easily be integrated
into existing applications (i.e. using the ``ctypes`` mechanism in
`Python <http://python.org>`_ or direct calls from
`Julia <http://julialang.org>`_ with ``ccall`` syntax).

The *GR* framework can be used in imperative programming systems or integrated
into modern object-oriented systems, in particular those based on GUI toolkits.
*GR* is characterized by its high interoperability and can be used with modern
web technologies and mobile devices. The *GR* framework is especially suitable
for real-time environments.

.. image:: media/gr-structure.png

----

*GR* was developed by the `Scientific IT-Systems <https://pgi-jcns.fz-juelich.de>`_ group at the `Peter Grünberg Institute <http://www.fz-juelich.de/pgi>`_
at `Forschunsgzentrum Jülich <http://www.fz-juelich.de>`_.
The main development has been done by Josef Heinen who currently maintains
the software.

*GR3* is a software library for simple visualization of 3D scenes.
It was developed by Florian Rhiem as part of his bachelor's thesis.
*GR3* is written in C and can also be used from *Python* or *Julia* through
a wrapper module.

Starting with release 0.6 *GR* can be used as a backend
for `Matplotlib <http://matplotlib.org>`_ and significantly improve
the performance of existing Matplotlib or PyPlot applications written
in Python or Julia, respectively.
In :doc:`this <tutorials/matplotlib>` tutorial section you can find
some examples.

Beginning with version 0.10.0 *GR* supports inline graphics which shows
up in IPython's Qt Console or interactive computing environments for *Python*
and *Julia*, such as `IPython <http://ipython.org>`_ and
`Jupyter <https://jupyter.org>`_.
A simple *IPython* notebook example (converted to HTML5) can be found
`here <https://pgi-jcns.fz-juelich.de/pub/doc/700K_460.html>`_.

Getting Started
---------------

If you want to get the very latest version you can clone the
`GR repository <https://github.com/jheinen/gr>`_ from GitHub::

    git clone https://github.com/jheinen/gr

The *GR* framewok is also available on PyPI:
`https://pypi.python.org/pypi/gr <https://pypi.python.org/pypi/gr/>`_

The installation method depends on the environment in which GR will
be used. For further information please refer to the :doc:`installation documentation <installation>`.

Once you have installed the GR framework, in Python you simply need to type::

    from gr import pygr
    pygr.plot([0,1,4],[3,2,5])

At this point, you should browse the gallery to get an impression
of GR's capabilities.

For the Julia programming language an official
`GR.jl <https://github.com/jheinen/GR.jl>`_ package has been registered.
You can add the GR framework to your Julia installation with the
``Pkg.add()`` function:

.. code-block:: julia

    Pkg.add("GR")
    ...
    using GR
    plot([0,1,4],[3,2,5])

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

* The GR framework has already been presented in a talk at PyCon DE `2012 <https://2012.de.pycon.org/programm/schedule/sessions/54>`_ and `2013 <https://2013.de.pycon.org/schedule/sessions/45/>`_, during a `poster session <https://us.pycon.org/2013/schedule/presentation/158/>`_ at PyCon US 2013, at `PythonCamps 2013 <http://josefheinen.de/rasberry-pi.html>`_ in Cologne, at EuroPython `2014 <https://ep2014.europython.eu/en/schedule/sessions/86/>`_ and `2015 <https://ep2015.europython.eu/conference/talks/speeding-up-matplotlib-with-gr>`_, at `EuroSciPy 2015 <https://www.euroscipy.org/2015/schedule/presentation/12/>`_, JuliaCon `2015 <http://juliacon.org/2015/>`_ and `2016 <http://juliacon.org/abstracts.html#GR>`_, and `SciPy 2016 <http://scipy2016.scipy.org/ehome/146062/332965/>`_. The are also HTML versions of the talks at `PyCon.DE 2013 <http://pgi-jcns.fz-juelich.de/pub/doc/PyCon_DE_2013>`_, EuroPython `2014 <http://pgi-jcns.fz-juelich.de/pub/doc/EP14>`_ and `2015 <https://pgi-jcns.fz-juelich.de/pub/doc/EP15/talk>`_, JuliaCon `2015 <http://pgi-jcns.fz-juelich.de/pub/doc/JuliaCon_2015/html>`_ and `2016 <http://pgi-jcns.fz-juelich.de/pub/doc/JuliaCon_2016/html>`_, `EuroSciPy 2015 <https://pgi-jcns.fz-juelich.de/pub/doc/EuroSciPy_2015/00-talk>`_ and `SciPy 2016 <https://pgi-jcns.fz-juelich.de/pub/doc/SciPy_2016/html>`_.

* There is an active developer group and :doc:`list <credits>` of people who have made significant contributions. More information can be found :doc:`here <references>`.

Indices and tables
------------------

* :ref:`genindex`
* :ref:`search`

