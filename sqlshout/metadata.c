/***************************************************************************
                          | metqadata.c |  -  Add and send metadata to
			  the streaming server.
                             -------------------
    begin                : | Jan-21-2004 |
    copyright            : (C) |2004| by | Joseph B. Phillips |
    email                : | <dphillips@gothic-hawaii.com> |
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
#include <shout/shout.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <lame/lame.h>
#include <shout/shout.h>
#include <pthread.h>

#include "types.h"
#include "utils.h"

extern FILE *debugfp ;

int snprintf(char *str, size_t size, const char *format, ...);

int send_metadata(shout_t *shoutconn, song_t *current) ;

int send_metadata(shout_t *shoutconn, song_t *current)
{
	int retval ;
	char songinfo[512] ;
	shout_metadata_t *metadata ;


	metadata = NULL ;

	memcpy(songinfo,"\0",512) ;
	/* Set songinfo to all NULL characters prior to starting */

	TRACE1("[%s] Instantiating metadata structure.\n",logTime());

	metadata = shout_metadata_new() ;

	TRACE1("[%s] Instantiated metadata structure.\n",logTime());

	snprintf(songinfo, sizeof(songinfo), "%s - %s",
		(current->artist == NULL ? "Unknown" : current->artist),
		(current->title == NULL ? "Unknown" : current->title));

	TRACE2("[%s] Assigned title string: %s.\n",logTime(),songinfo) ;

	if (shout_metadata_add(metadata, "song", songinfo)!=SHOUTERR_SUCCESS) {
		fprintf (debugfp, "ERROR: Failed to set stream metadata.\n") ;
	}
	TRACE1("[%s] Sent metadata to stream.\n",logTime()) ;

	/* Sets MP3 metadata.
	* Returns:
	*   SHOUTERR_SUCCESS
	*   SHOUTERR_UNSUPPORTED if format isn't MP3
	*   SHOUTERR_MALLOC
	*   SHOUTERR_INSANE
	*   SHOUTERR_NOCONNECT
	*   SHOUTERR_SOCKET
	*/
	retval = shout_set_metadata(shoutconn,metadata) ;

	if (retval != SHOUTERR_SUCCESS) {
		fprintf(debugfp,"[%s] Failed to send shout metadata: ",logTime()) ;
		switch (retval) {
			case SHOUTERR_UNSUPPORTED : 	{
					fprintf(debugfp,"[%s] Unsupported Format.\n",logTime()) ;
					break ;
				}
			case SHOUTERR_MALLOC : 		{
					fprintf(debugfp,"[%s] Memory Allocation Error\n",logTime()) ;
					break ;
				}
			case SHOUTERR_INSANE :		{
					fprintf(debugfp,"[%s] Invalid metadata specified\n",logTime()) ;
					break ;
				}
			case SHOUTERR_NOCONNECT :	{
					fprintf(debugfp,"[%s] Not connected.\n",logTime()) ;
					break ;
				}
			case SHOUTERR_SOCKET :		{
					fprintf(debugfp,"[%s] Shout socket error\n",logTime()) ;
					break ;
				}
		}
	}

	shout_metadata_free(metadata) ;
	metadata = NULL ;
	return(0) ;
}
