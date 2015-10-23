/**************************************************
08July2015

file: serial_txt.c rev1 by Dan Lindamood

purpose: Capture serial text from serial port and write to file.

Complile with rs232.c and rs232.h.

Example:
gcc serial_txt.c rs232.c serial_txt


 
exit the program by pressing Ctrl-C

**************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> /* close */
#include <errno.h>
#include <sys/fcntl.h>
#include <stdlib.h>
#include <time.h>

#include "rs232.h"



#define BUFFER_LEN      1024
#define LOG_FILE        "/var/www/active_log.txt"
#define MAX_MSG 100


int main (int argc, char *argv[]) {

  FILE * pFile;
  int first_active = 0,
      print_once = 0,
      n = 0,
      timeout = 0,
      cport_nr = 0,        /* /dev/ttyM0 (COM1 on MOXA-8100) */
      bdrate=9600,       /* 9600 baud */
      interval = atoi(argv[1]);
      unsigned char buf[4096];
      char print_time[48];  //used for providing time for logging process displayed on web page


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
printf("\nSerial Capture (serial) application Started  %s\n\nVersion 0 2014-JUNE-6 By -DL-\n\n\n",  print_time);//send to Status Log


        /* Loop forever*/
        while(1){

                /* Checking serial port activity */
                n = RS232_PollComport(cport_nr, buf, 4095); /* Check Serial Port activity */


                /* If serial port active after being dormant */
                if(n > 0){

			timeout = 0;/*reset timout timer*/

                        /*Send serial data to file and TCP socket if TCP socket is open*/
                        buf[n] = 0;   /* always put a "null" at the end of a string! */

                        if (argc > 2) printf("%s\n", (char *)buf); //Send To screen for monitoring
                        pFile = fopen (LOG_FILE , "a");
                        fprintf(pFile,(char *)buf);
                        fclose (pFile);
                        if (first_active == 0) first_active++ ;//Provide messages after first serial activity
                }
                else if ((timeout < (interval *10)) && (first_active == 1)) {

                        if ((timeout == 0) && (print_once == 0)){
                                time_t t = time(NULL); //used for providing time for logging process displayed on web page
                                struct tm tm = *localtime(&t); //used for providing time for logging process displayed on web page
                                sprintf(print_time,"%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                                printf("Serial Port Active  %s\n", print_time);
                                print_once = 1;
                        }
                        timeout++;
                        if (timeout == (interval *10)){
                                print_once = 0;
                                time_t t = time(NULL); //used for providing time for logging process displayed on web page
                                struct tm tm = *localtime(&t); //used for providing time for logging process displayed on web page
                                sprintf(print_time,"%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                                printf("Serial Port Idle%s\n\n\n", print_time); //Print to status lo
                }

			}

        fflush(stdout);//clear stdout periodically. This is for the proper function of writing to the process log.
        usleep(100000);  /* sleep for 100 milliSeconds */

        }//End bracket for time interval

  return(0);
}
