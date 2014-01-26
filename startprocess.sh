#!/bin/bash

# Start Processes 
# 01Apr2013 -DL-
# Version 0 

# Both command lines start with nohup. This runs the applications in the 
# backgroung and sends the terminal information to the log files that are
# assigned in the last part of the command line after the & sign.
#
# The command lines provide an array of arguments immediately following the
# application name. Each argument is seperated by a single space. All texts within
# the quotation marks are included in arguments.



# serial_to_tcp.py version from GITHUB.com. Modified to write socket data to active_log.txt and check communication
# to the printer. If printer disconnected, serial data is not written to TCP socket but the log file is still active.
# Printer IP address and port number can be modified here: 

# Record date and time at bootup for status log file names.
logdate="$(date +'%Y%m%d%H%M')"

#                                               IP ADRESS       PORT
nohup /var/www/applications/serial_to_tcp.py -H 192.168.0.127 -P 9100 &> /var/www/status/${logdate}_Process_tcp.txt &


# GETCYCLE 
#
# Different models can be selected by removing the remark from the line associated
# with the model.
#
# CycNum
# Locate is the unique string that locates the cycle number
#
# st
# pt is the location at which the first cycle number digit appears in the string
#
# Confirm Cycle string is the string of characters that are unique to a cycle print.
#
# End Of Cycly is the unique string that determines the end of a cycle.
#
# Lines Per page is the line count that the page can handle before a new page is printed.


#				       CycNum      st					   lines
#				       locate      pt  Confirm Cycle    End of Cycle   per page
#
#CL AB Controller SWE 80 Col.
nohup /var/www/applications/getcycle 10 "[ Cycle" 62 "TIME         " "End of Cycle" 62 &>/var/www/status/${logdate}_Process_log.txt &
#
# EX: ======================================================[ Cycle 1199 Page 1 ]=
#     TIME           PE3       TE2   TE1   TE0       LP1  LP2   LP3   LP4
#     End of Cycle. Process OK


