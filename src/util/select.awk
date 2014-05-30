#!/usr/bin/awk -f
# Example: select.awk -v first=34 -v last=1000 
#
BEGIN { line = 0 }
{
  line += 1
  if (line >= first && (last==0 || line <= last)) print
}
