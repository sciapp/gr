# gr-plots

## Introduction

This small program allows to create plots from console line while using 1-3 parameters.

## Console parameters

The following parameters are key:value pairs which defines the displayed plot.

1. file: contains the data which should be displayed. When no file is referred this results in an error message.
2. kind: defines the plot which should be displayed. Possible options are `line`, `heatmap` or `marginalheatmap` which is a heatmap with histograms on the side. The default is
   line.
3. columns: define the columns of the file which should be respected in the plot. The default is all columns. When all
   columns from x to y should be drawn use `x:y`.

The three parameters are separated through 1 whitespace. To select more than 1 specific column use the ',' without
whitespace as separator.

An example for the parameters on the commandline:

```shell
file:covid19.csv kind:line columns:1:3,5
```

## CSV file

First lines define parameters just like the title or the labels. For that the lines have to follow the pattern:

```text
# key : value
```

Values are seperated through `,`. This means `3, 5` for example.

The next line after the header is treated as a line full of labels. When the data has no labels use a blank line
instead. Now the data can be added with a tabulator (`\t`) as separator for the columns.
