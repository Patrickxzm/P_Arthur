#!/bin/sed -f
s/^\([^ :]*\) /\1:80 /g
s/^\([^ ]*\):\([0-9]*\) \(.*\)/INSERT INTO candidate(host, port, chk, reason) VALUES('\1', '\2', 'fail','\3') ON DUPLICATE KEY UPDATE chk='fail', reason='\3';/g
