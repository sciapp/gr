GR/GR3 and Matplotlib Interoperability Tutorial
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Matplotlib is the tool of choice for 2D data visualization and the creation
of publication-quality scientific graphics. Starting with version 0.6.0 of
the *GR* framework *Matplotlib*'s capabilities can be greatly expanded,
especially for dynamic or interactive plots.

In the following example it is demonstrated how different graphics
modules can be combined to produce a single animated plot:

 * A histogram created with *Matplotlib*
 * Results of a physics simulation rendered with
   `Mogli <https://pypi.python.org/pypi/mogli>`_/*GR3*
 * Annotation and a simple line graph (created with *GR*)

.. code-block:: python

    import numpy as np
    import matplotlib.pyplot as plt
    import mogli
    import gr

    molecules = mogli.read('700K_460.xyz')
    angles = np.load('700K_460.npy')
    lens = []

    gr.setregenflags(gr.MPL_POSTPONE_UPDATE)

    for t in range(100):

        plt.cla()
        fig = plt.subplot(133)
        fig.xaxis.set_ticks([-100, 0, 100])
        fig.yaxis.set_ticks([])
        plt.ylim([0, 1000])
        plt.hist(angles[t], 20, normed=0, facecolor='g', alpha=0.5)
        plt.show()

        gr.setviewport(0, 0.7, 0, 0.7)
        gr.setwindow(0.1, 0.9, 0.05, 0.85)
        mogli.draw(molecules[t], bonds_param=1.15, camera=((60, 0, 0),
                                                           (0, 0, 0),
                                                           (0, 1, 0)))

        gr.settextalign(gr.TEXT_HALIGN_CENTER, gr.TEXT_VALIGN_HALF)
        gr.text(0.35, 0.7, '700K (%.1f ps)  # of bonds: %d' %
                (t / 10.0, len(angles[t])))
        lens.append(len(angles[t]))
        if t > 0:
            gr.setwindow(0, 10, 3500, 5000)
            gr.setviewport(0.1, 0.6, 0.05, 0.1)
            gr.axes(1, 0, 0, 3500, 2, 0, 0.005)
            gr.polyline(np.arange(t + 1) / 10.0, lens)

        gr.updatews()

Looking toward the future, this approach may be a significant feature
enhancement for *Matplotlib*. There is no need to change a single line of code,
neither for the Matplotlib part nor for the *GR*/*GR3* part. The *GR* Matplotlib
backend can be activated by setting the ``MPLBACKEND`` environment variable
previous before starting Python::

        % export MPLBACKEND=gr
        % python ...

Even if you'd like to create animations no further module (like *Matplotlib*'s
animation toolkit) or any post-processing is required. To create an
MPEG video, you simply have to redirect the *GR* output to the movie
device driver by setting the ``GKS_WSTYPE`` (= *GKS* workstation type)
environment variable)::

        % export GKS_WSTYPE=mov
        % python ...

.. raw:: html

    <video width="640" height="480" autoplay controls>
    <source src="/media/mpl_interop/700K_460.mp4" type="video/mp4">
    <source src="/media/mpl_interop/700K_460.ogg" type="video/ogg">
    <source src="/media/mpl_interop/700K_460.webm" type="video/webm">
    </video>

----

To view the animation in a browser window, first start *GR*'s *Bottle* server::

        % cd lib/gks/html5
        % python server.py 
        Bottle v0.13-dev server starting up (using GeventServer())...
        Listening on http://127.0.0.1:8080/
        Hit Ctrl-C to quit.

Then, open the above URL (http://127.0.0.1:8080/) in a browser and start
the animation with the output redirected to the *GR*'s HTML5 device driver::

        % export MPLBACKEND=gr
        % python -o html ...

You can then watch the animation in your browser window.

.. raw:: html

    <video width="478" height="540" autoplay controls>
    <source src="/media/mpl_interop/700K_460-browser.mp4" type="video/mp4">
    <source src="/media/mpl_interop/700K_460-browser.ogg" type="video/ogg">
    <source src="/media/mpl_interop/700K_460-browser.webm" type="video/webm">
    </video>

