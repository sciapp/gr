# GR Plot Overview

## Introduction

This program allows to create plots from the command line while using simple key-value pairs as arguments. The latter
are converted into GRM containers which will then be used to plot the data.

## Command line arguments

The following parameters are key-value pairs which can be used for every plot type.

- `file`: contains the data which should be displayed. If no file is referred this results in an error message. If this
  parameter is the first argument the `file` keyword may be ommited. More information about these files can be found
  under the subchapter `Data file`.
- `kind`: defines the plot type that should be displayed. Possible options are: `barplot`, `contour`, `contourf`
  , `heatmap`, `hexbin`, `hist`, `imshow`, `isosurface`, `line`, `marginalheatmap`, `polar`, `polar_histogram`
  , `polar_heatmap`, `pie`, `plot3`, `scale`, `scatter`, `scatter3`, `shade`, `surface`, `stem`, `step`, `tricont`
  , `trisurf`, `quiver`, `volume`, `wireframe`.
  The default plot type is `line`.

  To get extra informations about a specific plot type use:

    ```shell
    --help plot_type
    ```
  Alternatively `-h` can be used instead of `--help`.

There is another parameter that can be used for all two-dimensional data sets:

- `keep_aspect_ratio` or `aspect`: defines whether the aspect ratio of the data is kept or not. Possible values for this
  parameter are 0 or 1.

For plots where multiple columns are read there is also a parameter that allows to select columns.

- `columns`: define the columns of the file which should be respected in the plot. The default is all columns. If all
  columns from x to y should be drawn use `x:y`. To select more than 1 specific column use the `,` without whitespace as
  separator.

There are more key-value parameters. These parameter only effect specific plot types. For example `bar_width` only makes
sense, when bars are drawn. These parameters are:

`accelerate`, `algorithm`, `bar_color`, `bar_width`, `bin_counts`, `bin_edges`, `c`, `colormap`, `draw_edges`
, `edge_color`, `edge_width`, `grplot`, `isovalue`, `kind`, `levels`, `marginalheatmap_kind`, `markertype`, `nbins`
, `normalization`, `orientation`, `phiflip`, `rotation`, `scatterz`, `spec`, `stairs`, `step_where`, `style`, `tilt`
, `xbins`, `xcolormap`, `xflip`, `xform`, `xticklabels`, `ybins`, `ycolormap`, `yflip`, `ylabels`

All parameters are separated by a blank. Some parameters are more complex than others. These parameters represent a
container inside GRM.

The valid container parameters are `error`, `ind_bar_color`, `ind_edge_color`, `ind_edge_width`.

Each parameter container can contain one or more of the following parameters:

1. `downwardscap_color`: only available for `error`
2. `errorbar_color`: only available for `error`
3. `indices`: only available for `ind_bar_color`, `ind_edge_color`, `ind_edge_width`
4. `rgb`: only available for `ind_bar_color`, `ind_edge_color`
5. `upwardscap_color`: only available for `error`
6. `width`: only available for `ind_edge_width`

Container parameters follow a different syntax than normal parameters. The parameters inside the container are enclosed
by `{` `}`. If more than one parameter is given to the container each of the key-value pairs is enclosed by `{` `}`
while the parameters are seperated by `,` without whitespace characters.

An example for some parameters on the command line:

```shell
file:covid19.csv kind:line columns:1:3,5
```

This is an advanced example for a command line, where container parameters are set:

```shell
../../../../lib/grm/grplot/data/test.dat kind:barplot ind_edge_width:2,{{indices:2},{width:5.0},{indices:1},{width:8.0}} error:{errorbar_color:4}
```

If wanted the plot can be exported as a `pdf`, `png`, `jpeg` or `svg` file using the interactive menu. Alternatively
these files can directly be created from console line when the enviroment variable `GKS_WSTYPE` is set.

## Data file

These files contain the data that should be plotted. Besides the data these files can include parameters which modify
the plot. The first lines define parameters just like the `title` and have the following pattern:

```text
# key : value
```

Valid keys are:

1. `title`: sets the title of the plot
2. `xlabel`, `ylabel`, `zlabel`: set the label for the respective axis
3. `resample_method`: defines how the data is resampled when needed
4. `location`: defines where the legend should be drawn
5. `xlog`, `ylog`, `zlog`: defines whether the respective axis is plotted logarithmic
6. `xgrid`, `ygrid`, `zgrid`: defines the grid for the respective axis
7. `philim`, `rlim`: defines which part of the specific polar axis should be displayed
8. `xlim`, `ylim`, `zlim`: defines which part of the respective axis should be displayed
9. `xrange`, `yrange`, `zrange`: defines the range of the values on the respective axis

