#include "session.h"

#include "../message.h"

void session(int* keep_running, key_t sessionKey) {
	// connect to session queue
	int sessionQueue = msgget(sessionKey, 0);
	while (*keep_running) {
		// receive message from client
		Message msg;
		ssize_t msg_size = msgrcv(sessionQueue, &msg, sizeof(msg), 31, 0);
		if (msg_size < 0) {
			perror("msgrcv");
			continue;
		}
	}
}