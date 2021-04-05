#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "protocol.h"

void strtolower(char * str);
void getfifo();
void send();

char fifo[64];
char msg[COMBUFSIZ];
int fd;
char * pid;

void strtolower(char * str) {
	for (;*str;++str) {
		tolower(*str);
	}
}

void getfifo() {
	sprintf(fifo, "/tmp/dui/%s", pid);
}

void send() {
	fprintf(stderr, "%s\n", msg);
	kill(atoi(pid), SIGUSR1);

	fd = open(fifo, O_WRONLY);
	write(fd, msg, strlen(msg)+1);
	close(fd);
}

int main(int argc, char ** argv) {
	if (argc < 3) {
		fprintf(stderr, "wcom [pid] [command]\n");
		return -1;
	}

	pid = argv[1];
	getfifo();

	strtolower(argv[2]);

	if (strcmp(argv[2], "debugecho") == 0) {
		if (argc != 4) {
			fprintf(stderr, "wcom [pid] debugecho [text]");
			return -1;
		}
		sprintf(msg, "%i;%s;", DEBUGECHO, argv[3]);
	} else if (strcmp(argv[2], "attach") == 0) {
		if (argc < 6) {
			fprintf(stderr, "wcom [pid] attach [parent] [name] [type]");
			return -1;
		}

		strtolower(argv[3]);

		if (strcmp(argv[5], "label") == 0) {
			if (argc != 7) {
				fprintf(stderr, "wcom [pid] attach [parent] [name] label [text]");
				return -1;
			}
			sprintf(msg, "%i;%s;%s;%i;%s;", ATTACH, argv[3], argv[4], LABEL, argv[6]);
		} else if (strcmp(argv[5], "button") == 0) {
			if (argc != 7) {
				fprintf(stderr, "wcom [pid] attach [parent] [name] button [text]");
				return -1;
			}
			sprintf(msg, "%i;%s;%s;%i;%s;", ATTACH, argv[3], argv[4], BUTTON, argv[6]);
		} else if (strcmp(argv[5], "hbox") == 0) {
			if (argc != 7) {
				fprintf(stderr, "wcom [pid] attach [parent] [name] hbox [homogeneous]");
				return -1;
			}
			sprintf(msg, "%i;%s;%s;%i;%s;", ATTACH, argv[3], argv[4], HBOX, argv[6]);
		} else if (strcmp(argv[5], "vbox") == 0) {
			if (argc != 7) {
				fprintf(stderr, "wcom [pid] attach [parent] [name] vbox [homogeneous]");
				return -1;
			}
			sprintf(msg, "%i;%s;%s;%i;%s;", ATTACH, argv[3], argv[4], VBOX, argv[6]);
		}
	}

	send();

	return 0;
}
