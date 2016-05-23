/***************************************************************************
                          main.c  -  description
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

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef PTHREAD_H
#include <pthread.h>
#endif

#include <shout/shout.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mad.h>
#include <errno.h>
#include <math.h>
#include <mysql/mysql.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "types.h"
#include "mysql.h"
#include "id3.h"
#include "reencode.h"
#include "metadata.h"
#include "utils.h"
#include "network.h"

/* Function Prototypes */

int fetch_config (program_t *prog, char *configfile) ;

int daemonize() ;

void close_standard_io() ;

/* Function Prototypes */

/* Global variables */

FILE *debugfp ;

/* Global Variables */

int daemonize ()
{

	int pid, sid ;

	pid = fork() ;

	if (pid==0) {
		umask(0) ;

		sid = setsid() ;
		if (sid < 0) {
			fatalErr("Failed to detach into background. Exiting.") ;
		}
		stderr = freopen("/dev/null","w",stderr) ;
		stdin = freopen("/dev/null","r",stdin) ;
		stdout = freopen("/dev/null","w",stdout) ;
	}		
	chdir("/") ;
	return pid ;
}

/* Read config file and parse values into configuration structure */

int fetch_config (program_t *prog, char *configfile)
{

	FILE *fileptr ;

	char buffer[1024], *temp;
	dict_t *tokens ;
	int isSetDBHost=0, isSetDBPort=0, isSetDBUser=0, isSetDBPass=0, isSetDBName=0, isSetShoutHost=0, isSetShoutPort=0 ;
	int isSetShoutUser=0, isSetShoutPass=0, isSetShoutName=0, isSetShoutDesc=0, isSetStreamURL=0, isSetBackground=0 ;
	int isSetMPEGMode=0, isSetChannels=0, isSetQuality=0, isSetBitRate=0, isSetSampleRate=0, isSetProtocol=0 ;
	int isSetMountPoint=0, isSetQueueSize=0, isSetNoRepeat=0, isSetLogFile=0, isSetListenPort=0, isSetIDRotation=0 ;
	int isSetPlayID=0 ;

	fileptr = fopen(configfile,"r") ;

	if (fileptr == NULL) {
		printf ("Error opening file %s for reading\n",configfile);
		return -1 ;
	}

	while (fgets(buffer,sizeof(buffer),fileptr)!=NULL)
	{
		if ((strlen(buffer)>3) && (buffer[0] != '#') && (buffer[0] != ';')) { /* Ignore lines starting with a ';' or with a '#' */
			TRACE2("[%s] DEBUG: Buffer |%s|.\n",logTime(),buffer) ;
			temp = &buffer[0] ;
			tokens = mystrsep(buffer,"=") ;
			if (tokens==NULL) {
				fatalErr("String parser error.") ;
				return 1 ;
			}
			
			if (tokens->rvalue!=NULL) {
				TRACE2("DEBUG: Option %s has value %s.\n",tokens->lvalue,tokens->rvalue) ;
				if (strcmp(tokens->lvalue,"DBHOST")==0) {
					TRACE1("DEBUG: %s\n",tokens->rvalue) ;
					prog->config->dbhost = stringdupe(tokens->rvalue) ;
					isSetDBHost = 1 ;
				}

				if (strcmp(tokens->lvalue,"DBPORT")==0) {
					prog->config->dbport = strtol(tokens->rvalue,NULL,0) ;
					if ((prog->config->dbport==0) || (prog->config->dbport>65535)) {
						fatalErr("Insane value for DBPORT (Must be an integer).") ;
					} else {
						isSetDBPort = 1 ;
					}
				}

				if (strcmp(tokens->lvalue,"DBUSER")==0) {
					prog->config->dbuser = stringdupe(tokens->rvalue) ;
					isSetDBUser = 1 ;
				}
				
				if (strcmp(tokens->lvalue,"LISTEN")==0) {
					prog->config->listen = strtol(tokens->rvalue,NULL,0) ;
					if ((prog->config->shoutport==0) || (prog->config->shoutport>65535) || (prog->config->shoutport==EINVAL) || (prog->config->shoutport==ERANGE)) {
						fatalErr("Insane value for LISTEN. Must be between 0 and 65535.") ;
					}
					isSetListenPort = 1 ;
				}

				if (strcmp(tokens->lvalue,"DBPASS")==0) {
					prog->config->dbpass = stringdupe(tokens->rvalue) ;
					isSetDBPass = 1 ;
				}

				if (strcmp(tokens->lvalue,"DBNAME")==0) {
					prog->config->dbname = stringdupe(tokens->rvalue) ;
					isSetDBName = 1 ;
				}

				if (strcmp(tokens->lvalue,"SHOUTHOST")==0) {
					prog->config->shouthost = stringdupe(tokens->rvalue) ;
					isSetShoutHost = 1 ;
				}

				if (strcmp(tokens->lvalue,"SHOUTUSER")==0) {
					prog->config->shoutuser = stringdupe(tokens->rvalue) ;
					isSetShoutUser = 1 ;
				}

				if (strcmp(tokens->lvalue,"SHOUTPORT")==0) {
					prog->config->shoutport = strtol(tokens->rvalue,NULL,0) ;
					if ((prog->config->shoutport==0) || (prog->config->shoutport>65535) || (prog->config->shoutport==EINVAL) || (prog->config->shoutport==ERANGE)) {
						fatalErr("Insane value for SHOUTPORT (Must be an integer).") ;
					} else {
						isSetShoutPort = 1 ;
					}
				}

				if (strcmp(tokens->lvalue,"SHOUTPASS")==0) {
					prog->config->shoutpass = stringdupe(tokens->rvalue) ;
					isSetShoutPass = 1 ;
				}

				if (strcmp(tokens->lvalue,"SHOUTNAME")==0) {
					prog->config->shoutname = stringdupe(tokens->rvalue) ;
					isSetShoutName = 1 ;
				}

				if (strcmp(tokens->lvalue,"STREAMDESC")==0) {
					prog->config->streamdesc = stringdupe(tokens->rvalue) ;
					isSetShoutDesc = 1 ;
				}

				if (strcmp(tokens->lvalue,"STREAMURL")==0) {
					prog->config->streamurl = stringdupe(tokens->rvalue) ;
					isSetStreamURL = 1 ;
				}

				if (strcmp(tokens->lvalue,"BACKGROUND")==0) {
					prog->config->daemon = strtol(tokens->rvalue,NULL,0) ;
					if ((prog->config->daemon!=0) && (prog->config->daemon!=1)) {
						fatalErr("Insane value for BACKGROUND (Must be 0 or 1).\n") ;
					} else {
						isSetBackground = 1 ;
					}
				}

				if (strcmp(tokens->lvalue,"MPEGMODE")==0) {
					prog->config->mpegmode = strtol(tokens->rvalue,NULL,0) ;
					if ((prog->config->mpegmode<0) || (prog->config->mpegmode>5)) {
						fatalErr("Insane value for MPEGMODE. (Must be between 0 and 5).") ;
					} else {
						isSetMPEGMode = 1 ;
					}
				}

				if (strcmp(tokens->lvalue,"SAMPLERATE")==0) {
					prog->config->samplerate = strtol(tokens->rvalue,NULL,0) ;
					if ((prog->config->samplerate==LONG_MIN) || (prog->config->samplerate==LONG_MAX)) {
						fatalErr("Insane value for SAMPLERATE (Must be an integer).") ;
					} else {
						isSetSampleRate = 1 ;
					}
				}

				if (strcmp(tokens->lvalue,"BITRATE")==0) {
					prog->config->bitrate = strtol(tokens->rvalue,NULL,0) ;
					if ((prog->config->bitrate==LONG_MIN) || (prog->config->bitrate==LONG_MAX)) {
						fatalErr("Insane value for BITRATE (Must be an integer).") ;
					} else {
						isSetBitRate = 1 ;
					}
				}

				if (strcmp(tokens->lvalue,"CHANNELS")==0) {
					prog->config->channels = strtol(tokens->rvalue,NULL,0) ;
					if ((prog->config->channels<0) || (prog->config->channels>4)) {
						fatalErr("Insane value for CHANNELS (Must be between 0 and 4).") ;
					} else {
						isSetChannels = 1 ;
					}
				}

				if (strcmp(tokens->lvalue,"QUALITY")==0) {
					prog->config->quality = strtol(tokens->rvalue,NULL,0) ;
					if ((prog->config->quality < 0) || (prog->config->quality > 8)) {
						fatalErr("Insane value for QUALITY (Must be between 0 and 7).") ;
					} else {
						isSetQuality = 1 ;
					}
				}

				if (strcmp(tokens->lvalue,"PROTOCOL")==0) {
					prog->config->shouttype = stringdupe(tokens->rvalue) ;
					isSetProtocol = 1 ;
				}

				if (strcmp(tokens->lvalue,"MOUNTPOINT")==0) {
					prog->config->mountpoint = stringdupe(tokens->rvalue) ;
					isSetMountPoint = 1 ;
				}

				if (strcmp(tokens->lvalue,"QUEUE_SIZE")==0) {
					prog->config->queue_size = strtol(tokens->rvalue,NULL,0) ;
					if ((prog->config->queue_size < 1) || (prog->config->queue_size > 20)) {
						fatalErr("Insane value for QUEUE_SIZE (Must be between 2 and 20).") ;
					} else {
						isSetQueueSize = 1 ;
					}
				}

				if (strcmp(tokens->lvalue,"NOREPEAT")==0) {
					prog->config->repeat_time = strtol(tokens->rvalue,NULL,0) ;
					if (prog->config->repeat_time < 0) {
						fatalErr("Insane value for NOREPEAT (Must be an integer > 0).") ;
					} else {
						isSetNoRepeat = 1 ;
					}
				}

				if (strcmp(tokens->lvalue,"STATIONID")==0) {
					prog->config->playid = strtol(tokens->rvalue,NULL,0) ;
					if ((prog->config->playid != 0) && (prog->config->playid != 1)) {
						fatalErr("Insane value for NOREPEAT (Must be either 1 or 0).") ;
					} else {
						isSetPlayID = 1 ;
					}
				}

				if (strcmp(tokens->lvalue,"IDROTATION")==0) {
					prog->config->idrotation = strtol(tokens->rvalue,NULL,0) ;
					if ((prog->config->idrotation <= 1)) {
						fatalErr("Insane value for IDROTATION (Must be an integer greater than 1.") ;
					} else {
						isSetIDRotation = 1 ;
					}
				}

				if (strcmp(tokens->lvalue,"LOGFILE")==0) {
					prog->config->outlog = stringdupe(tokens->rvalue) ;
					if (prog->config->outlog==NULL) {
						fatalErr("Insane value for outlog (Must be a path and filename).") ;
					} else {
						isSetLogFile = 1 ;
					}
				}
			}
			free(tokens) ;
			tokens=NULL ;
		}
	}
	
	/* The following section checks to see if various required fields are set, and if they are not it may set defaults or exit with an error. */
	
	if (!isSetDBHost) {
		prog->config->dbhost = stringdupe("localhost") ; /* Default to using localhost */
	}
	if (!isSetListenPort) {
		prog->config->listen = 8010 ; /* Default to using localhost */
	}
	if (!isSetDBPort) {
		prog->config->dbport = 3306 ;	/* Default port for MySQL databases */
	}
	if (!isSetDBUser) {
		fprintf (debugfp, "Config File Error: Database username not specified.\n") ; /* Required field, throw an error message and exit() */
		exit(1) ;
	}
	if (!isSetDBPass) {
		fprintf (debugfp, "Config File Warning: Database password not specified.\n") ; /* Throw a warning at the user and continue */
	}
	if (!isSetDBName) {
		fprintf (debugfp, "Config File Error: Database name not specified.\n") ; /* Required field, throw an error and exit() */
		exit (1) ;
	}
	if (!isSetShoutHost) {
		prog->config->shouthost = stringdupe("localhost") ; /* Default to localhost */
	}
	if (!isSetShoutPort) {
		fprintf (debugfp, "Config File Warning: ShoutCast server port not specified, using 8000.\n") ;
		prog->config->shoutport = 8000 ; /* Default to 8000 */
	}
	if (!isSetShoutPass) {
		fprintf(debugfp, "Config File Warning: ShoutCast server password not specified, using 'letmein'.\n") ;
		prog->config->shoutpass = stringdupe("letmein") ; /* Warn the user that the password for the Shout server is not specified */
		
	}
	if (!isSetShoutName) {
		prog->config->shoutname = stringdupe("I Haven't configured this yet") ;
	}
	if (!isSetShoutDesc) {
		prog->config->streamdesc = stringdupe("I Haven't configured this yet") ;
	}
	if (!isSetStreamURL) {
		prog->config->streamurl = stringdupe("http://sqlshout.sourceforge.net/") ;
	}
	if (!isSetBackground) {
		prog->config->daemon = 0 ;	/* Default to running in the foreground so that error messages can be seen */
	}
	if (!isSetMPEGMode) {
		prog->config->mpegmode = 3 ; /* Default to Monaural */
	}
	if (!isSetBitRate) {
		prog->config->bitrate = 32000 ; /* Default to 32KBPs */
	}
	if (!isSetChannels) {
		prog->config->channels = 1 ; /* Again, default for Monaural */
	}
	if (!isSetSampleRate) {
		prog->config->samplerate = 22050 ; /* Default to a 22kHz sample rate */
	}
	if (!isSetQuality) {
		prog->config->quality = 5 ; /* LAME encoder quality defaults to 5 (MODERATE) */
	}
	if (!isSetProtocol) {
		prog->config->shouttype = stringdupe("HTTP") ; /* Defaults to protocol for IceCast servers */
	}
	if (!isSetMountPoint) {
		prog->config->mountpoint = stringdupe("/") ; /* Not a required field, but we'll set it to something anyhow */
	}
	if (!isSetQueueSize) {
		prog->config->queue_size = 5 ;	/* Default to keeping 5 songs in the queue */
	}
	if (!isSetNoRepeat) {
		prog->config->repeat_time = 15 ; /* Default to setting the no-repeat limit to 15 songs */
	}
	if (!isSetLogFile) {
		prog->config->outlog = stringdupe("/tmp/sqlshout.log") ; /* Default PWD for logfile */
	}
	if (!isSetPlayID) {
		prog->config->playid = 0 ;
	}
	if (!isSetIDRotation) {
		prog->config->idrotation = 10 ;
	}
	return 0 ; /* Return to main() */
}

