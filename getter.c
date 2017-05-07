#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "getter.h"

static int conn;

__attribute__((constructor)) static void init() {
	struct sockaddr_un addr;
	memset( &addr, 0, sizeof( addr ) );
	addr.sun_family = AF_UNIX;
	strcpy( addr.sun_path, GETTER_SOCK );
	if( ( conn = socket( AF_UNIX, SOCK_DGRAM, 0 ) ) == -1 ) {
		perror( "getter.c" );
		exit( 1 );
	}
	if( connect( conn, (struct sockaddr*) &addr, sizeof( addr ) ) == -1 ) {
		perror( "getter.c" );
		exit( 1 );
	}
	fprintf( stderr, "getter enabled\n" );
	return;
}

void getter_disable() {
	close( conn );
	conn = -1;
}

int getter_httpget( int fd, char *url ) {
	if( conn == -1 ) {
		fprintf( stderr, "getter has been disabled\n" );
		return -1;
	}

	int len = strlen( url );
	if( len > 1023 ) {
		fprintf( stderr, "url too long\n" );
		return -1;
	}
	struct iovec par;
	par.iov_base = url;
	par.iov_len = len + 1;

	struct msghdr msg;
	char buf[CMSG_SPACE( 2 * sizeof( int ) )];
	memset( &msg, 0, sizeof( msg ) );
	msg.msg_control = buf;
	msg.msg_controllen = sizeof( buf );
	msg.msg_iov = &par;
	msg.msg_iovlen = 1;

	int ctrl[2];
	if( socketpair( AF_UNIX, SOCK_SEQPACKET, 0, ctrl ) == -1 ) {
		perror( "getter.c" );
		return -1;
	}

	struct cmsghdr *cmsg = CMSG_FIRSTHDR( &msg );
	cmsg->cmsg_len = CMSG_LEN( 2 * sizeof( int ) );
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	int *cdata = (int*) CMSG_DATA( cmsg );
	cdata[0] = fd;
	cdata[1] = ctrl[1];

	sendmsg( conn, &msg, 0 );
	close( ctrl[1] );
	int res;
	if( recv( ctrl[0], &res, sizeof( int ), 0 ) < sizeof( int ) )
		res = -1;
	close( ctrl[0] );
	return res;
}
