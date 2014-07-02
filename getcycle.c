
/**********************************************************************************************

GETCYCLE Cycle Detection Application getcycle.c Rev 15 by Dan Lindamood III

V11 2013-04 Initial Release
V12 2014-01-15 Added PDF security
V13 2014-03-30	1)Removed locked cycle number starting point argument.
		2)Created floating value (Line 59).
		3)Added Reboot Command after adjustable amount of prints completed.
V14 2014-04-18  Added alarm list for logging and optional email.
V15 2014-07-02  Replaced printing with libharu

Scope:
Read active_log.txt and determine if the print is from a cycle.
If not, clear the file.

Function:
If the print is from a cycle then:
find the cycle number to be used in the print file name.
wait for an end of cycle indication.
Print to PDF file and copy raw text to PCL folder.

Notes:
Model configurations are set by the shell script file startprocess.sh in the applications
folder located in the web server file location. This is done by command arguments. 1 through 7


***********************************************************************************************/

#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include <time.h>

#define false (0!=0)
#define LOG_FILE	"/var/www/active_log.txt"


  ////////// GET CYCLE NUMBER  ////////// GET CYCLE NUMBER  ////////// GET CYCLE NUMBER  ////////// GET CYCLE NUMBER  //////////

//get_cycle_number function started from main below
char *get_cycle_number(char *cycle_str, char *cycle_num) 
{
   FILE * pFile;
   int cycle_line_num = 1; //represents each line being checked for cycle number. Starts at 1
   int cycle_find_result = 0; //if cycle found, then 0 changes to how many times found.
   char cycle_temp[512]; //entire file
   char str[100]; //character strings withing each line
   char space[] = " ";//used to find end of cycle number
   int line_length; //length of string line where cycle is found
   int start_point = -1;//Location of cycle count number in print header. Let's try the strchr search?
   int num_length; //How many digits in the cycle count.

	pFile = fopen (LOG_FILE , "r"); // Read text file

	if (pFile != NULL) //if file exists then do this.....
	{
		while ((fgets(cycle_temp, 512, pFile)!= NULL) && (cycle_find_result == 0)) //while checking each line for a cycle and not yet found
		{
         		if((strstr(cycle_temp, cycle_str)) != NULL) //If cycle number found then extract the number from the text string.......
			{
				start_point = strstr(cycle_temp, cycle_str) -  cycle_temp + strlen(cycle_str);	//Determine start point of cycle number.
				//Determine how many characters the cycle number contains by finding the first space character at the end of the number.
				num_length = 1;	//start with a default 1 character
				if (cycle_temp[start_point+1]==space[0]) num_length = 1;
				else if (cycle_temp[start_point+2]==space[0])num_length = 2;
				else if (cycle_temp[start_point+3]==space[0])num_length = 3;
				else if (cycle_temp[start_point+4]==space[0])num_length = 4;
                                else if (cycle_temp[start_point+5]==space[0])num_length = 5;

				//http://www.dreamincode.net/forums/topic/54086-cut-a-string-into-different-peices/
       				strncpy(cycle_num, &cycle_temp[start_point], num_length);
			        cycle_num[num_length] = '\0';

//				printf("cycle number test %s\n", cycle_num);//For Testing
	     			cycle_find_result++; //indicate that cycle number was found
			}
			else // if cycle number not found, spoil the number to be ignored in the main function
			{
				cycle_num[1]='\0';
				strncpy(cycle_num,"X",1);
			}
			cycle_line_num++;
		}
		fclose (pFile);
	}
	return cycle_num;
}

  ////////// DETECT ALARM - EMAIL  ////////// DETECT ALARM - EMAIL  ////////// DETECT ALARM - EMAIL  //////////////////

int alarm_detected(char *str_alarm, int alarm_start_point){

   FILE * pFile;
   int alarm_line_num = 0;
   int alarm_find_result = 0;
   char alarm_temp[512];

   pFile = fopen (LOG_FILE , "r");

        while((fgets(alarm_temp, 512, pFile) != NULL)&& (alarm_find_result == 0))
        {
                if(((strstr(alarm_temp, str_alarm)) != NULL)&& (alarm_line_num > alarm_start_point))
                {
//                      printf("%s DETECTED AT LINE %d\n", str_end, end_line_num);//For testing
                        alarm_start_point = alarm_line_num;
                        alarm_find_result++;
                }
                alarm_line_num++;
        }

        fclose (pFile);

        return(alarm_find_result, alarm_start_point);
}



  ////////// DETECT CYCLE END  ////////// DETECT CYCLE END  ////////// DETECT CYCLE END  ////////// DETECT CYCLE END  //////////

