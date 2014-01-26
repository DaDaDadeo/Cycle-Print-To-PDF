#!/usr/bin/env python

# based on tcp_serial_redirect.py from pyserial
# http://pyserial.sourceforge.net/

""" (Ver 0 2014-01-21) -DL-  Added logging and ping feature. The additions write the data stream to a text file
and tests the communucation to the printer port. If communication is lost, the data tranfer to 
TCP is suspended. The communication test interval is 1 minute when the serial port is idle. 

Modified version from:
https://github.com/jaredly/pydbgp/blob/master/symbian/serial_tcp_redirect.py


USAGE: serial_tcp_redirect.py [options]
Simple Serial to Network (TCP/IP) redirector.

This redirector waits for data from the serial port, then connects
to the remote host/port and relays data back and forth.  If either
the COM port or the socket is closed, it restarts and waits for a
new connection on the COM port.

Options:
  -H, --iphost      Remote TCP/IP host (default 127.0.0.1)

Note: Only one connection at once is supported. If the connection is terminaed
it waits for the next connect.
"""

import sys, os, threading, getopt, socket, subprocess, datetime

try:
    import serial
except:
    print "Running serial_tcp_redirect requries pyserial"
    print "available at http://pyserial.sourceforge.net/"
    sys.exit(1)

try:
    True
except NameError:
    True = 1
    False = 0

class pingCommunication():
 
    def __init__(self, ipToPing):
        self.ipToPing = ipToPing
        self.pingQuantity = "1"
 
    def pingProcess(self):
        pingTest = "ping -c "+ self.pingQuantity + ' ' + self.ipToPing
        #print pingTest -DL- Unremark for testing
        process = subprocess.Popen(pingTest, shell=True, stdout=subprocess.PIPE)
        process.wait()
        returnCodeTotal = process.returncode
        return returnCodeTotal


class SerialRedirector:
    def __init__(self, ping_addr, tcp_addr):
      # print "PRINTER IP ADDRESS IS %r" % (ping_addr)
        self.ping = ping_addr
	self.addr = tcp_addr
        self.socket = None
        self.thread_write = None
        self.alive = False
        
        # create the serial connection
        ser = serial.Serial()
        ser.port    = '/dev/ttyAMA0'
        ser.baudrate = 9600
        ser.rtscts  = True
        ser.xonxoff = False
        ser.timeout = 90     #required so that the reader thread can exit
        try:
            ser.open()
        except serial.SerialException, e:
            print "Could not open serial port %s: %s" % (ser.portstr, e)
            sys.exit(1)
        self.serial = ser

    def go(self):
        """wait for incoming com data, then redirect
            to the tcp port"""
        self.alive = True
	self.reader()
    
    def socketStart(self):
        print "CONNECTING TO IDE AT %r" % self.addr
        # create the socket connection
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.addr[0], self.addr[1]))
        except socket.error, details:
            self.socket = None
	    raise
        
        # start redirecting from serial to tcpip
        self.thread_write = threading.Thread(target=self.writer)
        self.thread_write.setDaemon(1)
        self.thread_write.start()
        
    def reader(self):
        """loop forever and copy serial->socket"""
        print "Serial To TCP Application Started Using Port %s %s\n\nTCP/Printer IP Adress: %s\n\n" % (self.serial.portstr, str(datetime.datetime.now()),self.ping) 
        data = None
	comOK = 2
	data_on = 2
        while not data:
#	    print comOK # -DL- remove remark for testing
            comunicate = pingCommunication(self.ping)
            ignoreTCP = comunicate.pingProcess()
	    if ignoreTCP == 0:
		if not comOK == 1:
	    	    print "PRINTER CONNECTED  %s %s\n\n" %(self.addr, str(datetime.datetime.now()))
		comOK = 1
	    else:
		if not comOK == 0:
		    print "PRINTER DISCONNECTED %s %s\n\n" %(self.addr,  str(datetime.datetime.now()))
		comOK = 0
            data = self.serial.read(1)              #read one, blocking
            n = self.serial.inWaiting()             #look if there is more
	    sys.stdout.flush()
#        print "Received serial data on %s" % self.serial.portstr
#        print "data [%r]" %data -DL- ????
        while self.alive:
	    try:
                if n:
                    data = data + self.serial.read(n)   #and get as much as possible
                if data:
                    data = '\r\n'.join(data.split('\n'))
                    logfile = open("/var/www/active_log.txt","a")
                    logfile.write(data)
                    logfile.close()
                    #print ("%s" %data, end="")-DL- Removed for final testing
                    if not self.socket and ignoreTCP == 0:
                        self.socketStart()
		    if comOK == 1:
                    	self.socket.sendall(data)           #send it over TCP
		    if not data_on == 1:
			print "DATA RECEIVED  " + str(datetime.datetime.now()) 
                    data_on = 1
		    sys.stdout.flush()
		else:
	            comunicate = pingCommunication(self.ping)
        	    ignoreTCP = comunicate.pingProcess()
	            if ignoreTCP == 0:
        	        if not comOK == 1:
                	    print "PRINTER CONNECTED %s %s\n\n" %(self.addr,  str(datetime.datetime.now()))
               		comOK = 1
           	    else:
                	if not comOK == 0:
                    	    print "PRINTER DISCONNECTED %s %s\n\n" %(self.addr, str(datetime.datetime.now()))
                	comOK = 0
		    if data_on == 1:
                        print "SERIAL PORT IDLE  %s\n\n" % str(datetime.datetime.now())
		    data_on = 0
                data = self.serial.read(1)              #read one, blocking
                n = self.serial.inWaiting()             #look if there is more
		sys.stdout.flush()
            except socket.error, msg:
                print msg
                #probably got disconnected
                break
        self.alive = False
        self.serial.close()
        if self.thread_write:
            self.thread_write.join()
    
    def writer(self):
        """loop forever and copy socket->serial"""
        while self.alive:
            try:
                data = self.socket.recv(1024)
                if not data:
                    break
                #print "socket data [%r]" % data  -DL- removed for final testing
                self.serial.write(data)                 #get a bunch of bytes and send them
            except socket.error, msg:
                print msg
                #probably got disconnected
                break
        self.alive = False
        # close the socket
        try:
            self.socket.close()
        except socket.error, details:
            pass #we quiting, dont care about the error
        self.socket = None

    def stop(self):
        """Stop copying"""
        if self.alive:
            self.alive = False
            self.thread_write.join()

if __name__ == '__main__':
    
    #parse command line options
    try:
        opts, args = getopt.getopt(sys.argv[1:],
                "hP:H:",
                ["help","ipport=", "iphost="])
    except getopt.GetoptError:
        # print help information and exit:
        print >>sys.stderr, __doc__
        sys.exit(2)
    
    iphost = '192.168.0.158'
    ipport = 9100
    for o, a in opts:
        if o in ("-h", "--help"):   #help text
            usage()
            sys.exit()
        elif o in ("-H", "--iphost"):
            iphost = a

    print "\n---Serial to TCP/IP redirector ---\nVersion 0 04-April-2013 By -DL-\n\n"


    while 1:
        try:
            #enter console->serial loop
            r = SerialRedirector(iphost,iphost)
            r.go()
        except socket.error, msg:
            print msg

    print "\n--- exit ---"



