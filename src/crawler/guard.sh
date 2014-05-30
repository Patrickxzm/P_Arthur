#!/bin/sh

clear; date; 
echo "=============================="; 
echo "show Processlist;" \
     "select '========================================';" \
     "select host.crawler, count(*) as count, ProcessQuota from host left join crawler on crawler.name=host.crawler where host.crawler is not null group by crawler;" \
     "select '=======================';" \
     "select count(*) as crawlDelayed from host where schedule_time<NOW();" \
     "select count(*) as crawlScheduled from host where schedule_time is not null;" \
     "SELECT name AS Oldest, port, schedule_time, crawler, load_time FROM host WHERE schedule_time IS NOT NULL ORDER BY schedule_time limit 5;" | \
crawler.mysql; 