int end_detected(char *str_end)
{
   FILE * pFile;
   int end_line_num = 1;
   int end_find_result = 0;
   char end_temp[512];

   pFile = fopen (LOG_FILE , "r");

	while((fgets(end_temp, 512, pFile) != NULL) && (end_find_result == 0))
	{
		if((strstr(end_temp, str_end)) != NULL)
		{
//                	printf("%s DETECTED AT LINE %d\n", str_end, end_line_num);//For testing
                        end_find_result++;
                }
                end_line_num++;
        }

	fclose (pFile);

	return(end_find_result);
}




  ////////// CONFIRM CYCLE  ////////// CONFIRM CYCLE  ////////// CONFIRM CYCLE  ////////// CONFIRM CYCLE  ////////// CONFIRM CYCLE

int confirm_cycle(char *str_confirm)
{
   FILE * pFile;
   int confirm_line_num = 1;
   int confirm_find_result = 0;
   char confirm_temp[512];

   pFile = fopen (LOG_FILE , "r");

	while((fgets(confirm_temp, 512, pFile) != NULL) && (confirm_find_result == 0))
	{
		if((strstr(confirm_temp, str_confirm)) != NULL)
		{
//                	printf("%s CONFIRMS CYCLE AT LINE %d\n", str_confirm, confirm_line_num);// For testing
			confirm_find_result++;
                }
                confirm_line_num++;
        }

	fclose (pFile);

	return(confirm_find_result);
}





  ////////// PRINT  ////////// PRINT  ////////// PRINT  ////////// PRINT  ////////// PRINT  ////////// PRINT  ////////// PRINT

void print_to_pdf(char *cyclenum, char *lines, char *author)
{
FILE * pFile;
FILE * newpdfFile;
char cp_pcl[128];
char newpdf[128];
char prntcmnd[128];

	sprintf (cp_pcl,"cp /var/www/active_log.txt /var/www/pcl/cycle_%s.pcl", cyclenum);
	system(cp_pcl);

        sprintf(prntcmnd,"/var/www/apps/pcl_to_pdf /var/www/pdf/cycle_%s.pdf /var/www/pcl/cycle_%s.pcl %s \'%s\' GetCycle cycle_%s.pdf", cyclenum, cyclenum, lines, author, cyclenum); 


	printf("%s\n",prntcmnd);//for testing
        system(prntcmnd); // print to pdf

        sprintf(newpdf,"/var/www/pdf/cycle_%s.pdf",cyclenum);
//	printf("%s\n",newpdf);// For Testing
	newpdfFile = fopen(newpdf,"r"); //open new pdf file to verify that it exists.
	if  (newpdfFile == NULL) printf("PRINT TO PDF FAILED\n\n\n"); // if pdf file not created, indicate failure.
	else printf("6: PRINT TO PDF SUCCESSFUL\n\n\n"); // if pdf file created, clear log file.


        pFile = fopen (LOG_FILE , "w");
	fclose (pFile);
}

void clear_log()
{
FILE * pFile;

        pFile = fopen (LOG_FILE , "w");
	fclose (pFile);
}

////////// MAIN  ////////// MAIN  ////////// MAIN  ////////// MAIN  ////////// MAIN  ////////// MAIN  ////////// MAIN  ////////// MAIN

