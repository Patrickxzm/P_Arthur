#!/bin/sed -f
s/^\([^ :]*\)\t/\1:80\t/g
s/^\([^ ]*\):\([0-9]*\)\t\(.*\)/UPDATE candidate SET inlinks=inlinks+\3 WHERE host='\1' AND port=\2;/g
