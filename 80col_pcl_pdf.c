/*
 * << Haru Free PDF Library 2.0.0 >> -- font_demo.c
 *
 * Copyright (c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 **********************************************************************
 *
 * Remix by Dan Lindamood III
 * 
 * Scope: Create PDF file from text with plc6 code. 
 * 	  Match formatting to printers using raw 9100 telnet protocol.
 *
 * Rev 1  2014Jun30
 *
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "hpdf.h"
#include <time.h>

jmp_buf env;

#ifdef HPDF_DLL
void  __stdcall
#else
void
#endif
error_handler (HPDF_STATUS   error_no,
               HPDF_STATUS   detail_no,
               void         *user_data)
{
    printf ("ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
                (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

//RANDOM PASSWORD//////////////RANDOM PASSWORD//////////////RANDOM PASSWORD//////////////RANDOM PASSWORD//////////


char *rand_str(char *dst, int size)
{
   static const char text[] = "abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "1234567890";
   int i;
   for ( i = 0; i < 8; ++i )
   {
      dst[i] = text[rand() % (sizeof text - 1)];
   }
   dst[i] = '\0';
   return dst;
}



//MAIN /////////////////////MAIN//////////////////////////MAIN//////////////////////////MAIN//////////////////////////
int main (int argc, char **argv)
{
    HPDF_Doc  pdf;
    char fname[256];
    HPDF_Page page;
    HPDF_REAL height;
    HPDF_REAL width;



    if (argc < 4) {
        printf("\nAdd [Printed File] [File to Print] [Lines Per Page]\n\n\
		Example: /home/user/newfile.pdf /home/user/filetoprint.txt 62\n\n\
		Additional Options:\n\
		Author\n\
		Creator\n\
		Title\n\
		Subject\n\
		Keywords\n\n");
        return 0;
    }

    strcpy (fname, argv[1]);

    pdf = HPDF_New (error_handler, NULL);
    if (!pdf) {
        printf ("error: cannot create PdfDoc object\n");
        return 1;
    }

    if (setjmp(env)) {
        HPDF_Free (pdf);
        return 1;
    }

    /* Start with the first page. */
    page = HPDF_AddPage (pdf);

    if (argc >= 5) HPDF_SetInfoAttr(pdf, HPDF_INFO_AUTHOR, argv[4]);
    if (argc >= 6) HPDF_SetInfoAttr(pdf, HPDF_INFO_CREATOR, argv[5]);
    if (argc >= 7)HPDF_SetInfoAttr(pdf, HPDF_INFO_TITLE,  argv[6]);
    if (argc >= 8)HPDF_SetInfoAttr(pdf, HPDF_INFO_SUBJECT,  argv[7]);
    if (argc == 9)HPDF_SetInfoAttr(pdf, HPDF_INFO_KEYWORDS,  argv[8]);




    char owner_passwd[10];
    srand(time(0)); 
    rand_str(owner_passwd, sizeof owner_passwd);//Create random password
