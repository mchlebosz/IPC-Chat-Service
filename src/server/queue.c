#include "queue.h"

#include "session.h"

void serve(int* keep_running, int* msgid, FILE* db) {
	if (*msgid < 0) {
		perror("msgget");
		exit(1);
	}

	while (*keep_running) {
		Message msg;

		ssize_t msg_size = msgrcv(*msgid, &msg, sizeof(msg), 0, 0);
		if (msg_size < 0) {
			perror("msgrcv");
			continue;
		}
		if (msg_size > sizeof(Message)) {
			printf(
				"Error: message size exceeds buffer size, discarding message\n");
			continue;
		}
		// handle the received message
		for (int i = 0; i < TYPE_CODES; i++) {
			if (msg.header.type == typeCodes[i].code) {
				printf("Received %s request from %s at %s: %s\n",
					   typeCodes[i].name, msg.header.sender, msg.header.time,
					   msg.body);
				break;
			}
		}

		// when receive connection request
		if (msg.header.type = 0) {
			// extract client id from message
			key_t clientKey = msg.body;
			// create session for new client
			int sessionKey = openSession(clientKey);
			// on Error return 500 to client
			if (sessionKey == 500) {
				// send response message
				char* receiver = msg.header.sender;

				msgInit(&msg, 1, "server", receiver, 500,
						"Internal Server Error (Session)");

				// connect to client queue
				int clientQueue = msgget(clientKey, 0666 | IPC_CREAT);
				// send response message
				msgsnd(clientQueue, &msg, sizeof(msg), 0);
				continue;
			}
		}
		// check if the message is a register request
		if (msg.header.type == 11) {
			// check if the user is already registered
			if (isRegistered(msg.header.sender)) {
				// send response message
				msg.header.type       = 1;
				msg.header.statusCode = 409;
				msgsnd(*msgid, &msg, sizeof(msg), 0);
				continue;
			}
			// register the user
			registerUser(msg.header.sender, msg.body);
		}

		// send response message
		msg.header.type       = 34;
		msg.header.statusCode = 200;
		msgsnd(*msgid, &msg, sizeof(msg), 0);
	}
}

int openSession(key_t clientKey) {
	// create the message queue for session, then in session connect to
	// clientQueue create random session key based on client key
	key_t sessionKey = ftok(clientKey, rand());
	// create session queue
	int sessionQueue = msgget(sessionKey, 0666 | IPC_CREAT);
	// create session subprocess
	int pid = fork();

	if (pid == 0) {
		// send session key to client
		//  connect to client queue
		int clientQueue = msgget(clientKey, 0666 | IPC_CREAT);
		Message msg;
		msgInit(&msg, 1, "session", msg.header.sender, 200, sessionKey);
		// send response message
		msgsnd(clientQueue, msg, sizeof(sessionKey), 0);

		// serve session
		session(sessionKey, sessionQueue);

	} else {
		return sessionKey;
	}
	return 500;
}