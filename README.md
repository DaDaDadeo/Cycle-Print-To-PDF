Cycle-Print-To-PDF
========

Serial Print Cycle and Batch Report Capture

DESCRIPTION

The Cycle-Print-To-PDF device server receives serial or tcp socket print data, stores and forwards the data to an
Ethernet capable printer. Using the least invasive approach by connecting only to the print port,
Cycle-Print-To-PDF is designed to store the batch reports with little or no changes to the equipment providing the print data. After a proper configuration, Cycle-Print-To-PDF is “plug and play” and will run automatically on power up independent of the equipment controls.

The stored batch reports are archived in both secured Acrobat PDF and raw PCL text formats. At the end of a
cycle, the text data from the serial port is “printed” (converted) to PDF and also stored as a PCL
text file. The PDF files will include the original color format and fonts. Printer tests, calibration
reports, and other print functions not related to a cycle batch report are ignored. The batch reports
are stored on the internal SD card. The available memory capacity is over 4GB.

The Cycle-Print-To-PDF device is a web and network server. The stored files can be accessed using a
web browser and by mapping the network location using Windows.

Optional: If DataStore Plus (available on Allen Bradley PanelView Plus panels) is activated, the
historian data can be stored as CSV files to the Cycle-Print-To-PDF server. The data can be viewed as a
customized graph from a JavaScript compatible web browser.




THEORY OF OPERATION

The Cycle-Print-To-PDF runs within a compact embedded Linux computer.During the boot up process, a shell script application startprocess.sh starts the two programs essential to the operation of the device.

The shell script loads the two primary Cycle-Print-To-PDF applications, Cycle-Print-To-PDF and sertcp (or servprint). The shell script provides the opening commands for the two applications with the required arguments.

The Cycle-Print-To-PDF can function as a device server and receive serial or tcp socket data from an automation PLC, store the data to a file named active_log.txt and at the same time forward the data to an Ethernet printer port. 

The Cycle-Print-To-PDF can also be configured to run without forwarding the data to an Ethener port until the logged information or cycle is completed. This allows for a printer or server port to be used only for a short period.

The data stored to the text file is not filtered or changed. There are several applications that perform different functions depending on the options used. 

- If serial data is to be captured and forwarded to a printer, the serial_tcp or serial_to_tcp.py applications can be used. 
  
- If serial data is to be captured only, the serial_txt applciation can be used. The serial_to_tcp application can be used also by configuring the IP address to a non-exisisting address.
  
- If TCP data is captured and forwarded to a printer, the serv_print application is used.
	
- If TCP data is captured only, the application serv_txt is used.
	
- The cycle-to-pdf-19 will only print to PDF.
	
- The cycle-to-pdf-20 program will print both the PDF file of cycles only and all information that is stored to the log file. The log file text is sent to the printer using the file_tcp application.

The cycle-to-pdf application is a compiled binary program using C programming language. The
function is to periodically check the text file active_log.txt and determine whether a cycle has
started and ended. The Get-Cycle application at first only reads the text file. However, if the
received data is not from a cycle print, the text is cleared on the next periodic interval. This is to
eliminate any non-cycle prints from the controller such as calibration and test printer
functions. If the print data is verified as a cycle, the file will continue to be periodically checked
(read only) until the cycle is completed. After the cycle completes, the text file is printed and
stored as a PDF file using a compiled C program using the libharu free PDF libraries (https://github.com/libharu/libharu/). The active_log.txt text file is then copied as a backup PCL file and then completely cleared for the next incoming data. Both the
backup PCL and PDF files are named using the captured cycle number and the subsequent file
type extensions.



