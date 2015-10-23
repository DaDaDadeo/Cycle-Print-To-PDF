
# Start Processes
# 21OCT2015 -DL-
# Version 1

PRINTER_IP="192.168.0.133"
PRINTER_PORT="49500"
FILE="/var/www/active_log.txt"

/var/www/apps/file_tcp $PRINTER_IP $PRINTER_PORT $FILE 