/* Program Main */

int main (int argc, char **argv)
{
	char *conffile, *holder ;				/* Declare string pointers for the song filename and config filename */
	int errcount=0, retval=0 ;				/* Declare an integer variable to hold an error counter */
	program_t *prog ;			 		/* Declare the configuration data structure, definition in types.h */
	pid_t ppid ;						/* Only used when we are daemonize()ing */
	static MYSQL *dbptr;					/* Database connection pointer */
	pthread_t thread_id=0, netthread=0 ;					/* Thread ID for the streamer thread */
	void *returnval ;

	
	debugfp = stderr ;
	
	if (getuid()==0) {
		fatalErr("SQLShout cannot be run with root privileges.") ;
	}
	
	prog = malloc(sizeof(program_t)) ;
	if (prog==NULL) {
		fatalErr("Error allocating prog structure memory.") ;
	}
 	prog->song = malloc(sizeof(song_t)) ;		/* Allocate memory for the song data structure */
	if (prog->song==NULL) {					/* The 'song' structure is defined in types.h */
		fatalErr("Error allocating prog->song structure memory.") ;
	}
	prog->config = malloc(sizeof(config_t)) ;		/* Allocate memory for the config structure */
	if (prog->config==NULL) {				/* The 'config' structure is defined in types.h */
		fatalErr("Error allocating prog->config structure memory.") ;
	}
	prog->playing = malloc(sizeof(pthread_mutex_t)) ;
	if (prog->playing==NULL) {				/* The 'playing' structure is defined in types.h */
		fatalErr("Error allocating prog->playing memory.") ;
	}
	
	if (argc > 1) {
		conffile = stringdupe(argv[1]) ;			/* Check argc to see if a config file was specified, and if so, assign the config file name to the conffile string */
		TRACE1("Using configuration file located at %s\n",conffile) ;
	} else {
		conffile = stringdupe("./sqlshout.conf") ;			/* Assign the default value for the configuration file */
	}

	if (fetch_config(prog,conffile)>0) {	/* Call fetch_config() to parse the configuration file */
		fprintf(debugfp,"Failed to read/parse configuration file.\n") ;
		exit(1) ;
	}

	free(conffile) ;
	conffile=NULL ;
	
	if (prog->config->daemon!=0) {			/* If the program config calls for daemon mode,*/ 
		debugfp = fopen(prog->config->outlog,"a") ;
		if (debugfp==NULL) {
			debugfp = stderr ;
			TRACE0("Failed to open output log file. Disallowing background operation.\n") ;
			ppid = 0 ;
		} else {
			ppid = daemonize() ;		/* fork() and then detach from terminal */
		}
	} else {
		debugfp = stderr ;
		ppid = 0 ;				/* Otherwise, continue on as normal */
	}

	if (ppid==0) {
		dbptr = mysql_init(dbptr) ;		/* Call mysql_init() to initialize the MySQL connection structure */
		pthread_mutex_init(prog->playing,NULL) ;/* Call pthread_mutex_init() setup the mutex pointer */		

	/* Connect to the MySQL database using the values specfied in the configuration file */
		if (!mysql_real_connect(dbptr,prog->config->dbhost,prog->config->dbuser,prog->config->dbpass,prog->config->dbname,prog->config->dbport,NULL,0)) {
			fprintf (debugfp,"Error connecting to MySQL server. MySQL error follows.\n%s\n",mysql_error(dbptr)) ;
			exit(1) ;
		} else {
			TRACE0("Successfully connected to MySQL database.\n") ;
		}


	/*  Check to see that the connection to the database was successful */
		if (mysql_errno(dbptr)!=0)
		{
			fprintf(debugfp,"%s\n",mysql_error(dbptr));	/* if the connection failed, print the mysql_error() to STDERR */
			exit(1);						/* If the connection failed, exit the program with a non-zero exit code */
		}

		mysql_select_db(dbptr, prog->config->dbname) ;		/* Select the database as specified in the configuration file */
		if (mysql_errno(dbptr)!=0)				/* Check to see if mysql_select_db() was successful */
		{
			fprintf(debugfp,"%s\n",mysql_error(dbptr));			/* if mysql_select_db failed, print the mysql_error() to STDERR */
			exit(1);										/* Also, exit the program with a non-zero exit code */
		}
	/* Need to initialize the shoutcast stream here */
		shout_init() ;
		if (!(prog->shoutconn = shout_new())) {
			fprintf(debugfp,"Could not allocate shout_t\n");
			exit (1);
		}

		if (shout_set_host(prog->shoutconn, (char *)prog->config->shouthost) != SHOUTERR_SUCCESS) {
			fprintf(debugfp,"Error setting hostname: %s\n", shout_get_error(prog->shoutconn));
			return 1;
		}
		TRACE1("\nSHOUTCAST HOST: %s\n",prog->config->shouthost) ;

		if (shout_set_user(prog->shoutconn, (char *)prog->config->shoutuser) != SHOUTERR_SUCCESS) {
			fprintf(debugfp,"Error setting username: %s\n", shout_get_error(prog->shoutconn));
			return 1;
		}
		TRACE1("\nSHOUTCAST HOST: %s\n",prog->config->shouthost) ;

/* Determine which shoutcast protocol has been selected vi the configuration */
		if (strcmp(prog->config->shouttype,"ICY")==0) {
			TRACE0("Setting protocol to SHOUT_PROTOCOL_ICY\n") ;
			retval = SHOUT_PROTOCOL_ICY ;
		}
		if (strcmp(prog->config->shouttype,"HTTP")==0) {
			TRACE0("Setting protocol to SHOUT_PROTOCOL_HTTP\n") ;
			retval = SHOUT_PROTOCOL_HTTP ;
		}
		if (strcmp(prog->config->shouttype,"XAUDIOCAST")==0) {
			TRACE0("Setting protocol to SHOUT_PROTOCOL_XAUDIOCAST\n") ;
			retval = SHOUT_PROTOCOL_XAUDIOCAST ;
		}

		if (shout_set_protocol(prog->shoutconn, retval) != SHOUTERR_SUCCESS) {
			fprintf(debugfp,"Error setting protocol: %s\n", shout_get_error(prog->shoutconn));
			exit (1);
		}
		TRACE1("\nSHOUTCAST PROTOCOL: %i\n",retval) ;


		if (shout_set_port(prog->shoutconn, (int)prog->config->shoutport) != SHOUTERR_SUCCESS) {
			fprintf(debugfp,"Error setting port: %i - %s\n",(int)prog->config->shoutport, shout_get_error(prog->shoutconn));
			exit(1);
		}
		TRACE1("\nSHOUTCAST PORT: %i\n",prog->config->shoutport) ;

		if (shout_set_password(prog->shoutconn, (char *)prog->config->shoutpass) != SHOUTERR_SUCCESS) {
			fprintf(debugfp,"Error setting password: %s\n", shout_get_error(prog->shoutconn));
			exit(1);
		}
		TRACE1("\nSHOUTCAST PASS: %s\n",prog->config->shoutpass) ;

		if (shout_set_name(prog->shoutconn, (char *)prog->config->shoutname) != SHOUTERR_SUCCESS) {
			fprintf(debugfp,"Error setting stream name: %s\n", shout_get_error(prog->shoutconn));
			exit(1);
		}
		TRACE1("\nSHOUTCAST STREAM NAME: %i\n",(int)prog->config->shoutname) ;

		if (shout_set_url(prog->shoutconn, (char *)prog->config->streamurl) != SHOUTERR_SUCCESS) {
			fprintf(debugfp,"Error setting stream url: %s\n", shout_get_error(prog->shoutconn));
			exit(1);
		}
		TRACE1("\nSHOUTCAST STREAM URL: %i\n",(int)prog->config->streamurl) ;

		if (shout_set_mount(prog->shoutconn, prog->config->mountpoint) != SHOUTERR_SUCCESS) {
			fprintf(debugfp,"Error setting mount: %s\n", shout_get_error(prog->shoutconn));
			exit(1);
		}

		if (shout_set_format(prog->shoutconn, SHOUT_FORMAT_MP3) != SHOUTERR_SUCCESS) {
			fprintf(debugfp,"Error setting format: %s\n", shout_get_error(prog->shoutconn));
			exit(1);
		}

		holder = malloc(10) ;
		if (holder==NULL) {
			TRACE0("Unable to allocate memory for holder variable.\n") ;
			exit(1);
		}

		snprintf(holder,10,"%i",(int)prog->config->bitrate/1000) ;
		if (shout_set_audio_info(prog->shoutconn,"bitrate",holder)!=SHOUTERR_SUCCESS) {
			fprintf(debugfp,"Error setting bitrate. (%i)\n",(int)prog->config->bitrate) ;
			exit(1) ;
		}

		TRACE1("Setting bit rate to %s.\n",holder) ;

		snprintf(holder,10,"%i",(int)prog->config->samplerate/1000) ;
		if (shout_set_audio_info(prog->shoutconn,"samplerate",holder)!=SHOUTERR_SUCCESS) {
			fprintf(debugfp,"Error setting samplerate. (%i)\n",(int)prog->config->samplerate) ;
			exit(1) ;
		}

		TRACE1("Setting sample rate to %s.\n",holder) ;

		snprintf(holder,10,"%i",prog->config->quality) ;
		if (shout_set_audio_info(prog->shoutconn,"quality",holder)!=SHOUTERR_SUCCESS) {
			fprintf(debugfp,"Error setting quality. (%i)\n",prog->config->quality) ;
			exit(1) ;
		}

		TRACE1("Setting encoder quality to %s.\n",holder) ;
		
		snprintf(holder,10,"%i",prog->config->channels) ;
		if (shout_set_audio_info(prog->shoutconn,"channels",holder)!=SHOUTERR_SUCCESS) {
			fprintf(debugfp,"Error setting channels. (%i)\n",prog->config->channels) ;
			exit(1) ;
		}

		TRACE1("Setting number of channels to %s.\n",holder) ;
		
		free(holder) ;

		do {
			pthread_mutex_lock(prog->playing) ;
			retval = update_music_queue (dbptr, prog) ; /* Function defined in mysql.h */	
			/* Perform playlist updates, queue updates, and get next song file name */
			pthread_mutex_unlock(prog->playing) ;
			if (retval < 0) {
				fprintf (debugfp,"Failed to add song(s) to queue.\n") ;
				errcount++ ;
				if (errcount>=4) {
					pthread_cancel(thread_id) ;
					fatalErr("Too many consecutive errors, exiting") ;
				}
			} else {
				if (prog->song->filename==NULL) {							
					/* If the filename is NULL, generate an error */
					fprintf (debugfp,"Error assigning filename string:%i\n",errcount) ;
					errcount++ ;
					/* Keep an error count. If error 4 consecutive times, stop program */
					if (errcount>=4) {
						pthread_cancel(thread_id) ;
						fatalErr("Too many consecutive errors, exiting") ;
					}
				} else {
					TRACE1("Streaming \"%s\" to shoutcast server.\n", (char *)prog->song->filename) ;
					TRACE1("Current status of thread_id: %lu\n",thread_id) ;
					if (thread_id==0) {
						if (prog->playing==NULL) {
							fatalErr("Mutex initialization error.") ;
						}
						if (shout_get_connected(prog->shoutconn)!=SHOUTERR_CONNECTED) {
							fprintf (debugfp,"Shout stream disconnected, attempting to re-establish.\n") ;
							if (shout_open(prog->shoutconn)!=0) {
								fprintf (debugfp,"[%s] Failed to re-establish stream. EXIT.\n",logTime()) ;
								pthread_cancel(thread_id) ;
								exit(EXIT_FAILURE) ;
							}						
						}
						retval = send_metadata(prog->shoutconn, prog->song) ;
						/* Call send_metadata to stream meatadata headers to server */

						if (retval < 0) {
							fprintf(debugfp,"[%s] Warning: Unable to stream metadata to server.\n",logTime()) ;
       	   					}

						pthread_mutex_lock (prog->playing) ;
						retval = pthread_create (&thread_id, NULL, stream_mpeg, prog);
						if (retval!=0) {
							fprintf(debugfp,"[%s] Error spawning reencoder thread.\n",logTime()) ;
						}
						fprintf(debugfp,"[%s] Thread ID: %i\n",logTime(),thread_id) ;
						prog->encoder = &thread_id ;
						retval = pthread_create (&netthread, NULL, spawn_listener, prog) ;
						if (retval!=0) {
							fprintf(debugfp,"[%s] Error spawning reencoder thread.\n",logTime()) ;
						}
					} else {
						pthread_join (thread_id, &returnval) ;
						if (shout_get_connected(prog->shoutconn)!=SHOUTERR_CONNECTED) {
							fprintf (debugfp,"[%s] Shout stream disconnected, attempting to re-establish.\n",logTime()) ;
							if (shout_open(prog->shoutconn)!=0) {
								fprintf (debugfp,"[%s] Failed to re-establish stream. EXIT.\n",logTime()) ;
								pthread_cancel(thread_id) ;
								exit(EXIT_FAILURE) ;
							}						
						}
						retval = send_metadata(prog->shoutconn, prog->song) ;
						/* Call send_metadata to stream meatadata headers to server */

						if (retval < 0) {
							fprintf(debugfp,"[%s] Warning: Unable to stream metadata to server.\n",logTime()) ;
       	   					}
                    
						pthread_mutex_lock (prog->playing) ;
						retval = pthread_create (&thread_id, NULL, stream_mpeg, prog);  /* stream_mpeg() defined in reencode.c */
						if (retval!=0) {
							fprintf(debugfp,"[%s] Error spawning reencoder thread.\n",logTime()) ;
						}
						fprintf(debugfp,"[%s] Thread ID: %i\n",logTime(),thread_id) ;
					}
					
					/* real-time re-encode the MP3 and stream it to the shoutcast server */

					if (retval<0) {															/* check to see if the streaming function returned an error value */
						fprintf (debugfp,"[%s] Failed to add stream data to shoutcast server.\n",logTime()) ;
						errcount++ ;
						/* Keep an error count */
						if (errcount>=4) {
							/* If there are 4 consecutive errors, exit the program */
							pthread_cancel(thread_id) ;
							fatalErr("Too many consecutive errors, exiting") ;
						}
					} else {
						errcount=0 ;
						/* Reset the error counter if everything goes well */
					}
				}
			}
		} while (1) ;				/* Loop until killed */

		mysql_close(dbptr) ;			/* Close and free the mysql structure */
	}
	return 0 ;
}
