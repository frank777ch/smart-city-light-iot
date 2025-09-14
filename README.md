# smart-city-light-iot

##_red
sudo ip addr flush dev wlo1
sudo ip addr add 192.168.101.210/24 dev wlo1
sudo ip route add default via 192.168.101.1
sudo bash -c 'echo -e "nameserver 8.8.8.8\nnameserver 8.8.4.4" > /etc/resolv.conf'