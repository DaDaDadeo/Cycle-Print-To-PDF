/**********************************************************
* servprint.c by Dan Lindamood III
*
* Open Server Socket and listen.
* Allow Allen Bradley PLC to send print data.
* If data is active, open client socket and send data to printer.
* Simultaneously, write all data to a text file.
* Ignore client socket if printer is disconnected and continue
* writing to file.
* Close the client socket when finished
*
*
*
*
*
*
*
**********************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h> /* close */
#include <time.h>


int main(int argc, char**argv)
{
    int sock, connected, bytes_received, true = 1, live = 0, ignoreTCP = 0;
    char recv_data[1024];
    char ping[48];

    struct sockaddr_in server_addr,client_addr;
    int sin_size;
    FILE * pFile;

	if(argc < 4){
		printf("\n\nAdd required parameters ./application [local port] [printer IP] [printer port]\nExample: ./servprint 6001 192.168.0.108 9100\n\n");
		exit(1);
	}

	char print_time[48];  //used for providing time for logging process displayed on web page
	time_t t = time(NULL); //used for providing time for logging process displayed on web page
	struct tm tm = *localtime(&t); //used for providing time for logging process displayed on web page

	//used for providing time for logging process displayed on web page
	sprintf(print_time,"%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);// display on web page log
	//Printed to nohup.out (processTCP.txt) in web page
	printf("\nTCP Capture/Server (servprint) application Started  %s\n\nOpen local TCP port [%s]\nWhen active, capture to file and forward to printer [%s]. \n\nVersion 0 2014-JUNE-6 By -DL-\n\n\n",  print_time,argv[1], argv[2]);//send to Status Log


    ////////////CLIENT///////////
    int sd, rc;
    struct sockaddr_in localAddr, servAddr;
    struct hostent *h;


	//////////////OPEN SERVER SOCKET////////////////////////
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}

	if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) == -1) {
		perror("Setsockopt");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero),8);

	if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))== -1) {
		perror("Unable to bind");
		exit(1);
	}

	if (listen(sock, 5) == -1) {
		perror("Listen");
		exit(1);
	}

	fflush(stdout);

	sprintf(ping, "ping -c1 -w1 %s >/dev/null 2>&1", argv[2]);//prepare sys$

	/////////////WAIT FOR CONNECTION FROM PLC//////////////////////////////
	while(1)
	{
		sin_size = sizeof(struct sockaddr_in);

		/*Check for incomming connection, If connection, open connection to printer and store top file*/
		if (listen(sock, 1) == 0){
			connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
			live = 1;
			printf("Data Received\n");
			bytes_received = recv(connected,recv_data,1024,0);

		////////////// OPEN PRINTER CONNECTION ///////////// OPEN PRINTER CONNECTION ///////////// 
			/*Check if Printer connected before opening communications*/
			if (system(ping) != 0){
				ignoreTCP = 1;
				printf("Printer Disconnected\n");
				}
			/*Printer IP is active...Go ahead and establish socket communications */ 
			else {

				h = gethostbyname(argv[2]);/*printer IP address*/
					if (h == NULL) {
					printf("%s: unknown host '%s'\n", argv[0], argv[1]);//For Testing
					ignoreTCP = 1;
				}
				servAddr.sin_family = h->h_addrtype;
				memcpy((char *)&servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
				servAddr.sin_port = htons(atoi(argv[3]));
				/* create socket */
				sd = socket(AF_INET, SOCK_STREAM, 0);
				if (sd < 0) {
					perror("Printer Disconnected (1)\n ");// For Testing
					ignoreTCP = 1;
				}
				/* bind any port number */
				localAddr.sin_family = AF_INET;
				localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
				localAddr.sin_port = htons(0);
					rc = bind(sd, (struct sockaddr *) &localAddr, sizeof(localAddr));
					if (rc<0) {
					printf("%s: cannot bind port TCP %u\n", argv[0], atoi(argv[2]));//For testing
					perror("error ");//for testing
					ignoreTCP = 1;
				}
				/* connect to server */
				rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
				if (rc < 0) {
					perror("Printer Disconnected (2)\n ");//For testing
					ignoreTCP = 1;
				}
			}

			//////////// START WRITING FIRST LINE TO FILE AND PRINTER ///////////// 
			recv_data[bytes_received] = '\0';
//			printf("%s" , recv_data);//View On Screen for testing
			pFile = fopen("/var/www/active_log.txt","a");
			fprintf(pFile,recv_data);
			fclose (pFile);
			////////////////WRITE TO PRINTER SOCKET/////////////////////
			if (ignoreTCP == 0){
				rc = send(sd, recv_data, bytes_received, 0);
				if (rc < 0) {
					printf("cannot send data ");
					close(sd);
					ignoreTCP = 1;
				}
			}
		}
			//////////// CONTINUE WRITING TO FILE AND PRINTER ///////////// 
			while (live){
				bytes_received = recv(connected, recv_data, 1024, 0);
				recv_data[bytes_received] = '\0';
//				printf("%s" , recv_data);//For testing
				pFile = fopen("/var/www/active_log.txt","a");
				fprintf(pFile,recv_data);
				fclose (pFile);
				////////////////WRITE TO PRINTER SOCKET/////////////////////
				if (ignoreTCP == 0 && (recv_data[0] != '\0')){
					rc = send(sd, recv_data, bytes_received, 0);
					if (rc < 0) {
						printf("cannot send data ");
						close(sd);
						ignoreTCP = 1;
					}
				}
				if (recv_data[0]== '\0'){
					time_t t = time(NULL); //used for providing time for logging process displayed on web page
					struct tm tm = *localtime(&t); //used for providing time for logging process displayed on web page
					sprintf(print_time,"%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
					printf("End Data - %s\n\n", print_time);
					live = 0;
					ignoreTCP = 0;
					close(sd);
				}
				recv_data[0]= '\0';
				fflush(stdout);
			}
	}
	close(sock);
	return 0;
}