Values are seperated through commas (`,`), e.g. `3, 5`.

The next line after the header may contain the column labels. If the data does not have a label a blank line can be used
instead. Data items are separated with a tab character (`\t`). Depending on the plot type the data is interpreted
differently. The following list shows how the data is treated for different plot types.

1. `contour`, `contourf`, `heatmap`, `imshow`, `marginalheatmap`, `surface`, `wireframe`: The expected data is a matrix.
   Each element of the matrix is displayed according to its position inside the matrix. These elements are interpreted
   as values in x- and y-direction.
2. `line`, `scatter`: One or more columns are expected here. Each column will be displayed in a single plot. The values
   inside the columns gets therefore interpreted as y values.
3. `isosurface`, `volume`: The expected data are multiple matrices. Each matrix represents a slice inside the volume.
4. `plot3`, `scatter`, `scatter3`, `tricont`, `trisurf`: Three columns with data are expected, representing the x, y and
   z data.
5. `barplot`, `hist`, `stem`, `step`: One column is expected which represents the y data.
6. `pie`: The expected data are 1-4 lines. The first line represents the data which should be displayed. The next 3 rows
   are used to set the RGB of the pie charts. Each row stands for one RGB element.
7. `polar_histogram`: One column is expected which represents the values.
8. `polar`: The expected data are two columns containing the angles and values.
9. `polar_heatmap`: The expected data are two columns containing the angles and values.
10. `quiver`: The expected data are two matrices. The first matrix contains the information about the x-directions and
    the second matrix the information about the y-directions.
11. `hexbin`, `shade`: The expected data are two columns, representing the x and y data.

A plot type that expect the same data shape as other plot types can be converted using an interactive menu. The
interaction also yields extra information about the plot, when the mouse is being hovered over them.

## Advanced information for each plot type

# barplot

This plot type converts the data into a bar plot. A bar plot itself is used to display the relationship between a
numeric and a categorical variable. The resulting bars represent the frequency or quantity of different categories of
the data.

The expected data is one column representing the y data.

Possible parameters for the bar plot are:

