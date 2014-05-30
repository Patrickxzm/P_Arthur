#!/bin/sed -f
s/^\([^ :]*\) /\1:80 /g
s/^\([^ ]*\):\([0-9]*\) \(.*\)/INSERT INTO candidate(host, port, chk, ipaddr) values('\1', '\2', 'OK','\3') ON DUPLICATE KEY UPDATE chk='OK', ipaddr='\3';/g
