/**************************************************
12May2014

file: ser_print.c by Dan Lindamood
remix from http://www.teuniz.net/RS-232/


Scope: Capture serial text from serial port and also
forward to tcp port for printer 9100 telnet.

Details: When the serial port is active, the client socket is opened. 
If there is no printer, ignore the tcp socket and continue capturing
the serial to a text file. If the printer is connected, forward to 
the printer port 9100. When the serial port is active and the tcp
socket is still open, continue checking communications 
to the printer by pinging every 5 seconds. If communications are lost,
the tcp client socket is closed and the capture to file continues until
after the serial port is inactive and the configured timeout is done. 

Save this file along with rs232.c and rs232.h to the same directory.
compile with the rs232.c file. 
Ex: gcc ser_print.c rs232.c -o ser_print

Add 2 arguments for starting.
         App          Printer IP   Timeout
example: ./ser_print 192.168.0.100 90



exit the program by pressing Ctrl-C

**************************************************/
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> /* close */
#include <errno.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <time.h>

#include "rs232.h"



#define BUFFER_LEN	1024
#define LOG_FILE	"/var/www/active_log.txt"
#define SERVER_PORT 9100
#define MAX_MSG 100


int main (int argc, char *argv[]) {

  FILE * pFile;
  int check_com = 0,
      first_active = 0,
      print_once = 0,
      comOK = 0 ,
      ignoreTCP = 0,
      n = 0,
      timeout = 0,
      cport_nr=2,        /* /dev/ttyS0 (COM1 on windows) */
      bdrate=9600,       /* 9600 baud */
      interval = atoi(argv[2]);
  char print_time[48];  //used for providing time for logging process displayed on web page
  char ping[48];


////////////CLIENT///////////
  int sd, rc;
  struct sockaddr_in localAddr, servAddr;
  struct hostent *h;

  unsigned char buf[4096];


/* Determine If enough parameters were added */

  if(argc < 3) {
    printf("usage: %s <printer address> <Time Interval>\nAdd extra non-descript arguments for:\n3rd Show First Scan Tests\n4th Show Port Errors\n5th Show Serial Data\n", argv[0]);
    return(0);
  }


 sprintf(ping, "ping -c1 %s >/dev/null 2>&1", argv[1]);//prepare system command for comm check

	/*Verify Serial Port Available. Close Application if not*/
  	if(RS232_OpenComport(cport_nr, bdrate)){
    		printf("Can not open comport\n");
    		return(0);
  	}

time_t t = time(NULL); //used for providing time for logging process displayed on web page
struct tm tm = *localtime(&t); //used for providing time for logging process displayed on web page

//used for providing time for logging process displayed on web page
sprintf(print_time,"%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);// display on web page log
//Printed to nohup.out (processTCP.txt) in web page
printf("\nSerial Capture/Server (sertcp) application Started  %s\nClose TCP port [%s] after %s seconds of innactivity. \n\nVersion 0 2014-JUNE-6 By -DL-\n\n\n",  print_time,argv[1], argv[2]);//send to Status Log


	/* Loop forever*/
	while(1){

		/* Checking serial port activity */
		n = RS232_PollComport(cport_nr, buf, 4095); /* Check Serial Port activity */


		/* If serial port active after being dormant */
		if(n > 0){
			if ((ignoreTCP == 0) && (comOK == 0)) {

				timeout = 0;/*reset timout timer*/

				h = gethostbyname(argv[1]);/*printer IP address*/

				if(h==NULL) {
					if (argc > 4) printf("%s: unknown host '%s'\n",argv[0],argv[1]);
					comOK = 0 ;
					ignoreTCP = 1;
				}
				servAddr.sin_family = h->h_addrtype;
				memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
				servAddr.sin_port = htons(SERVER_PORT);

				/* create socket */
				sd = socket(AF_INET, SOCK_STREAM, 0);
				if(sd<0) {
					if (argc > 4) perror("Printer Disconnected (1)\n ");
					comOK = 0 ;
					ignoreTCP = 1;
				}

				/* bind any port number */
				localAddr.sin_family = AF_INET;
				localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
				localAddr.sin_port = htons(0);

				rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));

				if(rc<0) {
					if (argc > 4){ //add 4th argument to test port 
						printf("%s: cannot bind port TCP %u\n",argv[0],SERVER_PORT);
						perror("error ");
					}
					comOK = 0;
					ignoreTCP = 1;
				}

				/* connect to server */
				rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
				if(rc<0) {
					if (argc > 4) perror("Printer Disconnected (2)\n ");
					comOK = 0;
					ignoreTCP = 1;
				}
				else comOK = 1 ;
			}

		/*Send serial data to file and TCP socket if TCP socket is open*/
			check_com = 0;
			timeout = 0; /*reset Timeout timer */
			buf[n] = 0;   /* always put a "null" at the end of a string! */
			if (argc > 5) printf("%s\n", (char *)buf); //Send To screen for monitoring
			pFile = fopen (LOG_FILE , "a");
			fprintf(pFile,(char *)buf);
			fclose (pFile);
        	         ////////////////WRITE TO SOCKET/////////////////////
			if (ignoreTCP == 0){
				rc = send(sd,(char *)buf,n,0);
    				if(rc<0) {
					printf("cannot send data ");
					close(sd);
					ignoreTCP = 1;
				}
			}
		if (first_active == 0) first_active++ ;//Provide messages after first serial activity		
		}
			/*Close socket after configured (2nd Parameter) time
			when serial port is inactive*/
		else if ((timeout < (interval *10)) && (first_active == 1)) {

			if ((timeout == 0) && (print_once == 0)){
				time_t t = time(NULL); //used for providing time for logging process displayed on web page
                                struct tm tm = *localtime(&t); //used for providing time for logging process displayed on web page
                                sprintf(print_time,"%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
				printf("Serial Port Active  %s\n", print_time);
				print_once = 1; 
			}
                        timeout++;
                        check_com++;
                        if (check_com == 50 ){
				if (ignoreTCP == 0){
					if (system(ping) != 0){
						ignoreTCP = 1;
                                	        printf("Printer Disconnected During Active Cycle\n");
						close(sd);
					}
				}
				check_com = 0;
                                if (argc > 4) printf("check_com Timer Elapsed\n");//For Testing Only
			}
			if (timeout == (interval *10)){
                               	print_once = 0;
				close(sd);
				time_t t = time(NULL); //used for providing time for logging process displayed on web page
                                struct tm tm = *localtime(&t); //used for providing time for logging process displayed on web page
				sprintf(print_time,"%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
				if (comOK == 1) printf("Serial Port Idle / TCP Socket Closed  %s\n\n\n", print_time); //Print to status log
				else printf("Serial Port Idle / Printer Disconnected  %s\n\n\n", print_time);
				if (argc > 3) printf("%s  Second Start Timer Elapsed\n", argv[2]);//For Testing Only
				ignoreTCP = 0;
				comOK = 0;
			}
		
		}

	fflush(stdout);//clear stdout periodically. This is for the proper function of writing to the process log.
	usleep(100000);  /* sleep for 100 milliSeconds */

	}

  return(0);
}

