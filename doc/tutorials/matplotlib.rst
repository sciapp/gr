Examples built with GR's Matplotlib backend
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Matplotlib is the most popular graphics library for Python. It is the
workhorse plotting utility of the scientific Python world. However,
depending on the field of application, the software may be reaching
its limits. This is the point where the GR framework will help.

Starting with release 0.6.0 the GR framework can be used as a backend
for Matplotlib and significantly improve the performance of existing 
Matplotlib applications, e.g.::

    export GRDIR=/usr/local/gr
    export PYTHONPATH=${PYTHONPATH}:${GRDIR}/lib/python
    python artist_reference.py -dmodule://gr.matplotlib.backend_gr

Some of the original Matplotlib examples are shown below.


.. raw:: html
   
   <script language="JavaScript">
   var i = 0;
   var path = new Array();
    
   path[0] = "artist_reference.png";
   path[1] = "color_cycle_demo.png";
   path[2] = "contourf3d_demo2.png";
   path[3] = "ellipse_collection.png";
   path[4] = "gradient_bar.png";
   path[5] = "griddata_demo.png";
   path[6] = "hist.png";
   path[7] = "offset_demo.png";
   path[8] = "path_patch_demo.png";
   path[9] = "pcolormesh_levels.png";
   path[10] = "polar_bar_demo.png";
   path[11] = "pyplot_mathtext.png";
   path[12] = "surface3d_demo.png";
   path[13] = "tex_demo.png";
   path[14] = "tricontour_smooth_delaunay.png";
   path[15] = "trisurf3d_demo.png";
   path[16] = "unicode_demo.png";
   
   function swapImage()
   {
      document.slide.src = "../media/matplotlib/" + path[i];
      if (i < path.length - 1) i++; else i = 0;
      setTimeout("swapImage()", 3000);
   }
   window.onload=swapImage;
   </script>
   
   <img name="slide" src="../media/matplotlib/artist_reference.png" />
 
