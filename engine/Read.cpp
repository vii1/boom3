//////////////////////////////////////////////////////////////////////////
//                                                                      //
//   BOOM 2 Engine                                                      //
//                                                                      //
//   read.cpp - basic reading functions implementation                  //
//                                                                      //
//   by Ivaylo Beltchev                                                 //
//   e-mail: ivob@geocities.com                                         //
//   www:    www.geocities.com/SiliconValley/Bay/2234                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#define _READ_CPP
#include "read.h"

#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#define MAXFILES 10

static int fd;

// opens a file for reading
bool read_start( char *fn )
{
	fd = open( fn, O_RDONLY | O_BINARY );
	if( fd == -1 ) return false;
	return true;
}

// closes the current file
void read_end()
{
	if( fd != -1 ) close( fd );
	fd = -1;
}

// returns the current file pointer
long read_pos()
{
	return tell( fd );
}

// skips len bytes
void skip( unsigned len )
{
	lseek( fd, len, SEEK_CUR );
}

// reads size bytes in buf
void rdbuf( void *buf, unsigned size )
{
	read( fd, buf, size );
}

// reads a 4 byte integer
long rdlong()
{
	LE_int32 buf;
	read( fd, &buf, sizeof( buf ) );
	return buf;
}

// reads a 2 byte integer
short rdshort()
{
	LE_int16 buf;
	read( fd, &buf, sizeof( buf ) );
	return buf;
}

// reads a 1 byte integer
char rdchar()
{
	char buf;
	read( fd, &buf, sizeof( buf ) );
	return buf;
}

// reads a float
float rdfloat()
{
	LE_float buf;
	read( fd, &buf, sizeof( buf ) );
	return buf;
}
