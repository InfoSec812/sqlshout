/**************************************************************************
                          reencode.c  -  description
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
#include <lame/lame.h>
#include <shout/shout.h>
#include <mad.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#include "types.h"
#include "utils.h"

/* Function Prototypes */

void getEncodeError(int errcode, char *errString, size_t errLength) ;

static short MadFixedToShort(mad_fixed_t Fixed) ;

static int PrintFrameInfo(struct mad_header *Header) ;

void *stream_mpeg (program_t *prog) ;

long double floorl(long double x) ;

int snprintf (char *str, size_t size, const char *format, ...) ;

/* Function Prototypes */

extern FILE *debugfp ;

/* Define Constants */

#define INPUT_BUFFER_SIZE		(5*8192)
#define OUTPUT_BUFFER_SIZE		8192 /* Must be an integer multiple of 4. */
#define ENCODED_DATA_BUFFER_SIZE 	20000
#define ERROR_MESSAGE_SIZE		1024
#define READ_BUFFER_SIZE		1024

/* End of constant definitions */

/****************************************************************************
 * Converts a sample from mad's fixed point number format to an unsigned	*
 * short (16 bits).															*
 ****************************************************************************/
static short MadFixedToShort(mad_fixed_t Fixed)
{
	/* A fixed point number is formed of the following bit pattern:
	 *
	 * SWWWFFFFFFFFFFFFFFFFFFFFFFFFFFFF
	 * MSB                          LSB
	 * S ==> Sign (0 is positive, 1 is negative)
	 * W ==> Whole part bits
	 * F ==> Fractional part bits
	 *
	 * This pattern contains MAD_F_FRACBITS fractional bits, one
	 * should alway use this macro when working on the bits of a fixed
	 * point number. It is not guaranteed to be constant over the
	 * different platforms supported by libmad.
	 *
	 * The unsigned short value is formed by the least significant
	 * whole part bit, followed by the 15 most significant fractional
	 * part bits. Warning: this is a quick and dirty way to compute
	 * the 16-bit number, madplay includes much better algorithms.
	 */
	Fixed=Fixed>>(MAD_F_FRACBITS-15);
	return((short)Fixed);
}


/****************************************************************************
 * Print human readable informations about an audio MPEG frame.				*
 ****************************************************************************/
static int PrintFrameInfo(struct mad_header *Header)
{
	const char	*Layer,
				*Mode,
				*Emphasis;

	/* Convert the layer number to it's printed representation. */
	switch(Header->layer)
	{
		case MAD_LAYER_I:
			Layer="I";
			break;
		case MAD_LAYER_II:
			Layer="II";
			break;
		case MAD_LAYER_III:
			Layer="III";
			break;
		default:
			Layer="(unexpected layer value)";
			break;
	}

	/* Convert the audio mode to it's printed representation. */
	switch(Header->mode)
	{
		case MAD_MODE_SINGLE_CHANNEL:
			Mode="single channel";
			break;
		case MAD_MODE_DUAL_CHANNEL:
			Mode="dual channel";
			break;
		case MAD_MODE_JOINT_STEREO:
			Mode="joint (MS/intensity) stereo";
			break;
		case MAD_MODE_STEREO:
			Mode="normal LR stereo";
			break;
		default:
			Mode="(unexpected mode value)";
			break;
	}

	/* Convert the emphasis to it's printed representation. */
	switch(Header->emphasis)
	{
		case MAD_EMPHASIS_NONE:
			Emphasis="no";
			break;
		case MAD_EMPHASIS_50_15_US:
			Emphasis="50/15 us";
			break;
		case MAD_EMPHASIS_CCITT_J_17:
			Emphasis="CCITT J.17";
			break;
		default:
			Emphasis="(unexpected emphasis value)";
			break;
	}

  return(ferror(debugfp));
}

