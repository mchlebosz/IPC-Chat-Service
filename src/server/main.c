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

#include "dbHandler.h"
#include "serverQueue.h"

int keep_running = 1;

void sigint_handler(int signum) {
	keep_running = 0;
}

int main(int argc, char *argv[]) {
	// open DB - create if it doesn't exist
	FILE *db = openFile("serverdb.txt", "a+");
	fclose(db);

	// register signal handler
	signal(SIGINT, sigint_handler);
	// create the message queue
	char *path      = "server";
	int id          = 'B';
	key_t serverkey = ftok(path, id);

	int msgid = msgget(serverkey, 0666 | IPC_CREAT);

	serve(&keep_running, &msgid, "serverdb.txt");

	// remove the message queue
	msgctl(msgid, IPC_RMID, NULL);

	return 0;
}