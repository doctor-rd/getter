#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "getter.h"

int main( int argc, char* const argv[] ) {
	int fd;
	char *url;
	char *filename;

	if( pledge( "stdio rpath cpath wpath sendfd", NULL ) == -1 ) {
		perror( "pledge" );
		return 1;
	}

	if( argc < 2 ) {
		printf( "Usage: %s URL\n", argv[0] );
		return 0;
	}

	url = argv[1];
	filename = strrchr( url, '/' );
	if( filename == NULL ) {
		fprintf( stderr, "Unable to determine filename\n" );
		return 1;
	}
	filename++;

	fd = open( filename, O_RDWR|O_EXCL|O_CREAT, 0666 );
	if( fd == -1 ) {
		perror( filename );
		return 1;
	}
	if( getter_httpget( fd, url ) == -1 ) {
		unlink( filename );
		fprintf( stderr, "Failed\n" );
		return 1;
	}

	return 0;
}
