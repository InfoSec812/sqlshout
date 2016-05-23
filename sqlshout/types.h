/***************************************************************************
                          types.h  -  description
                             -------------------
    begin                : Thu Jan 15 09:53:41 HST 2004
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

#ifndef __SQLSHOUT_TYPES_H__
#define __SQLSHOUT_TYPES_H__

/* Debugging macro */
#ifdef DEBUG
#define TRACE0(a) fprintf(debugfp, a); fflush(debugfp)
#define TRACE1(a,b) fprintf(debugfp, a, b); fflush(debugfp)
#define TRACE2(a,b,c) fprintf(debugfp, a, b, c); fflush(debugfp)
#define TRACE3(a,b,c,d) fprintf(debugfp, a, b, c, d); fflush(debugfp)
#define TRACE4(a,b,c,d,e) fprintf(debugfp, a, b, c, d, e); fflush(debugfp)
#define TRACE6(a,b,c,d,e,f) fprintf(debugfp, a, b, c, d, e, f); fflush(debugfp)
#else
#define TRACE0(a)
#define TRACE1(a,b)
#define TRACE2(a,b,c)
#define TRACE3(a,b,c,d)
#define TRACE4(a,b,c,d,e)
#define TRACE6(a,b,c,d,e,f)
#endif

#define         MAX_U_32_NUM            0xFFFFFFFF

typedef struct config			/* Define parameters type. */
{
/* Database configuration options */
	char *dbhost;			/* MySQL Database Host : Defaults to localhost */
	int dbport ;			/* MySQL Database Port : Defaults to 3306 */
	char *dbuser ;			/* MySQL Database username */
	char *dbpass ;			/* MySQL Database password */
	char *dbname ;			/* MySQL Database name */

/* ShoutCast configuration options */
	char *shouthost ;		/* Shoutcast/Icecast server address : Defaults to localhost */
	int shoutport ;			/* Shoutcast/Icecast server stream port : Defaults to 8001 */
	char *shoutpass ;		/* Shoutcast/Icecast server stream password */
	char *shoutuser ;		/* Shoutcast/Icecast username */
	char *streamdesc ;		/* Shoutcast/Icecast stream description */
	char *streamurl ;		/* URL Associated with the stream */
	char *shouttype ;		/* Streaming server protocol ICY/HTTP/XAUDIOCAST */
	char *shoutname ;		/* Stream short name */
	char *mountpoint ;		/* IceCast server mount point (Only applies to IceCast) */

/* LAME configuration options */
	int samplerate ;		/* Sample Rate (in hertz) */
	int bitrate ;			/* Stream Bit Rate (in bits/sec) */
	int mpegmode ;			/* MPEG 1/2/3 : Defaults to MPEG 1 */
	int quality ;			/* Encoder quality : Defaults to 5 : 1=High Quality/High CPU usage - 2=Low Quality/Low CPU usage */
	int channels ;			/* Channels : Monoaural or J-Stereo (0 or 1) : Defaults to J-Stereo */

/* Program options */
	int listen ;
	int daemon ;			/* Should the program detach and run in the background? */
	int repeat_time;		/* The amount of time to wait before allowing a song to be played again */
	int queue_size;			/* How many entries to keep in the queue */
	char *outlog ;			/* Output log file */
	int playid ;			/* 0 means do not play station IDs | 1 Means play station ID every idrotation songs */
	int idrotation ;		/* Play station ID every X songs */
} config_t;           

typedef struct song			/* Define a structure for holding information about an MP3 track */
{
	char *artist ;			/* Artist name - Pulled from either the database or the ID3 Tag */
	char *title ;			/* Song title - Pulled from either the database or the ID3 Tag */
	char *album ;			/* Album title - Pulled from either the database or the ID3 Tag */
	char *filename ;		/* Song file name - Pulled from the playlist database */
	int songid ;			/* Song ID - Song identification number - Unique database identifier */
	int length ;			/* Length - The length of the song in seconds */
	int bitrate ;			/* Song bit rate (in kilobits/second) */
	float amplification ;	/* The apmplification value to be applied to the song */
} song_t;

typedef struct program
{
	void *encoder ;
	song_t *song;
	config_t *config;
	shout_t *shoutconn;		/* Declare the shoutcast connection structure */
	pthread_mutex_t *playing ;	/* Lock the current song info so that we don't get collisions betwee the threads */
	pthread_t canceler ;
	int connection ;
} program_t;	

typedef struct dict
{
	char *lvalue ;
	char *rvalue ;
} dict_t ;

#endif /* __SQLSHOUT_TYPES_H__ */
