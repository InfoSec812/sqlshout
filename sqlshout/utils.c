/**************************************************************************
                          utils.c  -  description
                             -------------------
    begin                : Thu Jan 15 2004
    copyright            : (C) 2004 by Deven Phillips
    email                : dphillips@gothic-hawaii.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern FILE *debugfp ;

int snprintf (char *str, size_t size, const char *format, ...) ;

char *ctime_r(const time_t *timep, char *buf) ;

char *logTime(void)
{
	char *timeString, *returnVal ; 
	time_t *current ;

	timeString = malloc(40) ;
	returnVal = malloc(40) ;
	
	current = malloc(sizeof(time_t)) ;
	time(current) ;
	timeString = ctime_r(current,timeString) ;
	returnVal = malloc(strlen(timeString)) ;
	returnVal = strrchr(timeString,'\n') ;
	returnVal[0] = 0 ;

	return timeString ;
}

char *stringdupe(char *inputStr)
{
	char *returnStr ;

	if (inputStr==NULL) {
		fprintf(debugfp,"%s: Error: NULL pointer in utils.c - stringdupe()\n",logTime()) ;
		return NULL ;
	}

	returnStr = malloc(strlen(inputStr)+1) ;
	if (returnStr==NULL) {
		fprintf(debugfp,"Error allocating memory for new string value.\n") ;
		return NULL ;
	} else {
		snprintf(returnStr,strlen(inputStr)+1,inputStr) ;
		if (returnStr==NULL) {
			fprintf(debugfp,"Error transferring string.") ;
			return NULL ;
		} else {
			return returnStr ;
		}
	}
}

typedef struct dict {
	char *rvalue ;
	char *lvalue ;
} dict_t ;

dict_t *mystrsep(char *tokens, char *seperators)
{
	dict_t *returnvalue ;
	char *test ;
	unsigned int loopA, loopB, position=0, memerror=0 ;

	returnvalue = (dict_t *)malloc(sizeof(dict_t)) ;
	if (returnvalue==NULL) {
		memerror = 1 ;
	}

	returnvalue->rvalue = (char *)malloc(strlen(tokens)) ;	
	if (returnvalue->rvalue==NULL) {
		memerror = 1 ;
	}

	returnvalue->lvalue = (char *)malloc(strlen(tokens)) ;	
	if (returnvalue->lvalue==NULL) {
		memerror = 1 ;
	}

	if (memerror==1) {
		fprintf (debugfp,"Error allocating memory for results.\n") ;
		return NULL ;
	} else {
		for (loopA=0;loopA<strlen(seperators);loopA++) {
			test = strchr(tokens, seperators[loopA]) ;
			if (test!=NULL) {
				for (loopB=0;loopB<strlen(tokens);loopB++) {
					if ((tokens[loopB]!=seperators[loopA]) && (position==0)) {
						if (tokens[loopB]!='\n') {
							returnvalue->rvalue[loopB]=tokens[loopB] ;
						}
					} else if ((tokens[loopB]==seperators[loopA]) && (position==0)) {
						position=loopB+1;
						returnvalue->rvalue[loopB]='\0';
					} else if (loopB>=position) {
						if (tokens[loopB]!='\n') {
							returnvalue->lvalue[loopB-position]=tokens[loopB] ;
						}
					}
				}
			}
		}
	}
	return returnvalue ;
}

void fatalErr(char *errorMessage)
{
        fprintf(debugfp,"%s\n",errorMessage) ;
        exit(EXIT_FAILURE) ;
}