1. `bar_color`: This parameter defines the color of all bars inside the plot except of those which are referred
   with `ind_bar_color`. The value of this parameter can either be an integer which represents a color index or three
   doubles which represent the RGB value of the color. When the parameter isn`t set the bars will have the color with
   index 989 (dark blue).
2. `bar_width`: This parameter defines the width of all bars inside the plot. Depending on the specified width the bars
   may overlap. The value of this parameter is a double number where its default is 0.8.
3. `edge_color`: This parameter defines the color of all edges inside the plot except of those which are referred
   with `ind_edge_color`. The value of this parameter can either be an integer which represents a color index or three
   doubles which represents the RGB value of the color. When the parameter isn't set the edges will have the color with
   index 1 (black).
4. `edge_width`: This parameter defines the width of all edges inside the plot except those which are referred
   with `ind_edge_width`. The value of this parameter is a double number where its default is 1.0.
5. `error`: With this parameter the relative error of each bar can be displayed. The values for this parameter are
   key-value pairs with the following keys:
    - `errorbar_color`: Defines the color of the error bars as an integer. If no color is given an error is raised.
    - `downwardscap_color`: Defines the downward scap color of the error bars as an integer.
    - `upwardscap_color`: Defines the upward scap color of the error bars as an integer.

   Note: If the error of the bars is to be displayed, the last two columns of the data are used for the error. The
   syntax of this parameter is:

   `error:{{errorbar_color:`color_index`},{downwardscap_color:`color_index`},{upwardscap_color:`color_index`}}`

6. `ind_bar_color`: With this parameter the color of specific bars can be changed. The value of this parameter are
   key-value pairs with the following keys:
    - `indices`: The index number of the bar, which color should be changed.
    - `rgb`: The new color for the specified bar. The value has to be three doubles or integer which represents the rgb
      value.

   The syntax of this parameter is:

   `ind_bar_color:`number_of_bars`,{{indices:`first_bar_index`},{rgb:`r1,g1,b1`}`,...`}`
7. `ind_edge_color`: With this parameter the color of specific edges can be changed. The value of this parameter are
   key-value pairs with the following keys:
    - `indices`: The index number of the edge, which color should be changed.
    - `rgb`: The new color for the specified edge. The value has to be three doubles or integer which represents the rgb
      value.

   The syntax of this parameter is:

   `ind_edge_color:`number_of_edges`,{{indices:`first_edge_index`},{rgb:`r1,g1,b1`}`,...`}`
8. `ind_edge_width`: With this parameter the width of specific edges can be changed. The value of this parameter are
   key-value pairs with the following keys:
    - `indices`: The index number of the edge, which width should be changed.
    - `width`: The new width for the specified edge.or double.

   The syntax of this parameter is:

   `ind_edge_width:`number_of_edges`,{{indices:`first_edge_index`},{width:`first_edge_width`}`,...`}`
9. `orientation`: This parameter defines the orientation of the displayed bars. They can either be drawn `horizontal`
   or `vertical` while the default is `horizontal`.
10. `style`: This parameter defines how the data inside the bar plot is displayed. There are three options:
- `default`: All values are displayed with a separate bar.
- `stacked`: The values are displayed with bars which are stacked over each other.
- `lined`: The values are displayed with smaller bars next to each other.

# contour

This plot type converts the data into a contour plot. A contour plot displays the three-dimensional data over a
rectangular mesh. Depending on the specified number of `levels` contour lines are displayed which represent the
different height values.

The expected data is a matrix. Each element of the matrix is displayed according to its position inside the matrix.
These elements are interpreted as values in x- and y-direction.

Possible parameters for the contour plot are:

1. `colormap` or `cmap`: This parameter defines the colormap to be used. The default is 44 (viridis).
2. `levels`: This parameter defines how many contour lines should be drawn. The default is 20.
3. `xflip`: This parameter defines whether the x-axis is flipped or not.
4. `yflip`: This parameter defines whether the y-axis is flipped or not.

# contourf

This plot type converts the data into a contourf plot. A contourf plot displays the three-dimensional data over a
rectangular mesh. Depending on the specified number of `levels` contour lines are displayed which represent the
different height values. The space between the contour lines is filled with a color respecting the specified colormap.

The expected data is a matrix. Each element of the matrix is displayed according to its position inside the matrix.
These elements are interpreted as values in x- and y-direction.

Possible parameters for the contourf plot are:

1. `colormap` or `cmap`: This parameter defines the colormap to be used. The default is 44 (viridis).
2. `levels`: This parameter defines how many contour lines should be drawn. The default is 20.
3. `scale`: This parameter sets the type of transformation to be used (refer to `gr_setscale`).
4. `xflip`: This parameter defines whether the x-axis is flipped or not.
5. `yflip`: This parameter defines whether the y-axis is flipped or not.

# heatmap

This plot type converts the data into a heatmap. A heatmap uses the current colormap to display the data. Each data
point is represented with a colored square which size depends on the amount of data.

The expected data is a matrix. Each element of the matrix is displayed according to its position inside the matrix and
is interpreted as coordinate in x- and y-direction.

Possible parameters for the heatmap are:

1. `colormap` or `cmap`: This parameter defines the colormap to be used. The default is 44 (viridis).
2. `levels`: This parameter defines how many contour lines should be drawn. The default is 20.
3. `xflip`: This parameter defines whether the x-axis is flipped or not.

# hexbin

This plot type converts the data into a hexbin. A hexbin uses hexagonal binning and the current colormap to display the
data.

Possible parameters for the hexbin are:

1. `colormap` or `cmap`: This parameter defines the colormap to be used. The default is 44 (viridis).
2. `nbins`: This parameter defines the number of bins which represent a hexagonal cell in the plot. The default is 40.
3. `levels`: This parameter defines how many contour lines should be drawn. The default is 20.
4. `xflip`: This parameter defines whether the x-axis is flipped or not.

# hist

This plot type converts the data into a histogram. A histogram is an approximate representation of the distribution of
numerical data. Two columns are expected containing the x and the y data.

Possible parameters for the histogram are:

1. `bar_color`: This parameter defines the color of all bars inside the plot. The value of this parameter can either be
   an integer which represents a color index or three doubles which represents the RGB value of the color. If the
   parameter is omitted color 989 (dark blue) will be used.
2. `bins`: This parameter defines the number of bins which should be used to represent the distribution of the data. The
   defauklt value is 3.3 * log10(pnumber of points) + 0.5) + 1.
3. `edge_color`: This parameter defines the color of all edges inside the plot. The value of this parameter can either
   be an integer which represents a color index or three doubles which represents the rgb value of the color. If the
   parameter is omitted color 1 (black) will be used.
4. `error`: With this parameter the relative error of each bar can be displayed. The parameters are key-value pairs with
   the following keys:
    - `errorbar_color`: Defines the color of the error bars. If no color is given an error is raised.
    - `downwardscap_color`: Defines the downward scap color of the error bars.
    - `upwardscap_color`: Defines the upward scap color of the error bars.

   Note: If the error of the bars is to be displayed, the last two columns of the data are used for the error. The
   syntax of this parameter is:

   `error:{{errorbar_color:`color_index`},{downwardscap_color:`color_index`},{upwardscap_color:`color_index`}}`
5. `orientation`: This parameter defines the orientation of the displayed bars. They can either be drawn `horizontal`
   or `vertical` while the default is `horizontal`.

# imshow

This plot type converts the data into an imshow plot. The data is displayed as a colored image where each data point is
represented as a colored rectangle inside the image.

The expected data is a matrix. Each element of the matrix is displayed according to its position inside the matrix and
are interpreted as coordinates in x- and y-direction.

Possible parameters for the imshow plot are:

1. `colormap` or `cmap`: This parameter defines the colormap to be used. The default is 44 (viridis).
2. `grplot`: This parameter is there for the consistency between the plot types `contour`, `contourf`, `heatmap`
   , `imshow`, `marginalheatmap`, `surface`, `wireframe`. When set the data is displayed in the same order by all of
   these types where in general the imshow plot would be flipped. The value can be either 0 or 1 where the value if not
   otherwise set is always 1.
3. `xflip`: This parameter defines whether the x-axis is flipped or not.
4. `yflip`: This parameter defines whether the y-axis is flipped or not.

# isosurface

This plot type converts the data into an isosurface. An isosurface is a three-dimensional analog of a contour- or
isoline.

The expected data are multiple matrices. Each matrix is similar to a slice inside the volume.

Possible parameters for the isosurface are:

1. `isovalue`: Values greater than the isovalue will be seen as outside the isosurface, while values less than the
   isovalue will be seen as inside the isosurface. The value has to an integer or double where the default is 0.5.
2. `rotation`: This parameter defines the rotation of the displayed data in degrees.or double. The default is no
   rotation.
3. `tilt`: This parameter defines the tilt of the camera in degrees.or double. The default is no tilt.

# line

This plot type converts the data into a line plot. A line plot is a simple plot where the data points are connected by a
polyline.

The expected data are columns. Each column will be interpreted as a single line. Multiple columns leads to multiple
displayed lines. The values inside the columns gets therefore interpreted as y values.

Possible parameters for the line plot are:

1. `error`: With this parameter the relative error of each line can be displayed. The value of this parameter are
   key-value pairs with the following keys:
    - `errorbar_color`: Defines the color of the error bars. The value of this parameter has to be an integer. When no
      color is given this leads to an error.
    - `downwardscap_color`: Defines the downward scap color of the error bars. The value of this parameter has to be an
      integer.
    - `upwardscap_color`: Defines the upward scap color of the error bars. The value of this parameter has to be an
      integer.

      Note: If the error of the points is to be displayed, the last two columns of the data are used for the error. The
      syntax of this parameter is:

   `error:{{errorbar_color:`color_index`},{downwardscap_color:`color_index`},{upwardscap_color:`color_index`}}`

2. `orientation`: This parameter defines the orientation of the displayed lines. They can either be drawn `horizontal`
   or `vertical` while the default is `horizontal`.

# marginalheatmap

This plot type converts the data into a marginalheatmap. A marginalheatmap is a combination of a heatmap and either step
plots or histograms. With those extra plots some parts of the heatmap data are displayed in vertical and horizontal
direction at the margins of the heatmap.

The expected data is a matrix. Each element of the matrix is displayed according to its position inside the matrix.
These elements are interpreted as values in x- and y-direction.

Possible parameters for the marginalheatmap are:

1. `algorithm`: This parameter defines whether the data is summed up or if the maximum to be used for the side plots.
   Possible values are `max` and `sum` where the default is `sum`.
2. `colormap` or `cmap`: This parameter defines the colormap to be used. The default is 44 (viridis).
3. `marginalheatmap_kind` or `hkind`: This parameter defines what part of the data is displayed inside the side plots.
   Possible options are:
    - `all`: When the kind is set to all, the side plots respect the complete data of the heatmap. This is the default
      case.
    - `line`: When the kind is set to line, the side plots respect only a specific row/column of the data.
4. `xflip`: This parameter defines whether the x-axis is flipped or not.
5. `yflip`: This parameter defines whether the y-axis is flipped or not.

# polar

This plot type converts the data into a polar plot. A polar plot displays a polyline in polar coordinates, with theta
indicating the angle in radians and rho indicating the radius value for each point.

The expected data are two columns containing the angles and values.

# polar_heatmap

This plot type converts the data into a polar heatmap. A polar heatmap is a heatmap in polar coordinates.

The expected data are two columns containing the angles and values.

Possible parameters for the polar heatmap are:

1. `colormap` or `cmap`: This parameter defines the colormap to be used. The default is 44 (viridis).

# polar_histogram

This plot type converts the data into a polar histogram. A polar histogram corresponds to a histogram in polar
coordinates.

The expected data is one column containing the values.

Possible parameters for the polar histogram are:

1. `bin_counts`: This parameter sets the amount of classes without binning.
2. `bin_edges`: This parameter sets the borders of the classes.
3. `bin_width`: This parameter sets the width of all the bins.
4. `draw_edges`: When a colormap is used this parameter decides whether the outer shape is drawn or not. The value can
   be either 0 or 1.
5. `nbins`: This parameter sets the number of classes which are respected during the binning.
6. `normalization`: This parameter sets the type of normalization for the polar histogram. The value can be `count`
   , `probability`, `countdensity`, `pdf`, `cumcount` or `cdf`.
7. `phiflip`: This parameter decides whether the phi values are flipped or not. The value can be either 0 or 1.
8. `stairs`: When this parameter is set, only the outer shapes of the bins are drawn. The value can be either 0 or 1.
9. `xcolormap`: This parameter sets the colormap for the x direction.
10. `xflip`: This parameter defines whether the x-axis is flipped or not.
11. `ycolormap`: This parameter sets the colormap for the y direction.
12. `yflip`: This parameter defines whether the y-axis is flipped or not.

# pie

This plot type converts the data into a pie plot. A Pie Chart is a circular statistical plot that can display one series
of data. The area of the chart is the total percentage of the given data. The area of slices of the pie represents the
percentage of the parts of the data.

The expected data are 1-4 lines. The first line represents the data to be displayed. The next three rows are used to set
the RGB values for the slices.

# plot3

This plot type converts the data into a three-dimensional line plot. The data points inside the three-dimensional space
are connected through polylines.

The expected data are three columns containing the x, y and z data.

# scatter

This plot type converts the data into a scatter plot. A scatter plot displays each data point as a point inside the
two-dimensional coordinate system.

The expected data are columns. Each column will be interpreted as a single plot. Multiple columns lead to multiple
plots. The column values are interpreted as y values. There is also a special case of the scatter plot where the size of
the points can be given.

Possible parameters for the scatter plot are:

1. `error`: With this parameter the relative error of each line can be displayed. The value of this parameter are
   key-value pairs with the following keys:
    - `errorbar_color`: Defines the color of the error bars. The value of this parameter has to be an integer. When no
      color is given this leads to an error.
    - `downwardscap_color`: Defines the downward scap color of the error bars. The value of this parameter has to be an
      integer.
    - `upwardscap_color`: Defines the upward scap color of the error bars. The value of this parameter has to be an
      integer.

   Note: If the error of the points is to be displayed, the last two columns of the data are used for the error. The
   syntax of this parameter is:

   `error:{{errorbar_color:`color_index`},{downwardscap_color:`color_index`},{upwardscap_color:`color_index`}}`

2. `markertype`: This parameter defines the style of the visualized data points.where the effect belonging to the
   numbers is the same as for gr_setmarkertype.

# scatter3

This plot type converts the data into a three-dimensional scatter plot. A three-dimensional scatter plot displays each
data point as a point inside the three-dimensional coordinate system.

The expected data are three columns containing the x, y and z data.

# shade

This plot type converts the data into a shade plot. With a shade plot a point or line based heatmap can be drawn from
the data.

The expected data are two columns containing the x and y data.

Possible parameters for the shade plot are:

1. `xbins`: This parameter defines the bins in x direction.where the default is 100.
2. `xform`: This parameter defines the transformation type used for color mapping.where the default is 1. The effect
   belonging to the numbers is the same as for gr_shadepoints.
3. `ybins` : This parameter defines the bins in y direction.where the default is 100.

# surface

This plot type converts the data into a surface plot. A surface plot is to be compared with a visualization of a
function of two variables inside the three-dimensional space.

The expected data is a matrix. Each element of the matrix is displayed according to its position inside the matrix.
These elements are interpreted as values in x- and y-direction.

Possible parameters for the surface plot are:

1. `accelerate`: This parameter defines whether the surface plot is calculated with the GR or the GR3
   functionalities.where in its default case the GR3 to be used.
2. `colormap` or `cmap`: This parameter defines the colormap to be used. The default is 44 (viridis).
3. `xflip`: This parameter defines whether the x-axis is flipped or not.
4. `yflip`: This parameter defines whether the y-axis is flipped or not.

# stem

This plot type converts the data into a stem plot. A stem plot draws lines perpendicular to a baseline at each location
from the baseline to the data values.

The expected data is one column containing the the y data.

Possible parameters for the stem plot are:

1. `orientation`: This parameter defines the orientation of the displayed bars. They can either be drawn `horizontal`
   or `vertical` while the default is `horizontal`.

# step

This plot type converts the data into a step plot. A step plot is a piecewise constant function having only finitely
many pieces.

The expected data is 1 column. This column represents the y data.

Possible parameters for the step plot are:

1. `orientation`: This parameter defines the orientation of the displayed bars. They can either be drawn `horizontal`
   or `vertical` while the default is `horizontal`.
2. `step_where`: This parameter defines the calculation of the steps. The possible values are `pre`, `post` and `mid`
   which is also the default case.

# tricont

This plot type converts the data into a tricontour plot. A tricontour plot displays contour lines on an unstructured
triangular grid.

The expected data are 3 columns. The first columns represents x, the second y and the last the z data.

The possible parameters for the tricontour plot are:

1. `colormap` or `cmap`: This parameter defines the colormap to be used. The default is 44 (viridis).
2. `levels`: This parameter determines the number and positions of the contour lines/regions. The default is 20.
3. `xflip`: This parameter defines whether the x-axis is flipped or not.
4. `yflip`: This parameter defines whether the y-axis is flipped or not.

# trisurf

This plot type converts the data into a trisurfaces. A trisurface is a type of surface plot, created by triangulation of
compact surfaces of finite number of triangles which cover the whole surface in a manner that each and every point on
the surface is in triangle.

The expected data are 3 columns. The first columns represents x, the second y and the last the z data.

Possible parameters for the trisurface are:

1. `colormap` or `cmap`: This parameter defines the colormap to be used. The default is 44 (viridis).
2. `xflip`: This parameter defines whether the x-axis is flipped or not.
3. `yflip`: This parameter defines whether the y-axis is flipped or not.

# quiver

This plot type converts the data into a quiver plot. A quiver plot is a type of 2D plot that shows vector lines as
arrows.

The expected data are 2 matrices. The first matrix contains the information of the x-directions and the second matrix
the information of the y-directions.

Possible parameters for the quiver plot are:

1. `xflip`: This parameter defines whether the x-axis is flipped or not.
2. `yflip`: This parameter defines whether the y-axis is flipped or not.

# volume

This plot type converts the data into a volume plot. A volume plot draws a three-dimensional data set using volume
rendering. The volume data is reduced to a two-dimensional image using an emission or absorption model or by a maximum
intensity projection. After the projection the current colormap is applied to the result.

The expected data are multiple matrices. Each matrix is similar to a slice inside the volume.

Possible parameters for the volume plot are:

1. `algorithm`: This parameter defines which model to be used for the reduction of the data. The possible values
   are `emission`, `absorption` and `mip` which is the same as `maximum`.
2. `xflip`: This parameter defines whether the x-axis is flipped or not.
3. `yflip`: This parameter defines whether the y-axis is flipped or not.

# wireframe

This plot type converts the data into a wireframe plot. A wireframe plot takes a grid of values and projects it onto the
specified three-dimensional surface. The surface itself has a grid structure.

The expected data is a matrix. Each element of the matrix is displayed according to its position inside the matrix.
These elements are interpreted as values in x- and y-direction.
