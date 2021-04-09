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

		strtolower(argv[5]);

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
		} else if (strcmp(argv[5], "entry") == 0) {
			if (argc != 6) {
				fprintf(stderr, "wcom [pid] attach [parent] [name] entry");
				return -1;
			}
			sprintf(msg, "%i;%s;%s;%i;%s;", ATTACH, argv[3], argv[4], ENTRY);
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
	} else if (strcmp(argv[2], "get") == 0) {
		if (argc != 5) {
			fprintf(stderr, "wcom [pid] get [path] [type]");
			return -1;
		}

		strtolower(argv[4]);

		if (strcmp(argv[4], "label") == 0) {
			sprintf(msg, "%i;%s;%i;", GET, argv[3], LABEL);
		} else if (strcmp(argv[4], "button") == 0) {
			sprintf(msg, "%i;%s;%i;", GET, argv[3], BUTTON);
		} else if (strcmp(argv[4], "entry") == 0) {
			sprintf(msg, "%i;%s;%i;", GET, argv[3], ENTRY);
		}
	} else if (strcmp(argv[2], "set") == 0) {
		if (argc != 6) {
			fprintf(stderr, "wcom [pid] set [path] [type] [value]");
			return -1;
		}

		strtolower(argv[4]);

		if (strcmp(argv[4], "label") == 0) {
			sprintf(msg, "%i;%s;%i;%s;", SET, argv[3], LABEL, argv[5]);
		} else if (strcmp(argv[4], "button") == 0) {
			sprintf(msg, "%i;%s;%i;%s;", SET, argv[3], BUTTON, argv[5]);
		} else if (strcmp(argv[4], "entry") == 0) {
			sprintf(msg, "%i;%s;%i;%s;", SET, argv[3], ENTRY, argv[5]);
		}
	}


	send();

	return 0;
}
