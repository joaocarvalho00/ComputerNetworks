#!/bin/bash 

ifconfig eth0 up
ifconfig eth0 172.16.21.1/24
route add default gw 172.16.21.254
route add -net 172.16.21.0/24 gw 0.0.0.0

echo 0 > /proc/sys/net/ipv4/ip_forward
echo 1 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts

echo 1 > /proc/sys/net/ipv4/conf/eth0/accept_redirects
echo 1 > /proc/sys/net/ipv4/conf/all/accept_redirects
