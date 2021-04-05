#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <gtk/gtk.h>

#include "protocol.h"

int in_pipe[2];
int fd;
char fifo[64];
struct stat st = {0};

GtkApplication * app;

struct HLE;

void handle_termination(int sig);
void handle_usrint(int sig);
void process_input(char *);
void debug(char **);
void attach(char **);
struct HLE * get_HLE(struct HLE *, char *);
void run();
void activate(GtkApplication * app, gpointer user_data);
void cleanup(struct HLE *);
void send_ready();
void send_action(char *);
void button_clicked(GtkButton *);
struct HLE * get_HLE_with_widget(struct HLE *, GtkWidget *);

struct HLE {
	GtkWidget * widget;
	struct HLE * chh;
	struct HLE * prev;
	struct HLE * next;
	char name[32];
} root;

void handle_termination(int sig) {
	fprintf(stderr, "Caught signal %d\n", sig);

	gtk_window_close(GTK_WINDOW(root.widget));
	send_action("quit");

	unlink(fifo);
	cleanup(root.chh);
}

void handle_usrint(int sig) {
	fprintf(stderr, "Caught signal %d\n", sig);

	char msg[COMBUFSIZ];

	fd = open(fifo, O_RDONLY);
	read(fd, msg, COMBUFSIZ);

	process_input(msg);

	close(fd);
}

void process_input(char * input) {
	char * arg[ARGSIZ] = {0};
	char * c;
	char * last;
	int action;
	int i = 0;

	for (c = input; *c != ';'; ++c);

	*c = 0;
	++c;
	last = c;

	for (; *c != 0; ++c) {
		if (*c == ';') {
			*c = 0;
			arg[i] = last;
			last = c + 1;
			++i;
		}
	}

	action = atoi(input);

	switch (action) {
		case SET: break;
		case GET: break;
		case ATTACH: attach(arg); break;
		case REMOVE: break;
		case DEBUGECHO: debug(arg); break;
		default: fprintf(stderr, "WARNING: Event %i does not exist!\n", action); break;
	}

}

void debug(char ** arg) {
	fprintf(stderr, "DEBUG: %s\n", arg[0]);
}

void attach(char ** arg) {
	struct HLE * parent;
	struct HLE * current;
	char * path[MAXDEPTH] = {0};
	char * last = arg[0];
	char * c = arg[0];
	int i = 0;

	for (; *c != 0; ++c) {
		if (*c == '/') {
			*c = 0;
			path[i] = last;
			last = c + 1;
			++i;
		}
	}

	path[i] = last;
	parent = get_HLE(&root, path[0]);

	if (!parent) {
		fprintf(stderr, "Could not find: ");
		for (int x = 0; path[x]; ++x)
			fprintf(stderr, "%s/", path[x]);

		fprintf(stderr, "\n");
		return;
	}

	for (i = 1; i < MAXDEPTH; ++i) {
		if (!path[i])
			break;
		if (parent->chh) {
			parent = get_HLE(parent->chh, path[i]);
		} else {
			fprintf(stderr, "Could not find: ");
			for (int x = 0; path[x]; ++x)
				fprintf(stderr, "%s/", path[x]);

			fprintf(stderr, "\n");
			return;
		}
	}

	if (!parent) {
		fprintf(stderr, "Could not find: ");
		for (int x = 0; path[x]; ++x)
			fprintf(stderr, "%s/", path[x]);

		fprintf(stderr, "\n");
		return;
	}

	current = malloc(sizeof(struct HLE));
	current->next = NULL;
	current->chh = NULL;
	strcpy(current->name, arg[1]);

	int type = atoi(arg[2]);

	if (type == LABEL) {
		current->widget = gtk_label_new(arg[3]);
	} else if (type == BUTTON) {
		current->widget = gtk_button_new_with_label(arg[3]);
		g_signal_connect(current->widget, "clicked", G_CALLBACK(button_clicked), 0);
	} else if (type == HBOX) {
		current->widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_box_set_homogeneous(GTK_BOX(current->widget), atoi(arg[3]));
	} else if (type == VBOX) {
		current->widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_box_set_homogeneous(GTK_BOX(current->widget), atoi(arg[3]));
	} else if (type == SCROLL) {
		// TODO: Add SCROLL
	} else {
		fprintf(stderr, "This Widget code %i does not exist\n", type);
		free(current);
		return;
	}

	gtk_container_add(GTK_CONTAINER(parent->widget), current->widget);
	gtk_widget_show_all(parent->widget);

	if (parent->chh) {
		parent = parent->chh;

		while (parent->next) {
			parent = parent->next;
		}

		parent->next = current;
	} else {
		parent->chh = current;
	}
}

struct HLE * get_HLE(struct HLE * hle, char * path) {
	while (strcmp(hle->name, path)) {
		if (hle->next) {
			hle = hle->next;
		} else {
			return NULL;
		}
	}

	return hle;
}

void send_action(char * message) {
	int fd = open(fifo, O_WRONLY);
	write(fd, message, strlen(message) + 1);
	close(fd);
}

void button_clicked(GtkButton * button) {
	struct HLE * button_hle = get_HLE_with_widget(&root, GTK_WIDGET(button));

	if (button_hle) {
		char message[COMBUFSIZ] = {0};
		strcpy(message, button_hle->name);
		strcat(message, " clicked");
		send_action(message);
	} else {
		fprintf(stderr, "Could not find HLE with widget %i\n", button);
	}
}

struct HLE * get_HLE_with_widget(struct HLE * hle, GtkWidget * widget) {
	if (hle->widget == widget) {
		return hle;
	} else {
		if (hle->chh) {
			struct HLE * c = get_HLE_with_widget(hle->chh, widget);
			if (c) {
				return c;
			}
		}

		if (hle->next) {
			return get_HLE_with_widget(hle->next, widget);
		}
	}

	return NULL;
}

void run() {
	int status;
	app = gtk_application_new("org.dui", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	status = g_application_run(G_APPLICATION(app), 0, NULL);
	g_object_unref(app);
}

void activate(GtkApplication * app, gpointer user_data) {
	root.widget = gtk_application_window_new(app);
	strcpy(root.name, "root");
	gtk_window_set_title(GTK_WINDOW(root.widget), "DUI Window");
	gtk_window_set_default_size(GTK_WINDOW(root.widget), 200, 200);
	gtk_widget_show_all(root.widget);

	send_ready();
}

void send_ready() {
	mkfifo(fifo, 0666);
}

void cleanup(struct HLE * element) {
	if (element) {
		if (element->chh)
			cleanup(element->chh);

		if (element->next)
			cleanup(element->next);

		free(element);
	}
}

int main() {
	signal(SIGUSR1, handle_usrint);
	signal(SIGTERM, handle_termination);
	signal(SIGINT, handle_termination);

	pid_t pid = getpid();

	if (stat("/tmp/dui", &st) == -1)
		mkdir("/tmp/dui", 0700);

	sprintf(fifo, "/tmp/dui/%i", pid);

	run();

	send_action("quit");
	unlink(fifo);

	cleanup(root.chh);

	return 0;
}
