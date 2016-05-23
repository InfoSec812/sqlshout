/***************************************************************************
                          mysql.h  -  description
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

/* Queries MySQL database using libmysqlclient and returns 0 on success.
Returns -1 on irrecoverable error. Takes database pointer, configuration
pointer, and current song pointer as arguments */
 
int update_music_queue(void *dbptr, program_t *prog) ;
