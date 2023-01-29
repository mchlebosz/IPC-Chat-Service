#include "communication.h"

int clientQueueID;

void APICreateConnection(void) {
	int sessionId = rand();
	char name[5];
	sprintf(name, "p%d", sessionId);
	int clientKey = ftok(name, sessionId);
	clientQueueID = msgget(clientKey, 0666 | IPC_CREAT);
	Message msg;
	msg.mtype             = 31;
	msg.mtext.header.type = 0;
	sprintf(msg.mtext.body, "%s;%d;", name, sessionId);

	const char* path      = "server";

	key_t serverKey = hash(path);
	printf("serverKey: %d\n", serverKey);
	int serverSessionId = msgget(serverKey, 0666 | IPC_CREAT);
	msgsnd(serverSessionId, &msg, sizeof(msg), 0);

	msgrcv(clientQueueID, &msg, sizeof(msg), 2, 0);

	if (msg.mtext.header.type == 1) {
		printf("Connection established\n");
	} else {
		printf("Connection failed\n");
	}
}
// login
Message APILogin(char* username, char* password) {
	// send username and password to server
	// receive response from server
	// if response is success, return username auth token
	// else return error code
	Message response;
	response.mtext.header.type = 10;
	return response;
}
// register
Message APIRegister(char* username, char* password) {
	// send username and password to server
	// receive response from server
	// if response is success, return username auth token
	// else return error code

	Message response;
	response.mtext.header.type = 11;

	return response;
}
// logout
Message APILogout(char* username) {
	// send username to server
	// receive response from server
	// if response is success, show login interface
	// else, show error message
	Message response;
	response.mtext.header.type = 12;

	return response;
}
