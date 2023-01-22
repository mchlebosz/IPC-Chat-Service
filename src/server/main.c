#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "communication.h"

int keep_running = 1;

void sigint_handler(int signum) {
	keep_running = 0;
}

int main(int argc, char *argv[]) {
	// register signal handler
	signal(SIGINT, sigint_handler);
	// create the message queue
	key_t key = ftok("server.c", 'B');
	int msgid = msgget(key, 0666 | IPC_CREAT);

	serve(keep_running, msgid);

	// remove the message queue
	msgctl(msgid, IPC_RMID, NULL);

	return 0;
}