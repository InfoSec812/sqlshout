/***************************************************************************
                          mysql.c  -  description
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
#include <mysql/mysql.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <lame/lame.h>
#include <shout/shout.h>
#include <pthread.h>

#include "types.h"
#include "utils.h"

#define SQL_QUERY_BUFFER_SIZE 4096

extern FILE *debugfp ;

int snprintf (char *str, size_t size, const char *format, ...) ;

int update_music_queue(MYSQL *dbptr, program_t *prog) ;
   
/* Update the queue and return the name of the next track */

int update_music_queue(MYSQL *dbptr, program_t *prog)
{

	MYSQL_RES *res;
	MYSQL_ROW row ;
	int usable_songs=0, num_to_add=1, limit=0;
	int songid, ratingidx, pingval;
	unsigned long *rowlengths ;
	char *buffer ;
	
	buffer = malloc(SQL_QUERY_BUFFER_SIZE) ;
	/* Check to ensure that the MySQL connection is up, and reconnect if needed. */

	pingval = mysql_ping(dbptr) ;
	if (pingval != 0) {
		fprintf (debugfp,"[%s] Error: Ping test failed with code:%i\nMySQL connection invalid. Unable to automatically reconnect.\n",logTime(),pingval) ;
		exit(1) ;
	}

	/* Create INSERT for log table, copying requester */
	snprintf(buffer,SQL_QUERY_BUFFER_SIZE,"INSERT INTO nuke_music_recent (song,requesttime) SELECT song AS song, requesttime AS requesttime FROM nuke_music_queue ORDER BY ratingidx LIMIT 1");

	TRACE2("[%s] Filename: %s\n",logTime(),prog->song->filename);

	TRACE2("[%s] DEBUG: %s\n",logTime(),buffer);

	mysql_real_query(dbptr,buffer,strlen(buffer)) ;
	if (mysql_errno(dbptr)!=0) {
		fprintf (debugfp,"[%s] MySQL error:Failed to update recently played list:178: %s\n",logTime(),mysql_error(dbptr)) ;
		/* Not a fatal error. Just means the song was not added to the recently played list */
	}

	snprintf(buffer,SQL_QUERY_BUFFER_SIZE,"DELETE FROM nuke_music_queue ORDER BY ratingidx LIMIT 1");
	mysql_real_query(dbptr,buffer,strlen(buffer)) ;
	if (mysql_errno(dbptr)!=0) {
		fprintf (debugfp,"[%s] MySQL error:Failed to remove last song from queue:189: %s\n",logTime(),mysql_error(dbptr)) ;
		return 1 ;
	}

	/* Query to see how many songs are in the recently played list and remove excess to prevent database
	slowdowns */
	snprintf(buffer,SQL_QUERY_BUFFER_SIZE,"SELECT count(recentidx) FROM nuke_music_recent GROUP BY recentidx");

	if (mysql_real_query(dbptr,buffer,strlen(buffer))!=0) {
		fprintf (debugfp,"[%s] MySQL error:61: %s\n:",logTime(),mysql_error(dbptr)) ;
		return -1 ;
	}

	res = mysql_store_result(dbptr) ;
	if (mysql_errno(dbptr)!=0) {
		fprintf (debugfp,"[%s] MySQL error:67: %s\n:",logTime(),mysql_error(dbptr)) ;
		return -1 ;
	}
	
	pingval = mysql_num_rows(res) ;
	if (mysql_errno(dbptr)!=0) {
		fprintf (debugfp,"[%s] MySQL error:95: %s\n:",logTime(),mysql_error(dbptr)) ;
		return -1 ;
	}
	
	if (pingval > 0) {
		row = mysql_fetch_row(res) ;
		if (mysql_error(dbptr)[0]!='\0') {
			fprintf (debugfp,"[%s] MySQL error:111: %s\n:",logTime(),mysql_error(dbptr)) ;
			return -1 ;
		} else if (row==NULL) {
			fprintf (debugfp,"[%s] MySQL error:114: %s\n:",logTime(),mysql_error(dbptr)) ;
			return -1 ;
		}		

		fprintf (debugfp,"[%s] DEBUG: mysql.c:75: %d - %lu\n",logTime(),prog->config->repeat_time,strtol(row[0],NULL,0)) ;

		if (prog->config->repeat_time > strtol(row[0],NULL,0)) {
			limit = 0 ;
		} else {
			limit = strtol(row[0],NULL,0) - prog->config->repeat_time ;
		}

		fprintf (debugfp,"[%s] Deleting %d songs and keeping %lu.\n",logTime(),limit,strtol(row[0],NULL,0)) ;

		snprintf(buffer,SQL_QUERY_BUFFER_SIZE,"DELETE FROM nuke_music_recent ORDER BY recentidx LIMIT %d",limit);

		if (mysql_real_query(dbptr,buffer,strlen(buffer))!=0) {
			fprintf (debugfp,"[%s] MySQL error:61: %s\n:",logTime(),mysql_error(dbptr)) ;
			return -1 ;
		}

		/* Free existing result set */
		mysql_free_result(res);
	}

	/* Query for songs we can use that are in the queue
	 * (Disregards entries that were played recently, etc.) */
	snprintf(buffer,SQL_QUERY_BUFFER_SIZE,"SELECT q.song,p.filename,p.artist,p.title,q.ratingidx FROM nuke_music_queue AS q, nuke_music_playlist AS p LEFT JOIN nuke_music_recent AS r ON (r.song = q.song) WHERE q.song=p.songidx ORDER BY q.ratingidx");
	TRACE2("[%s] DEBUG: %s\n",logTime(),buffer) ;

	if (mysql_real_query(dbptr,buffer,strlen(buffer))!=0) {
		fprintf (debugfp,"[%s] MySQL error:61: %s\n:",logTime(),mysql_error(dbptr)) ;
		return -1 ;
	}

	res = mysql_store_result(dbptr) ;
	if (mysql_errno(dbptr)!=0) {
		fprintf (debugfp,"[%s] MySQL error:67: %s\n:",logTime(),mysql_error(dbptr)) ;
		return -1 ;
	}

	/* How many songs are there? */
	usable_songs = (int)mysql_num_rows(res);

	TRACE3("[%s] There are %i usable songs and we want %i songs in the queue.\n",logTime(),usable_songs,prog->config->queue_size);

	if (mysql_errno(dbptr)!=0) {
		fprintf (debugfp,"[%s] MySQL error:74: %s\n:",logTime(),mysql_error(dbptr)) ;
		return -1 ;
	}

	/* See if we're supposed to replenish the queue */
	if (usable_songs < prog->config->queue_size)
	{
		/* Free existing result set */
		mysql_free_result(res);

		/* Yep, replenish queue. */

		/* TODO: come up with a better algorithm for queue replenishment */
		num_to_add = prog->config->queue_size - usable_songs ;

		TRACE2("[%s] Adding %i songs to the queue.\n",logTime(),num_to_add);
		TRACE2("[%s] MYSQL: %s\n",logTime(),mysql_info(dbptr)) ;

		snprintf(buffer,SQL_QUERY_BUFFER_SIZE,"INSERT INTO nuke_music_queue (song) SELECT songidx AS song FROM nuke_music_playlist AS p LEFT JOIN nuke_music_recent AS r ON (r.song = p.songidx) WHERE !bad ORDER BY RAND() LIMIT %i", num_to_add);
		TRACE2("[%s] DEBUG: %s\n",logTime(),buffer) ;
		mysql_real_query(dbptr,buffer,strlen(buffer)) ;
		if (mysql_errno(dbptr)!=0) {
			fprintf (debugfp,"[%s] MySQL error:94: %s\n:",logTime(),mysql_error(dbptr)) ;
			return -1 ;
		}
		TRACE1("[%s] Queue is full!!\n",logTime()) ;
	} else {
		/* Free existing result set */
		mysql_free_result(res);
	}
	res = NULL ;

	/* Query for songs we can use that are in the queue
	 * (Disregards entries that were played recently, etc.) */
	snprintf(buffer,SQL_QUERY_BUFFER_SIZE, "SELECT q.song,p.filename,p.artist,p.title,q.ratingidx,p.amplification FROM nuke_music_queue AS q, nuke_music_playlist AS p LEFT JOIN nuke_music_recent AS r ON (r.song = q.song) WHERE q.song=p.songidx ORDER BY q.ratingidx");

	if (mysql_real_query(dbptr,buffer,strlen(buffer))!=0) {
		fprintf (debugfp,"[%s] MySQL error:61: %s\n:",logTime(),mysql_error(dbptr)) ;
		return -1 ;
	}

	res = mysql_store_result(dbptr) ;
	if (mysql_errno(dbptr)!=0) {
		fprintf (debugfp,"[%s] MySQL error:67: %s\n:",logTime(),mysql_error(dbptr)) ;
		return -1 ;
	}

	row = mysql_fetch_row(res) ;
	if (mysql_error(dbptr)[0]!='\0') {
		fprintf (debugfp,"[%s] MySQL error:111: %s\n:",logTime(),mysql_error(dbptr)) ;
		return -1 ;
	} else if (row==NULL) {
		fprintf (debugfp,"[%s] MySQL error:114: %s\n:",logTime(),mysql_error(dbptr)) ;
		return -1 ;
	}		

	rowlengths = mysql_fetch_lengths(res) ;
	if (mysql_error(dbptr)[0]!='\0') {
		fprintf (debugfp,"[%s] MySQL error:120: %s\n:",logTime(),mysql_error(dbptr)) ;
		return -1 ;
	} else if (row==NULL) {
		fprintf (debugfp,"[%s] MySQL error:123: %s\n:",logTime(),mysql_error(dbptr)) ;
		return -1 ;
	}		

	/* q.song, p.filename, q.request, p.artist, p.title, q.ratingidx */
	songid = strtol(row[0],NULL,0);
	ratingidx = strtol(row[4],NULL,0);

	TRACE1("ratingidx = %i\n",ratingidx) ;

	if (row[5]==NULL) {
		TRACE0("row[5] is NULL\n") ;
	}

	prog->song->filename = stringdupe(row[1]);
	if (prog->song->filename == NULL) {
		fprintf(debugfp,"%s: Error: stringdupe returned a NULL pointer.\n",logTime()) ;
		return 1 ;
	}

	prog->song->artist = stringdupe(row[2]);
	if (prog->song->artist == NULL) {
		fprintf(debugfp,"%s: Error: stringdupe returned a NULL pointer.\n",logTime()) ;
		return 1 ;
	}
	prog->song->title = stringdupe(row[3]);
	if (prog->song->artist == NULL) {
		fprintf(debugfp,"%s: Error: stringdupe returned a NULL pointer.\n",logTime()) ;
		return 1 ;
	}
	buffer = stringdupe(row[5]) ;
	TRACE1("buffer = %s\n",buffer) ;
	if (buffer == NULL) {
		fprintf(debugfp,"%s: Error: stringdupe returned a NULL pointer.\n",logTime()) ;
		return 1 ;
	}
	prog->song->amplification = strtof(buffer,NULL) ;
	
	TRACE2("[%s] mysql.c:137:Filename: %s\n",logTime(),prog->song->filename) ;

	free(buffer) ;
	buffer = NULL ;
	return 0 ;

}