void getEncodeError(int errcode, char *errString, size_t errLength)
{
	switch (errcode) {
		case (-1) : {
					snprintf(errString,errLength,"MP3 Buffer was too small (%i)",errcode) ;
					break ;
				}
		case (-2) : {
					snprintf(errString,errLength,"Memory Allocation Error (%i)",errcode) ;
					break ;
				}
		case (-3) : {
					snprintf(errString,errLength,"LAME initialization needed (%i)",errcode) ;
					break ;
				}
		case (-4) : {
					snprintf(errString,errLength,"Psycho acoustic problems (%i)",errcode) ;
					break ;
				}
		case (-5) : {
					snprintf(errString,errLength,"OGG Cleanup Encoding Error (%i)",errcode) ;
					break ;
				}
		case (-6) : {
					snprintf(errString,errLength,"OGG Frame encoding error (%i)",errcode) ;
					break ;
				}
	}
}


/* Use libmp3lame and libmad to re-encode MP3 and stream the mpeg data to the shoutcast server. */

void *stream_mpeg (program_t *prog)
{
	int retval=0, frames=0, test=0, limiter=0, bytesread=0, errLength=100 ;
	short int samples, left[1152], right[1152] ;
	unsigned char *mp3output ;
	char *errmsg, *buffer, *encoderErrorPtr ;
	double buff_size=0 ;
	lame_global_flags *encPtr;                              /* declare lame flags pointers for the encoder and decoder */
	mad_fixed_t amplification ;

	struct mad_stream	Stream;
	struct mad_frame	Frame;
	struct mad_synth	Synth;
	mad_timer_t 		Timer;
	unsigned char		InputBuffer[INPUT_BUFFER_SIZE], OutputBuffer[OUTPUT_BUFFER_SIZE], *OutputPtr=OutputBuffer;
	int 			Status=0, i ;
	unsigned long 		FrameCount=0, FrameTotal=0;
	FILE 			*filePtr ;			/* Declare a file pointer for the input MP3 file */
	
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) ;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) ;
	pthread_cleanup_push(pthread_mutex_unlock, prog->playing) ;

	amplification = prog->song->amplification ;
	/* First the structures used by libmad must be initialized. */
	mad_stream_init(&Stream);
	mad_frame_init(&Frame);
	mad_synth_init(&Synth);
	mad_timer_reset(&Timer);

#ifdef DEBUG
	fprintf(debugfp,"[%s] reencode.c:204:Filename: %s\n\n",logTime(),prog->song->filename) ;
#endif


	mp3output = malloc(ENCODED_DATA_BUFFER_SIZE) ;
	if (mp3output==NULL) {
		fprintf(debugfp,"[%s] Error allocating MP3 output buffer.\n",logTime()) ;
		pthread_exit(NULL) ;
	}
	pthread_cleanup_push(free,mp3output) ; 

	encoderErrorPtr = malloc(errLength) ;
	if (encoderErrorPtr==NULL) {
		fprintf(debugfp,"[%s] Error allocating encoder error holder.\n",logTime()) ;
		pthread_exit(NULL) ;
	}
	pthread_cleanup_push(free,encoderErrorPtr) ; 

/* The default (if you set nothing) is a  J-Stereo, 44.1khz
128kbps CBR mp3 file at quality 5.  Override various default settings
as necessary, for example: */

