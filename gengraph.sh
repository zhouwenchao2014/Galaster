#!/bin/sh

rm -f *.pdf

for file in *.dot
do
	outfile=`echo $file | sed s/\.dot$/\.pdf/g`
	echo "Generating $outfile ..."
	fdp -Tpdf -o $outfile $file
done
