BATCH-CAPTURE
========

Serial Print Cycle and Batch Report Capture

Batch-Capture DESCRIPTION

The Batch-Capture device server receives serial print data, stores and forwards the data to an
Ethernet capable printer. Using the least invasive approach by connecting only to the print port,
Batch-Capture is designed to store the batch reports with little or no changes to the equipment providing the print data. After a proper configuration, Batch-Capture is “plug and play” and will run automatically on power up
independent of the equipment controls.

The stored batch reports are archived in both secured Acrobat PDF and raw PCL text formats. At the end of a
cycle, the text data from the serial port is “printed” (converted) to PDF and also stored as a PCL
text file. The PDF files will include the original color format and fonts. Printer tests, calibration
reports, and other print functions not related to a cycle batch report are ignored. The batch reports
are stored on the internal SD card. The available memory capacity is over 4GB.

The Batch-Capture device is a web and network server. The stored files can be accessed using a
web browser and by mapping the network location using Windows.

Optional: If DataStore Plus* (available on Allen Bradley PanelView Plus panels) is activated, the
historian data can be stored as CSV files to the Batch-Capture server. The data can be viewed as a
customized graph from a JavaScript compatible web browser.




THEORY OF OPERATION

The Batch-Capture device boots up using the Raspbian “wheezy” 2012-10-28 operating system.
The 4GB operating system image is loaded on a 8GB SD card and the remaining partition is expanded to use the remaining 4GB for storage. 

During the boot up process, a shell script application startprocess.sh starts the two programs essential to the operation of the device.

The application startprocess.sh loads the two primary Batch-Capture applications, Batch-Capture and
serial_to_tcp.py. The shell script provides the opening commands for the two applications with
the required arguments (configurations). The startprocess.sh application is part of the three
programs that can be edited by the programmer.

One of the Batch-Capture functions is to receive serial data from a printer port of a controller, store the data to a file named active_log.txt and at the same time forward the data to an Ethernet printer port. The data is not filtered or changed. The application that performs this is the serial_to_tcp.py program.

The Batch-Capture application is a compiled binary program using C programming language. The
function is to periodically check the text file active_log.txt and determine whether a cycle has
started and ended. The Batch-Capture application at first only reads the text file. However, if the
received data is not from a cycle print, the text is cleared on the next periodic interval. This is to
eliminate any non-cycle prints from the Getinge controller such as calibration and test printer
functions. If the print data is verified as a cycle, the file will continue to be periodically checked
(read only) until the cycle is completed. After the cycle completes, the text file is printed and
stored as a PDF file using GhostPDL 9.06 [PCL6]. The active_log.txt text file is then
copied as a backup PCL file and then completely cleared for the next incoming data. Both the
backup PCL and PDF files are named using the captured cycle number and the subsequent file
type extensions.

Note: The date and time tags listed for each creation of the files are created using the GET
CYCLE clock and are not based on the date and time within the cycle print data.
The user has secured access to the archived files using a web browser. A user name and
password is required in order to view the site. Only the administrator Admin1 has access to the
FTP directory.

User name and Passwords are configured.

Additional passwords can be created using an “htaccces Authentication generator” available at
many internet locations ex: http://www.htaccesstools.com/htpasswd-generator/ . The results can
be copied and pasted in the .htpasswd file in the main directory of the Batch-Capture web server
location.
