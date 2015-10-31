Examples built with GR's Matplotlib backend
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Matplotlib is the most popular graphics library for Python. It is the
workhorse plotting utility of the scientific Python world. However,
depending on the field of application, the software may be reaching
its limits. This is the point where the GR framework will help.

Starting with release 0.6.0 the GR framework can be used as a backend
for Matplotlib and significantly improve the performance of existing 
Matplotlib applications, e.g.::

    python artist_reference.py -dmodule://gr.matplotlib.backend_gr

With Matplotlib 1.5 you can set the backend using the ``MPLBACKEND``
environment variable::

    export MPLBACKEND="module://gr.matplotlib.backend_gr"

Some of the Matplotlib examples (along with the unmodified Python code)
are shown below.


.. raw:: html
   
   <script language="JavaScript">
   var i = 0;
   var path = ["artist_reference", "color_cycle_demo", "contourf3d_demo2",
               "ellipse_collection", "gradient_bar", "griddata_demo", "hist",
               "offset_demo", "path_patch_demo", "pcolormesh_levels",
               "polar_bar_demo", "pyplot_mathtext", "surface3d_demo",
               "tex_demo", "tricontour_smooth_delaunay", "trisurf3d_demo",
               "unicode_demo"];

   function swapImage()
   {
      $.get("../media/matplotlib/" + path[i] + ".html", success=function(data) {
          document.slide.src = "../media/matplotlib/" + path[i] + ".png";
          $("#slide_source").html(data);
          if (i < path.length - 1) i++; else i = 0;
          setTimeout("swapImage()", 5000);
      });
   }
   window.onload=swapImage;
   </script>
   
   <img name="slide" src="../media/matplotlib/artist_reference.png" />
   <div id="slide_source"></div>
