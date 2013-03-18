
"""
    Copyright (C) 2013 Matteo Martelli.

    This file is part of WiFlyBot.

    WiFlyBot is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    WiFlyBot is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with WiFlyBot.  If not, see <http://www.gnu.org/licenses/>.
"""
#!/usr/bin/env python
import commands
import socket
import sys
import time
import string


#Return the rssi between the given interface and the remote MAC address
def getRssi(interface, rMAC):
	stations = commands.getoutput("iw dev "+interface+" station dump")
	#TODO: error check
	stations = stations.split()
	found = False
	for i in range(0, len(stations)):
		if(found == False and stations[i] == "Station" and stations[i+1] == rMAC):
			found = True
			
		if(found and stations[i] == "signal:" and i+1 <= len(stations)):
			return stations[i+1]
	
	#Some error has occured if it comes here		
	return False 


#some constants
MAC_ADDR_LEN = 17

if len(sys.argv) < 5:
	print "USAGE: "+sys.argv[0]+" interface remote_ip remote_port remote_MAC"
	sys.exit();

iface = sys.argv[1]
# addressing information of target
#find local IP address
local_ip = commands.getoutput("echo -n $(ifconfig '"+iface+"' | grep 'inet addr' | cut -d':' -f2 | cut -d' ' -f1)")

if local_ip == "":
	print "ERROR: Can't find the ip address associated to the given interface"
	sys.exit()
	
remote_ip = sys.argv[2]
remote_port = int(sys.argv[3])
remote_MAC = sys.argv[4]

cmdIfConfig = "ifconfig "+iface;
ifconf = commands.getoutput(cmdIfConfig)
#cmdRssi = "echo -n $(cat /proc/net/wireless | grep '"+iface+"' | cut -d'.' -f2 | sed s/' '//g)" #TODO use this for broadcom interfaces

#find MAC address
if ifconf == "":
	print "ERROR: Cant't find given interface"
	sys.exit()

idx = string.find(ifconf, "HWaddr")
if idx != -1:
	start_idx = idx+len("HWaddr")+1
	mac = ifconf[start_idx:start_idx+MAC_ADDR_LEN]
else:
	mac = "00:00:00:00:00:00"
	
while True:
	# SOCK_DGRAM specifies that this is UDP
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, 0)
	s.bind((local_ip, 0))
	rssi = getRssi(iface, remote_MAC)
	if rssi == False:
		rssi = "NAN"
	# enter the data content of the UDP packet
	#data = '*OPEN*'+rssi+'*CLOS*';
	data = "{###;"+local_ip+";"+mac+";"+rssi+"}"
	s.sendto(data, (remote_ip, remote_port))
	if __debug__:
		print "DEBUG: UDP packet sent:"+data
	time.sleep(1)
	# close the socket
	s.close()
