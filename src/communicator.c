#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "protocol.h"

int main(int argc, char ** argv) {
	char fifo[64];
	int fd;

	sprintf(fifo, "/tmp/dui/%s", argv[1]);

	kill(atoi(argv[1]), atoi(argv[2]));

	char arr1[COMBUFSIZ];
	fgets(arr1, COMBUFSIZ, stdin);

	fd = open(fifo, O_WRONLY);

	write(fd, arr1, strlen(arr1)+1);
	close(fd);

	return 0;
}
