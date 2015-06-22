#!/bin/sh
export MPLBACKEND=gr
for f in  *.py
do
  gr -o pdf $f
  x=`echo $f | awk -F\. '{print $1}'`
  pdf2pdf -X 320 -Y 240 -p 1 gks.pdf ${x}.pdf
  rm -f gks.pdf
done
rm -f tex_demo.png
