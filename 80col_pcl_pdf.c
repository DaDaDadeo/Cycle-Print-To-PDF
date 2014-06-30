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
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "/usr/local/include/hpdf.h"

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

//////////////////////////MAIN//////////////////////////MAIN//////////////////////////MAIN//////////////////////////
int main (int argc, char **argv)
{
    HPDF_Doc  pdf;
    char fname[256];
    HPDF_Page page;
    HPDF_REAL height;
    HPDF_REAL width;



    if (argc < 3) {
        printf("Add [printed file name] [cycle count]\n");
        return 0;
    }

    strcpy (fname, argv[1]);
    strcat (fname, ".pdf");

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

    const static char* owner_passwd = "owner";

    HPDF_SetPassword (pdf, owner_passwd, ""); 
    HPDF_SetPermission (pdf, HPDF_ENABLE_PRINT);
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
   char logfile[64];

      sprintf(logfile, "/var/www/pcl/cycle_%s.pcl",argv[2]);
      pFile = fopen (logfile , "r");

        while((fgets(temp, 138, pFile) != NULL)) {  //Retrieve each line from text file and check it for printing.


                if (newpage == 1){ //if new page is triggered and \f at end of previous line, go ahead and start new page now
                        page = HPDF_AddPage (pdf);
                        HPDF_Page_SetSize(page,HPDF_PAGE_SIZE_LETTER,HPDF_PAGE_PORTRAIT);
                        newpage = 0;
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
                        height = HPDF_Page_GetHeight (page);
                        width = HPDF_Page_GetWidth (page);
                        HPDF_Page_BeginText (page);
                        HPDF_Page_MoveTextPos (page, 25, height - 25);
                }

                HPDF_Font font = HPDF_GetFont (pdf, "Courier", NULL);//Use this font for entire print.

                if (strstr(temp, "(s16H")!= NULL) { //Change font size to small if pcl6 code indicates to do so
			HPDF_Page_SetFontAndSize (page, font, 7.5);
                        memmove(temp,temp+6,strlen(temp)-6);//Remove pcl6 code
			if (strstr(temp, "(s10H")!= NULL) null_loc = strstr(temp, "(s10H")- temp - 1;//Look for location of dead pcl6 code at end of line
                  	else null_loc = 120;//Default to 120 characters in case of error
//			printf("%d\n",null_loc);//For Testing
		 	temp[null_loc]= '\0';//Nullify the pcl6 code at end of line.
		}
		else HPDF_Page_SetFontAndSize (page, font, 12);
		if (strstr(temp, "*r-3U")!= NULL) { //Change font color to red if pcl6 code indicates to do so. (for alarms).
			HPDF_Page_SetRGBFill (page, 1.0, 0.0, 0);//code for red
                        memmove(temp,temp+12,strlen(temp)-12);//Remove pcl6 code
//			printf("%s\n",temp);//For Testing
			if (strstr(temp, "v07S")!= NULL) null_loc = strstr(temp, "v07S")- temp - 2;//Remove pcl6 code
                        else null_loc = 120;//Default to 120 characters in case of error
			printf("%d\n",null_loc);//For Testing
                        temp[null_loc]= '\0';//Nullify the pcl6 code at end of line.
                }
                else HPDF_Page_SetRGBFill (page, 0.0, 0.0, 0);//Default to black font colr if not alarm
//		printf("%s\n",temp); //For Testing
                HPDF_Page_ShowText (page, temp);//Send line to pdf print
                HPDF_Page_MoveTextPos (page, 0 , - 12);//Set next position for new line


        	line_num++;
        }

        fclose (pFile);




    HPDF_Page_EndText (page);

    HPDF_SaveToFile (pdf, fname);

    /* clean up */
    HPDF_Free (pdf);

    return 0;
}


