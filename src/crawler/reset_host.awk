#!/bin/awk -f
{ 
  print "UPDATE host SET sum_page_num=sum_page_num+page_num, page_num=0, zip_root=NULL"
  print "    , refresh_num=0, rotate_num=rotate_num+1, schedule_time=NOW()"
  print "    WHERE name='" $1 "' and port=" $2 ";"
  print "DELETE FROM memory WHERE hostID=(SELECT ID FROM host"
  print "    WHERE name='" $1 "' and port=" $2 ");"
}

