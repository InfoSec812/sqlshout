/***************************************************************************
                          network.h  -  definitions for network functions
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


extern int hostlookup(char *hostnane, char *address) ;

extern char *strip_white_space(char *input) ;

extern void *spawn_listener(void *prog) ;

