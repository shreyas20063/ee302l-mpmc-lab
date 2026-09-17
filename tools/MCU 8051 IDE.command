#!/bin/bash
docker info >/dev/null 2>&1 || { open -a Docker; until docker info >/dev/null 2>&1; do sleep 2; done; }
docker start mcu8051ide >/dev/null
until curl -s -o /dev/null http://localhost:6080/; do sleep 1; done
sleep 3
docker exec mcu8051ide pgrep -f main.tcl >/dev/null || docker exec -d -u ubuntu -e DISPLAY=:1 -e HOME=/home/ubuntu -w /home/ubuntu/lab mcu8051ide mcu8051ide
open "http://localhost:6080/vnc.html?autoconnect=true&resize=remote"
