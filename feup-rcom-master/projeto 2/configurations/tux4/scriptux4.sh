#!/bin/bash

ifconfig eth0 up
ifconfig eth1 up
ifconfig eth0 172.16.20.254/24
ifconfig eth1 172.16.21.253/24

route add default gw 172.16.21.254
route add -net 172.16.0.0/16 gw 0.0.0.0
route add -net 172.16.20.0/24 gw 0.0.0.0

echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
