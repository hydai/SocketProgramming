#/bin/sh
sudo apt-get update
sudo apt-get install -y g++
sudo apt-get install -y libsqlite3-dev sqlite3

#Modify packet loss rate
#sudo  tc qdisc add dev eth0 root netem loss 90%
#sudo tc qdisc del dev eth0 root