int main( int argc, char ** argv)
{

FILE * pFile; //active_log.txt
FILE *pAlarms; //Alarm log
int sequence = 0; //used for delaying functions
int interval = atoi(argv[1]); // Interval timer for checking log file
int count = 0; //used to count characters
int receiving = 0; //compared with character count to determine if log file still receiving data
int c; //each character
char cycnum[6];
char print_time[48];  //used for providing time for logging process displayed on web page
int done = false ;
int cycles = 0 ; //Amount of printed cycles
int alarm_argc = 0;
int alarm_start_point = 0;
int start_relay = 0;
int alarm_count = 0;
int alarm_active = 0;
char alarm_cmd[64];
int i=0;

// for (i=1; i< argc; i++) {
//     printf("\n%s", argv[i]);
// }


time_t t = time(NULL); //used for providing time for logging process displayed on web page
struct tm tm = *localtime(&t); //used for providing time for logging process displayed on web page

//used for providing time for logging process displayed on web page
sprintf(print_time,"%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);// display on web page log
//Printed to nohup.out (processlog.txt) in web page
printf("\n\nCycle Detection Application Started  %s\nMonitors active_log.txt file every %d seconds.\n\nVersion 0 2014-JAN-15 By -DL-\n\n",  print_time, interval);//send to web page log once at startup.

    while( !done )  // Stay Running Continously unless error or manually stopped.
    {

        sleep(interval) ;// Check Log file every ## seconds---must have argument or segment fault

	time_t t = time(NULL); //used for providing time for logging process displayed on web page
	struct tm tm = *localtime(&t); //used for providing time for logging process displayed on web page

        alarm_argc = 0; //reset alarms list

	//used for providing time for logging process displayed on web page
	sprintf(print_time,"%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	pFile = fopen (LOG_FILE , "r"); // read only active_log.txt

	if (pFile != NULL){ //if active_log.txt exist, then do this below......

		c = getc(pFile); // open log file to read characters

		while(c != EOF) {//Continue to read each character until the end of file is detected.
    			count ++; // increase count for each character.
   		 	c = getc(pFile);
		}

		fclose(pFile); //must close the file to reset for next interval pass

		if ( count == 0){
			sequence = 0; // Always reset sequence when file empty
			start_relay = 0;
			alarm_start_point = 0;
			alarm_count = 0;
		}

		else { //If log file is active, perform the following actions within the brackets.

			if (sequence == 0){ // Start the sequence.
			sequence = 1; //Chna
			printf("1: LOG FILE ACTIVE %s\n", print_time);
			}

			if (sequence == 3){ // After allowing page header to print, confirm cycle
				if (confirm_cycle(argv[3]) == 0){//Even though cycle number detected, if not valid cycle then clear log
					clear_log();
					printf("3: NO CYCLE - active_log.txt CLEARED\n\n\n");
					sequence = 0;//reset sequence to 0
				}
				else { //if cycle confirmed, display cycle number on web page process log
					printf("3: CYCLE NUMBER: %s  \n", cycnum);//display cycle number
					sequence = 4;//Allow for cycle end detection
				}
			}
			if (sequence == 4){ //After cycle is confirmed, monitor for end of cycle.
				if (end_detected(argv[4]) >= 1){
					printf("4: END OF CYCLE DETECTED  %s\n", print_time);
					sequence = 5;//allow print function
				}
			}
			if ((sequence == 5)&&(receiving == count)){//Wait for printing to finish after detecting end.
				printf("5: PRINTING \n");
				print_to_pdf(cycnum, argv[5], argv[argc-1]);//Start printing function. Argument 5 sets lines per page.
				sequence = 0; //reset sequence to 0
				cycles ++;
				printf("(%d)out of (%d) cycles printed before reboot.\n\n\n", cycles,atoi(argv[6]));
				if (cycles == atoi(argv[6])){
					printf("Rebooting system");
					system("sudo /sbin/shutdown -r now");
				}
			}

			if (sequence == 2){ //Get cycle number
				printf("2: VERIFY IF CYCLE\n");
				get_cycle_number(argv[2], cycnum);//send to function for cycle number
				if (strlen(cycnum) >= 2) sequence = 3; // Is there a cycle number (at least 2 digits)? If yes, go to next sequence.
				else { // if no cycle number, clear the log and reset sequence.
				    clear_log();
				    printf("3: NO CYCLE DETECTED - active_log.txt CLEARED\n\n\n");
				    sequence = 0; //reset sequence to 0
				}
			}

			if (sequence == 1) sequence = 2;// To allow the 1st page to print, get the cycle number during next interval.

                        if ((sequence >3)&&(alarm_count < atoi(argv[7]))){
                                for (alarm_argc=8;alarm_argc < argc; alarm_argc++) {
                                        alarm_active, start_relay = alarm_detected(argv[alarm_argc], alarm_start_point);
                                        if (start_relay > alarm_start_point){
                                                printf("%s ALARM DETECTED  %s\n",argv[alarm_argc], print_time);
		                                pAlarms = fopen ("/var/www/alarms.txt" , "a"); // add to alarm log
						fprintf(pAlarms, "%s  %s\n",argv[alarm_argc], print_time);
						fclose(pAlarms);
// Email Service Only				sprintf(alarm_cmd, "/var/www/applications/email.sh \'%s\' \'%s\' %s",argv[alarm_argc], print_time, cycnum);
//                                                system(alarm_cmd);
//                                                printf("%s\n", alarm_cmd);
                                                alarm_start_point = start_relay ;
                                                alarm_count++;
                                        }
                                }
                        }

		} // End bracket for active_log.txt not empty

		receiving = count; //equalize with character count to be check for active printing on next interval
		count = 0; // reset for next interval

       	} //End bracket for checking if active_log.txt exists.

        else printf("LOG_FILE = NULL\n");//if active_log file does no exist
	fflush(stdout);//clear stdout every interval. This is for the proper function of writing to the process log.

   }
}


