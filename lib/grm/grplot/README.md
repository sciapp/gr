# gr-plots

## Introduction

This program allows to create plots from console line while using simple key-value parameters. These key-value
parameters are converted into GRM container which represents the plot.

## Console parameters

The following parameters are key:value pairs which can be used for every plot type.

1. file: contains the data which should be displayed. When no file is referred this results in an error message. When
   this parameter stands at the beginning the key is not needed.
2. kind: defines the plot which should be displayed. Possible options are `barplot`, `contour`, `contourf`, `heatmap`
   , `hexbin`, `hist`, `imshow`, `isosurface`, `line`, `marginalheatmap`, `polar`, `polar_histogram`, `pie`, `plot3`
   , `scatter`, `scatter3`, `shade`, `surface`, `stem`, `step`, `tricont`, `trisurf`, `quiver`, `volume`, `wireframe`.
   The default is
   line.

For plots where multiple columns are read there is also a parameter which allows to select columns.

1. columns: define the columns of the file which should be respected in the plot. The default is all columns. When all
   columns from x to y should be drawn use `x:y`. To select more than 1 specific column use the ',' without
   whitespace as separator.

There are more key-valid parameters. These parameter only effect specific plot types. For example `bar_width` makes only
sense, when bars are drawn.

`accelerate`, `algorithm`, `bar_color`, `bar_width`, `bin_edges`, `c`, `colormap`, `draw_edges`, `edge_color`,
`edge_width`, `grplot`, `isovalue`, `keep_aspect_ratio`, `kind`, `levels`, `marginalheatmap_kind`, `markertype`,
`nbins`, `normalization`, `orientation`, `phiflip`, `scatterz`, `spec`, `stairs`, `step_where`, `style`, `xbins`,
`xcolormap`, `xflip`, `xform`, `xticklabels`, `ybins`, `ycolormap`, `yflip`, `ylabels`

All parameters are separated through 1 whitespace.
Some parameters are more complex than others. These parameters represent a container inside GRM.

The valid container parameter are `error`, `ind_bar_color`, `ind_edge_color`, `ind_edge_width`.
Each parameter container can contain one or more of the following parameters:

1. downwardscap_color: only available for `error`
2. errorbar_color: only available for `error`
3. indices: only available for `ind_bar_color`, `ind_edge_color`, `ind_edge_width`
4. rgb: only available for `ind_bar_color`, `ind_edge_color`, `ind_edge_width`
5. upwardscap_color: only available for `error`
6. width: only available for `ind_bar_color`, `ind_edge_color`, `ind_edge_width`
   Container parameter follow a different syntax then normal parameter. The parameters inside the container are enclosed
   by `{` `}`. When more than one parameter is given to the container each of the key-value pairs is enclosed by `{` `}`
   while the parameters are seperated by `,` without whitespaces.

An example for the parameters on the commandline:

```shell
file:covid19.csv kind:line columns:1:3,5
```

This is an advanced example for a console input line, where container parameters are set:

```shell
../../../../lib/grm/grplot/data/test.dat kind:barplot ind_edge_width:2,{{indices:2},{width:5.0},{indices:1},{width:8.0}} error:{errorbar_color:4}
```

## CSV file

These files contain the data that should be plotted. Besides the data these files can include parameter which modifies
the plot.
The first lines define parameters just like the `title`. For that the lines have to follow the pattern:

```text
# key : value
```

Valid keys are:

1. title: sets the value as title of the plot
2. xlabel, ylabel, zlabel: set the value as label for the specific axis
3. resample_method: defines how the data is resampled when needed
4. location:
5. xlog, ylog, zlog: defines is the specific axis is plotted logarithmic
6. xgrid, ygrid, zgrid: defines the grid for the specific axis
7. philim, rlim: defines which part of the specific polar axis should be displayed
8. xlim, ylim, zlim: defines which part of the specific axis should be displayed
9. xrange, yrange, zrange: defines the range of the values on the specific axis
   Values are seperated through `,`. This means `3, 5` for example.

The next line after the header is treated as a line full of labels. When the data has no labels use a blank line
instead. Now the data can be added with a tabulator (`\t`) as separator for the columns.
Depending on the plot type the data is also interpreted differently. The following list shows how the data is treaten by
the different plot types.

1. `contour`, `contourf`, `heatmap`, `imshow`, `marginalheatmap`, `surface`, `wireframe`: The expected data is a matrix.
   Each element of the matrix is displayed according to its position inside the matrix.
2. `line`, `scatter`: The expected data are columns. Each column will be interpreted as a single plot. Multiple columns
   leads to multiple plots.
3. `isosurface`, `volume`: The expected data are multiple matrices. Each matrix is similar to a slice inside the volume.
4. `plot3`, `scatter`, `scatter3`, `tricont`, `trisurf`: The expected data are 3 columns. The first columns represents
   x, the second y and the last the z data.
5. `barplot`, `hist`, `stem`, `step`: The expected data is 1 column. This column represents the y data.
6. `pie`: The expected data are 1-4 lines. The first line represents the data which should be displayed. The next 3 rows
   are used to set thr rgb of the pie parts. Each row stands for one rgb part.
7. `polar_histogram`: The expected data is 1 column. The data represents theta.
8. `polar`: The expected data are 2 columns.
9. `quiver`: The expected data are 2 matrices. The first matrix contains the information of the x-directions and the
   second matrix the information of the y-directons.
10. `hexbin`, `shade`: The expected data are 2 columns. The first column contains the x and the second the y data.
