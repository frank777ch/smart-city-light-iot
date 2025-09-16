# smart-city-light-iot

##_red

sudo ip addr flush dev wlo1
sudo ip addr add 192.168.101.210/24 dev wlo1
sudo ip route add default via 192.168.101.1
sudo bash -c 'echo -e "nameserver 8.8.8.8\nnameserver 8.8.4.4" > /etc/resolv.conf'

docker compose up -d 
docker ps
docker compose down

## Arduino IDE

ESP32

Preferencias

https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

Gestor de tarjetas

esp32 by Espressif Systems

Gestor de librerias

PubSubClient - PubSubClient by Nick O'Leary

## Node Red

http://localhost:1880/
