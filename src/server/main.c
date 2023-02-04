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

/**
 * It sets the global variable `keep_running` to 0
 *
 * @param signum The signal number.
 */
void sigint_handler(int signum) {
	keep_running = 0;
}

/**
 * It creates a message queue, then calls the `serve` function, which loops
 * forever, waiting for messages to arrive, and then processing them
 *
 * @param argc the number of arguments passed to the program
 * @param argv the command line arguments
 *
 * @return The server key.
 */
int main(int argc, char *argv[]) {
	srand(time(NULL));
	printf("Server started\n");
	// open DB - create if it doesn't exist
	char *dbPath = "serverdb.json";
	FILE *db     = openDB(dbPath, "a+");
	fclose(db);

	// register signal handler
	signal(SIGINT, sigint_handler);
	// create the message queue
	const char *path = "server";
	key_t serverkey  = hash(path);

	int msgid = msgget(serverkey, 0666 | IPC_CREAT);

	printf("Server key: %d\n", serverkey);

	if (msgid == -1) {
		printf("Error creating message queue\n");
		return 1;
	}

	serve(&keep_running, &msgid, dbPath);

	// remove the message queue
	msgctl(msgid, IPC_RMID, NULL);

	return 0;
}