//    printf("Password Test: %s\n", owner_passwd);//For Testing

    HPDF_SetPassword (pdf, owner_passwd, ""); 
    HPDF_SetPermission (pdf, HPDF_ENABLE_PRINT | HPDF_ENABLE_COPY);
    HPDF_SetEncryptionMode (pdf, HPDF_ENCRYPT_R3, 16);
    HPDF_Page_SetRGBFill (page, 0.0, 0.0, 0);
    HPDF_Page_SetSize(page,HPDF_PAGE_SIZE_LETTER,HPDF_PAGE_PORTRAIT); 
 
    height = HPDF_Page_GetHeight (page);
    width = HPDF_Page_GetWidth (page);


    HPDF_Page_BeginText (page);
    HPDF_Page_MoveTextPos (page, 25, height - 25);


   FILE * pFile;
   int line_num = 0;
   char temp[138];
   char *find_ff;
   int newpage = 0;
   int null_loc = -1 ;
   int page_line = 0;
	pFile = fopen (argv[2] , "r");//text file to print

        while((fgets(temp, 138, pFile) != NULL)) {  //Retrieve each line from text file and check it for printing.


                if ((newpage == 1) || (page_line == atoi(argv[3]))){ //if new page is triggered and \f at end of previous line (or line count == ##), go ahead and start new page now
                        page = HPDF_AddPage (pdf);
                        HPDF_Page_SetSize(page,HPDF_PAGE_SIZE_LETTER,HPDF_PAGE_PORTRAIT);
                        newpage = 0;
                        page_line = 0;//reset line count for new page
			height = HPDF_Page_GetHeight (page);
                        width = HPDF_Page_GetWidth (page);
                        HPDF_Page_BeginText (page);
                        HPDF_Page_MoveTextPos (page, 25, height - 25);
		}

		find_ff = strchr(temp, '\f');

		if(find_ff){ //If a new form feed (\f  ascii 12) character or 61 lines, trigger new page bit
			/* Add a new page object. */
			newpage++;
			size_t len = find_ff - temp;
			memmove(&temp[len], &temp[len+1], strlen(temp) - len);//Remove character from print

		}
                if (newpage == 1 && ((find_ff-temp)==0)){ //if new page is triggered and \f at start of string, start new page now
                        page = HPDF_AddPage (pdf);
                        HPDF_Page_SetSize(page,HPDF_PAGE_SIZE_LETTER,HPDF_PAGE_PORTRAIT);
			newpage = 0;
                        page_line = 0;//reset line count for new page
			height = HPDF_Page_GetHeight (page);
                        width = HPDF_Page_GetWidth (page);
                        HPDF_Page_BeginText (page);
                        HPDF_Page_MoveTextPos (page, 25, height - 25);
                }

                HPDF_Font font = HPDF_GetFont (pdf, "Courier", NULL);//Use this font for entire print.

                if (strstr(temp, "(s16H")!= NULL) { //Change font size to small if pcl characters indicates to do so
			HPDF_Page_SetFontAndSize (page, font, 7.5);
                        memmove(temp,temp+6,strlen(temp)-6);//Remove pcl charcters
			if (strstr(temp, "(s10H")!= NULL) null_loc = strstr(temp, "(s10H")- temp - 1;//Look for location of dead pcl code at end of line
                  	else null_loc = 120;//Default to 120 characters in case of error
//			printf("%d\n",null_loc);//For Testing
		 	temp[null_loc]= '\0';//Nullify the pcl characters at end of line.
		}
		else HPDF_Page_SetFontAndSize (page, font, 12);
		if (strstr(temp, "*r-3U")!= NULL) { //Check to change font color to red if pcl characters indicates to do so. (for alarms).
			HPDF_Page_SetRGBFill (page, 1.0, 0.0, 0);//Change to red
                        memmove(temp,temp+12,strlen(temp)-12);//Remove pcl characters
//			printf("%s\n",temp);//For Testing
			if (strstr(temp, "v07S")!= NULL) null_loc = strstr(temp, "v07S")- temp - 2;//Remove pcl characters
                        else null_loc = 120;//Default to 120 characters in case of error
//			printf("%d\n",null_loc);//For Testing
                        temp[null_loc]= '\0';//Nullify the pcl6 code at end of line.
                }
                else HPDF_Page_SetRGBFill (page, 0.0, 0.0, 0);//Default to black font color if not alarm
//		printf("%s\n",temp); //For Testing
                HPDF_Page_ShowText (page, temp);//Send line to pdf print
                HPDF_Page_MoveTextPos (page, 0 , - 12);//Set next position for new line


        	line_num++;
		page_line++;
        }

        fclose (pFile);




    HPDF_Page_EndText (page);

    HPDF_SaveToFile (pdf, fname);

    /* clean up */
    HPDF_Free (pdf);

    return 0;
}


