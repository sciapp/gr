GR - a universal framework for visualization applications
=========================================================

[![MIT license](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE.md)
[![GitHub tag](https://img.shields.io/github/tag/jheinen/gr.svg)](https://github.com/jheinen/gr/releases)
[![PyPI version](https://img.shields.io/pypi/v/gr.svg)](https://pypi.python.org/pypi/gr)

*GR* is a universal framework for cross-platform visualization applications.
It offers developers a compact, portable and consistent graphics library for
their programs. Applications range from publication quality 2D graphs to the
representation of complex 3D scenes.

*GR* is essentially based on an implementation of a Graphical Kernel System (GKS)
and OpenGL. As a self-contained system it can quickly and easily be integrated
into existing applications (i.e. using the `ctypes` mechanism in Python or `ccall`
in Julia).

The *GR* framework can be used in imperative programming systems or integrated
into modern object-oriented systems, in particular those based on GUI toolkits.
*GR* is characterized by its high interoperability and can be used with modern
web technologies. The *GR* framework is especially suitable for real-time
or signal processing environments.

*GR* was developed by the Scientific IT-Systems group at the Peter Grünberg
Institute at Forschunsgzentrum Jülich. The main development has been done
by Josef Heinen who currently maintains the software, but there are other
developers who currently make valuable contributions. Special thanks to
Florian Rhiem (*GR3*] and Christian Felder (qtgr, setup.py).

Starting with release 0.6 *GR* can be used as a backend
for [Matplotlib](http://matplotlib.org) and significantly improve
the performance of existing Matplotlib or PyPlot applications written
in Python or Julia, respectively.
In [this](http://gr-framework.org/tutorials/matplotlib.html) tutorial
section you can find some examples.

Beginning with version 0.10.0 *GR* supports inline graphics which shows
up in IPython's Qt Console or interactive computing environments for *Python*
and *Julia*, such as [IPython](http://ipython.org) and
[Jupyter](https://jupyter.org). An interesting example can be found
[here](http://pgi-jcns.fz-juelich.de/pub/doc/700K_460.html).

For further information please refer to the [GR home page](http://gr-framework.org).
