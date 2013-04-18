
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
from optparse import OptionParser


#Return the rssi between the given interface and the remote MAC address
def getRssi(interface, rMAC, useiw):
	
	if (useiw):
		stations = commands.getoutput("iw dev "+interface+" station dump")
		#TODO: error check (use directly MAC search from iw command. See man iw)
		stations = stations.split()
		found = False
		for i in range(0, len(stations)):
			if(found == False and stations[i] == "Station" and stations[i+1] == rMAC):
				found = True
			
		if(found and stations[i] == "signal:" and i+1 <= len(stations)):
			return stations[i+1]
	else:
	 	rssi = commands.getoutput("echo -n $(cat /proc/net/wireless | grep '"+iface+"' | cut -d'.' -f2 | sed s/' '//g)") #TODO use this for broadcom interfaces
		if (rssi != ""):
			return rssi
				
	#Some error has occured if it comes here		
	return False 


#Convert the ip string in a hex format string
def formatIP(ip):
	ipArray = ip.split('.')
	ipRet = ""
	for i in ipArray:
		t = hex(int(i)).replace('0x', '')
		if((len(t) % 2) != 0): #If the number of chars is odd
			t = '0'+t			 #add a 0 before the char
		ipRet = ipRet + t
	return ipRet

parser = OptionParser()
parser.add_option("--noiw",
                  action="store_false", dest="useiw", default=True,
                  help="don't use the iw command to get the rssi")
parser.add_option("-d", "--power-difect",dest="powerDifect", default="0",
                  help="add a power difect to the interface")

(options, args) = parser.parse_args()

#some constants
MAC_ADDR_LEN = 17

#TODO CONVERT OPT SELECTION WITH OptionParser
if len(sys.argv) < 5:
	print "USAGE: "+sys.argv[0]+" interface remote_ip remote_port remote_MAC [power_difect]"
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
powerDifect = int(options.powerDifect)

cmdIfConfig = "ifconfig "+iface;
ifconf = commands.getoutput(cmdIfConfig)

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

mac = mac.replace(':', '') #Remove dots separators	

while True:
	# SOCK_DGRAM specifies that this is UDP
	s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, 0)
	s.bind((local_ip, 0))
	rssi = getRssi(iface, remote_MAC, options.useiw)
	
	if rssi == False:
		rssi = "01"
		"""else:
		#Convert the rssi string to a hex format string
		rssi = hex(int(rssi.replace('-', ''))).replace('0x','')
		if((len(rssi) % 2) != 0): #If the number of chars is odd
			rssi = '0'+rssi			 #add a 0 before the char"""
	else:
		rssi = str(int(rssi)-powerDifect)	

	# enter the data content of the UDP packet
	#data = '*OPEN*'+rssi+'*CLOS*';
	data = "###"+local_ip+","+mac+","+rssi+";"
	s.sendto(data, (remote_ip, remote_port))
	if __debug__:
		print "DEBUG: UDP packet sent:"+data
	time.sleep(1)
	# close the socket
	s.close()
