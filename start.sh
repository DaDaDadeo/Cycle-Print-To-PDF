#!/bin/bash

# Start Processes
# 01JUL2014 -DL-
# Version 3 

# Both command lines start with nohup. This runs the applications in the 
# backgroung and sends the terminal information to the log files that are
# assigned in the last part of the command line after the & sign.
#
# The command lines provide an array of arguments immediately following the
# application name. Each argument is seperated by a single space. All texts within
# the quotation marks are included in arguments.



# serial_tcp to write serial data to active_log.txt and tcp socket
# check communication to the printer. If printer disconnected, 
# serial data is not written to TCP socket but the log file is still active.


# Record date and time at bootup for status log file names.

logdate="$(date +'%Y%m%d%H%M')"
PRINTER_IP=192.168.0.108
PORT=9100
TIMEOUT=90


nohup /var/www/apps/sertcp/sertcp $PRINTER_IP $PORT $TIMEOUT &> /var/www/status/${logdate}_Process_tcp.txt &



INTERVAL=10
CYCLENUM="[ Cycle "
CONFIRM="TIME    "
ENDCYCLE="End of Cycle"
LINES=62
REBOOT=10
MAX=3
AUTHOR='West Pharma'
A1='PRESSURE TIMEOUT'
A2='VACUUM TIMEOUT'
A3='LOW AIR SUPPLY'
A4='LOW WATER SUPPLY'
A5='LOW CLEAN STEAM SUPPLY'
A6='LOW INSTRUMENT AIR SUPPLY'
A7='VACUUM PUMP FAILURE'
A8='DOOR FAILURE'
A9='DOOR SEAL FAILURE'
A10='CPU POWER FAILURE'
A11='EMERGENCY STOP'
A12='LEAK RATE TEST FAILURE'
A13='MASSIVE LEAK'
A14='HEAT TIMEOUT'
A15='STERILIZATION TIMEOUT'
A16='COOLING TIMEOUT'
A17='LOW TEMPERATURE'
A18='HIGH TEMPERATURE'
A19='HIGH JACKET TEMPERATURE'
A20='HIGH CHAMBER PRESSURE'
A21='LOW JACKET TEMPERATURE'
A22='RUPTURE DISC'
A23='HIGH CONDENSATE LEVEL'
A24='MAIN POWER FAILURE'
A25='DOOR INTERLOCK'
A26='DOOR SEAL INTERLOCK'
A27='PRESSURE INTERLOCK'
A28='PROCESS STOP'
A29='FAN FAILURE'
A30='FAN SEAL FAILURE'
A31='BIG HEAT EXCHANGER LEAK'
A32='SMALL HEAT EXCHANGER LEAK'
A33='AI FAILURE - CHAMBER TEMP'
A34='AI FAILURE - JACKET TEMP'
A35='AI FAILURE - CHAMBER DRAIN TEMP'
A36='AI FAILURE - JACKET DRAIN TEMP'
A37='AI FAILURE - FILTER F7 TEMP'
A38='AI FAILURE - FILTER F6 TEMP'
A39='AI FAILURE - AIR FILTER TEMP'
A40='AI FAILURE - CHAMBER PRESSURE'
A41='AI FAILURE - LOAD TEMP TE12'
A42='AI FAILURE - LOAD TEMP TE13'



nohup /var/www/apps/getcycle $INTERVAL \
"$CYCLENUM" "$CONFIRM" "$ENDCYCLE" $LINES $REBOOT $MAX \
"$A1" "$A2" "$A3" "$A4" "$A5" "$A6" "$A7" "$A8" "$A9" "$A10" \
"$A11" "$A12" "$A13" "$A14" "$A15" "$A16" "$A17" "$A18" "$A19" "$A20" \
"$A21" "$A22" "$A23" "$A24" "$A25" "$A26" "$A27" "$A28" "$A29" "$A30" \
"$A31" "$A32" "$A33" "$A34" "$A35" "$A36" "$A37" "$A38" "$A39" "$A40" \
"$A41" "$A42" \
"$AUTHOR" \
&>/var/www/status/${logdate}_Process_log.txt &