/* Open designated MP3 file */

	filePtr = fopen((char *)prog->song->filename,"rb") ;	/* Open the input MP3 file for reading with buffering support */
	if (filePtr==NULL) {
		fprintf(debugfp,"[%s] ERROR: Could not open file %s for reading.\n",logTime(),prog->song->filename) ;
		pthread_exit(NULL) ;
	}

	encPtr = lame_init() ;		/* Initialize the lame_decoder */

	errmsg = malloc(ERROR_MESSAGE_SIZE) ;
	if (errmsg==NULL) {
		fprintf(debugfp,"[%s] Error allocating memory for error message string.\n",logTime()) ;
		pthread_exit(NULL) ;
	}
	pthread_cleanup_push(free,errmsg) ; 
	
	buffer = malloc(READ_BUFFER_SIZE) ;
	if (buffer==NULL) {
		fprintf(debugfp, "[%s] Error allocating MP3 encoder buffer.\n",logTime());
		pthread_exit(NULL) ;
	}
	pthread_cleanup_push(free,buffer) ; 

	if (lame_set_num_channels(encPtr,prog->config->channels)<0) {
		fprintf(debugfp,"[%s] Error setting channels.\n",logTime()) ;
	}
	if (lame_set_padding_type(encPtr,0)<0) {
		fprintf(debugfp,"[%s] Error setting channels.\n",logTime()) ;
	}
	if (lame_set_brate(encPtr,prog->config->bitrate/1000)<0) {
		fprintf(debugfp,"[%s] Error setting bit rate.\n",logTime()) ;
	}
	if (lame_set_out_samplerate(encPtr,prog->config->samplerate)<0) {
		fprintf(debugfp,"[%s] Error setting sample rate.\n",logTime()) ;
	}
	if (lame_set_mode(encPtr,prog->config->mpegmode)<0) {
		fprintf(debugfp,"[%s] Error setting MPEG mode.\n",logTime()) ;
	}
	if (lame_set_quality(encPtr,prog->config->quality)<0) {
		fprintf(debugfp,"[%s] Error setting quality.\n",logTime()) ;
	}
	id3tag_init(encPtr) ;
	id3tag_add_v2(encPtr) ;
	id3tag_set_title(encPtr,prog->song->title) ;
	id3tag_set_artist(encPtr,prog->song->artist) ;
	id3tag_set_comment(encPtr,prog->config->streamurl) ;

	retval = shout_get_connected(prog->shoutconn) ;
	if (retval!=SHOUTERR_CONNECTED) {
		fprintf (debugfp,"[%s] Attempting to reconnect to shoutcast server.\n",logTime()) ;
		retval = shout_open(prog->shoutconn) ;
		if (retval!=SHOUTERR_SUCCESS) {
			fprintf (debugfp,"[%s] FATAL ERROR: Could not reconnect to shoutcast server reencode).\n",logTime()) ;
			pthread_exit(NULL) ;
		}
	}
	
	pthread_mutex_unlock(prog->playing) ;	
	/* This is the decoding loop. */
	do {
		/* The input bucket must be filled if it becomes empty or if
		 * it's the first execution of the loop.
		 */
		if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
		{
			size_t ReadSize, Remaining;
			unsigned char	*ReadStart;

			/* {1} libmad may not consume all bytes of the input
			 * buffer. If the last frame in the buffer is not wholly
			 * contained by it, then that frame's start is pointed by
			 * the next_frame member of the Stream structure. This
			 * common situation occurs when mad_frame_decode() fails,
			 * sets the stream error code to MAD_ERROR_BUFLEN, and
			 * sets the next_frame pointer to a non NULL value. (See
			 * also the comment marked {2} bellow.)
			 *
			 * When this occurs, the remaining unused bytes must be
			 * put back at the beginning of the buffer and taken in
			 * account before refilling the buffer. This means that
			 * the input buffer must be large enough to hold a whole
			 * frame at the highest observable bit-rate (currently 448
			 * kb/s). XXX=XXX Is 2016 bytes the size of the largest
			 * frame? (448000*(1152/32000))/8
			 */
			if(Stream.next_frame!=NULL)
			{
				Remaining=Stream.bufend-Stream.next_frame;
				memmove(InputBuffer,Stream.next_frame,Remaining);
				ReadStart=InputBuffer+Remaining;
				ReadSize=INPUT_BUFFER_SIZE-Remaining;
			} else {
				ReadSize=INPUT_BUFFER_SIZE ;
				ReadStart=InputBuffer ;
				Remaining=0; 
			}
			/* Fill-in the buffer. If an error occurs print a message
			 * and leave the decoding loop. If the end of stream is
			 * reached we also leave the loop but the return status is
			 * left untouched.
			 */
			ReadSize=fread(ReadStart,1,ReadSize,filePtr);
			bytesread+=ReadSize;
			if(ReadSize<=0)
			{
				if(ferror(filePtr))
				{
					fprintf(debugfp,"[%s] ERROR: read error on bitstream (%s)\n",logTime(),strerror(errno));
					Status=1;
				}
				if(feof(filePtr)) 
					fprintf(debugfp,"[%s] ERROR: end of input stream\n",logTime());
				break ;
			}

			/* Pipe the new buffer content to libmad's stream decoder facility. */
			mad_stream_buffer(&Stream,InputBuffer,ReadSize+Remaining);
			Stream.error=0;
		}

		/* Decode the next mpeg frame. The streams is read from the
		 * buffer, its constituents are break down and stored the the
		 * Frame structure, ready for examination/alteration or PCM
		 * synthesis. Decoding options are carried in the Frame
		 * structure from the Stream structure.
		 *
		 * Error handling: mad_frame_decode() returns a non zero value
		 * when an error occurs. The error condition can be checked in
		 * the error member of the Stream structure. A mad error is
		 * recoverable or fatal, the error status is checked with the
		 * MAD_RECOVERABLE macro.
		 *
		 * {2} When a fatal error is encountered all decoding
		 * activities shall be stopped, except when a MAD_ERROR_BUFLEN
		 * is signaled. This condition means that the
		 * mad_frame_decode() function needs more input to achieve
		 * it's work. One shall refill the buffer and repeat the
		 * mad_frame_decode() call. Some bytes may be left unused at
		 * the end of the buffer if those bytes forms an incomplete
		 * frame. Before refilling, the remainign bytes must be moved
		 * to the begining of the buffer and used for input for the
		 * next mad_frame_decode() invocation. (See the comments marked
		 * {1} earlier for more details.)
		 *
		 * Recoverable errors are caused by malformed bit-streams, in
		 * this case one can call again mad_frame_decode() in order to
		 * skip the faulty part and re-sync to the next frame.
		 */

		if(mad_frame_decode(&Frame,&Stream)) {
			if(MAD_RECOVERABLE(Stream.error)) {
				fprintf(debugfp,"[%s] ERROR: recoverable frame level error (%s)\n",logTime(),mad_stream_errorstr(&Stream));
				fflush(debugfp);
				continue;
			} else {
				if(Stream.error==MAD_ERROR_BUFLEN) {
					continue;
				} else {
					fprintf(debugfp,"[%s] ERROR: unrecoverable frame level error (%s).\n",logTime(),mad_stream_errorstr(&Stream));
					Status=1;
					break;
				}
			}
		}

		/* The characteristics of the stream's first frame is printed
		 * on debugfp. The first frame is representative of the entire
		 * stream.
		 */
		if (test==0) {
			if (lame_set_in_samplerate(encPtr,Frame.header.samplerate)<0) {
				fprintf(debugfp,"[%s] Error setting samplerate.\n",logTime()) ;
			}
			TRACE2("[%s] Input samplerate set to %i.\n",logTime(),Frame.header.samplerate) ;

			retval = lame_init_params(encPtr) ;

			if (retval<0) {
				fprintf (debugfp,"Error initializing LAME samplerate parameter.\n") ;
			}
			test = 1;
		}

		if(FrameCount==0)
			if(PrintFrameInfo(&Frame.header))
			{
				Status=1;
				break;
			}

		/* Accounting. The computed frame duration is in the frame
		 * header structure. It is expressed as a fixed point number
		 * whole data type is mad_timer_t. It is different from the
		 * samples fixed point format and unlike it, it can't directly
		 * be added or substracted. The timer module provides several
		 * functions to operate on such numbers. Be careful there, as
		 * some functions of mad's timer module receive some of their
		 * mad_timer_t arguments by value!
		 */
		FrameCount++;
		mad_timer_add(&Timer,Frame.header.duration) ;
		
		/* Once decoded the frame is synthesized to PCM samples. No errors
		 * are reported by mad_synth_frame();
		 */
		mad_synth_frame(&Synth,&Frame) ;

		/* Synthesized samples must be converted from mad's fixed
		 * point number to the consumer format. Here we use unsigned
		 * 16 bit big endian integers on two channels. Integer samples
		 * are temporarily stored in a buffer that is flushed when
		 * full.
		 */
		for(i=0;i<Synth.pcm.length;i++)
		{
			/* Left channel */
			left[i]=MadFixedToShort(Synth.pcm.samples[0][i]);
			*(OutputPtr++)=left[i]>>8;
			*(OutputPtr++)=left[i]&0xff;

			/* Right channel. If the decoded stream is monophonic then
			 * the right output channel is the same as the left one.
			 */
			if(MAD_NCHANNELS(&Frame.header)==2)
				right[i]=MadFixedToShort(Synth.pcm.samples[1][i]);
			*(OutputPtr++)=right[i]>>8;
			*(OutputPtr++)=right[i]&0xff;
		}

		samples = Synth.pcm.length ;

		if (shout_get_connected(prog->shoutconn)!=SHOUTERR_CONNECTED) {
			retval = shout_open(prog->shoutconn) ;
			if (retval!=SHOUTERR_SUCCESS) {
				fprintf (debugfp,"[%s] FATAL ERROR: Not connected to shoutcast server and could not reconnect (%i).\n",logTime(),retval) ;
				pthread_exit(NULL) ;
			} else {
				fprintf(debugfp,"[%s] Reconnected to shoutcast server.\n",logTime()) ;
			}
		}

		/* Flush the buffer if it is full. */
		if (limiter==0) {
			TRACE2("[%s] Encoding Bitrate: %i\n",logTime(),prog->config->bitrate) ;
			TRACE2("[%s] Encoding Samplerate: %i\n",logTime(),prog->config->samplerate) ;
			TRACE2("[%s] PCM Samples: %i\n",logTime(),Synth.pcm.length) ;
			TRACE2("[%s] PCM Sample Rate: %i\n",logTime(),Synth.pcm.samplerate) ;
			TRACE2("[%s] Channels: %i\n",logTime(),prog->config->channels) ;
			TRACE2("[%s] MPEG Mode: %i\n",logTime(),prog->config->mpegmode) ;
			limiter = 1 ;
		}
		buff_size = Synth.pcm.length*1.25 + 7200 ;
		frames = ceil(buff_size) + 0 ;
		retval = lame_encode_buffer(encPtr,left,right,Synth.pcm.length,mp3output,frames) ;
		if ((retval < 0) || (mp3output==NULL)) {
			getEncodeError(retval, encoderErrorPtr, errLength) ;
			fprintf(debugfp,"[%s] ERROR: Encoder error (%s).\n",logTime(),encoderErrorPtr) ;
			Status=2;
			pthread_exit(NULL) ;
		} else {
			if (shout_send(prog->shoutconn,mp3output,retval)>=SHOUTERR_SUCCESS) {
                		FrameTotal = FrameTotal + FrameCount ;
            		} else {
				fprintf(debugfp,"[%s] SHOUT ERR: %s\n",logTime(),shout_get_error(prog->shoutconn)) ;
			}
			shout_sync(prog->shoutconn);
		}
		OutputPtr=OutputBuffer;
	} while(!feof(filePtr));
	
	/* print out a new line to clear the line from the bytes read/encoded debugging printout */

