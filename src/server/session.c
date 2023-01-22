#include "session.h"

#include "../message.h"

void session(int* keep_running, key_t sessionKey) {
	// connect to session queue
	int sessionQueue = msgget(sessionKey, 0);
	Message msg;
	while (*keep_running) {
		// receive message from client
		int status = receiveMessage(&sessionQueue, &msg, 32);
		// print status
		printf("Status: %d\n", status);
		// place to handle messages from client
		//  pass message to server with msg.mtype = 21
		msg.mtype = 21;
		msgsnd(sessionQueue, &msg, sizeof(msg), 0);

		// receive message from server
		status = receiveMessage(&sessionQueue, &msg, 12);
		printf("Status: %d\n", status);
		// place to handle messages from server
		//  pass message to client with msg.mtype = 23
		msg.mtype = 23;
		msgsnd(sessionQueue, &msg, sizeof(msg), 0);
	}
}