#ifndef GETRQ_H
#define GETRQ_H

struct getrq {
	int ctrl;
	int fd;
	char url[1024];
};

int getrq_enable();
void getrq_disable();
struct getrq* getrq_recv();
void getrq_wipe( struct getrq *r );

#endif