/* Call lame_encode_finish() to clear the remianing frames and to free the resources allocated to encPtr */

	retval = lame_encode_finish(encPtr,mp3output,OUTPUT_BUFFER_SIZE) ;	
	encPtr = NULL ;
	if (retval < 0) {
		fprintf(debugfp,"[%s] Failed to flush MP3 Buffer",logTime()) ;
	} else {
		shout_send(prog->shoutconn,mp3output,retval) ;
	}
	
/* Close the file pointer and free the resources associated with it */
	fclose(filePtr) ;

/* Close out the libmad decoder and free the associated resources */
	mad_synth_finish(&Synth) ;
	mad_frame_finish(&Frame) ;
	mad_stream_finish(&Stream) ;

/* Free the resources for various pointers used in the function and set them equal to NULL for safety */
	pthread_cleanup_pop(0) ;
	free(buffer) ;
	buffer = NULL ;
	
	pthread_cleanup_pop(0) ;
	free(errmsg) ;
	errmsg = NULL ;

	pthread_cleanup_pop(0) ;
	free(encoderErrorPtr) ;
	encoderErrorPtr = NULL ;

	pthread_cleanup_pop(0) ;
	free(mp3output) ;
	mp3output = NULL ;
	
	pthread_cleanup_pop(0) ;
/* Return a value of 0 to indicate that we made it to the end of the function with no errors */
	return NULL ;
}

