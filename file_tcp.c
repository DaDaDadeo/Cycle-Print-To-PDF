/**************************************************
21OCT2015

file: file_tcp.c rev1 by Dan Lindamood
purpose: Read file and forward to tcp port for printer 9100 telnet.



V1 Initial Release

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


//////////////Get Time/////////////////////////////////////////////////////////////////////////////////
int print_time()
  {
    char *tz;

   time_t rawtime;
   struct tm *localtime;
   time_t offset;

    setenv( "TZ", "EST5EDT", 1 );
    tzset();


   time(&rawtime);
   /* Get GMT time */
   offset = (rawtime - timezone + (daylight*3600));
   localtime = gmtime(&offset);
   printf("%02d/%02d/%02d %2d:%02d:%02d",localtime->tm_year+1900, localtime->tm_mon+1, localtime->tm_mday,  localtime->tm_hour, localtime->tm_min, localtime->tm_sec);

   return(0);
  }

int main (int argc, char *argv[]) {

  FILE * pFile;
  int comOK = 1 ,
      n = 0;
      char ping[48];
      unsigned char str[512];

////////////CLIENT///////////
  int sd, rc;
  struct sockaddr_in localAddr, servAddr;
  struct hostent *h;



/* Determine If enough parameters were added */

	if(argc < 4) {
		printf("\n\n---  usage: %s <Server Address> <Port Number> <File Path>\n--- Example ./file_tcp 192.168.0.85 9100 /var/www/pcl/cycle_1234.pcl\n\n\n", argv[0]);
		exit(0);
	}

	sprintf(ping, "ping -c1 -w1 %s >/dev/null 2>&1", argv[1]);//prepare system command for comm check

	//Printed to nohup.out (processTCP.txt) in web page
	/* opening file for reading */
	pFile = fopen(argv[3] , "r");
   
	///////////// VERIFY IF FILE EXISTS //////////////////////////////
	if(pFile == NULL) {
      perror("Error opening file");
      return(-1);
	  comOK = 0;
	  exit(0);
	}
    ////////////  OPEN AND READ FILE /////////////////////////////////////
	else{
	
		///////////////// CHECK ETHERNET COMMUNICATIONS //////////////////
		if (system(ping) != 0){
			comOK = 0;
			printf("\nPrinter Disconnected\n\nETHERNET PRINTING FAILED\n");
		}
		///////////////// OPEN TCP COMMUNICATIONS //////////////////
		else {

			h = gethostbyname(argv[1]);/*printer IP address*/

			if(h==NULL) {
				printf("\n%s: unknown host '%s'\n\nETHERNET PRINTING FAILED\n",argv[0],argv[1]);
				comOK = 0 ;
				exit(0);
			}
		
			servAddr.sin_family = h->h_addrtype;
			memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
			servAddr.sin_port = htons(atoi(argv[2]));

			/* create socket */
			sd = socket(AF_INET, SOCK_STREAM, 0);

			if(sd<0) {
				perror("\nPrinter Disconnected (1)\n\nETHERNET PRINTING FAILED\n");
				comOK = 0 ;
				exit(0);
			}

			/* bind any port number */
			localAddr.sin_family = AF_INET;
			localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
			localAddr.sin_port = htons(0);

			rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));

			if(rc<0) {
				printf("%s: cannot bind port TCP %u\n\nETHERNET PRINTING FAILED\n",argv[0],atoi(argv[2]));
				perror("error ");//for testing
				comOK = 0;
				exit(0);
			}

			/* connect to server */
			rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
			if(rc<0) {
				perror("\nPrinter Disconnected (2)\n\nETHERNET PRINTING FAILED\n");
				comOK = 0;
				exit(0);
			}

			while( fgets (str, 512, pFile)!=NULL ) {
			/* writing content to stdout */
				n = strlen(str);
				str[n] = '\0';
 
				if (comOK == 1){
					rc = send(sd,str,n,0);
					if(rc<0) {
						printf("\ncannot send data to printer/server\n\nETHERNET PRINTING FAILED\n");
						close(sd);
						comOK = 0;
						exit(0);
					} 
				}
//			printf("%s", str); // For testing
			usleep(100000);  /* sleep for 100 milliSeconds */
			}
		}
	
	fclose(pFile);
	}
	if (comOK == 1) printf("ETHERNET PRINTING COMPLETED\n");
	fflush(stdout);//clear stdout periodically. This is for the proper function of writing to the process log.

  return(0);
}


