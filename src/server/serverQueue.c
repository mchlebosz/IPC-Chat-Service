#include "serverQueue.h"

#include "session.h"

void serve(int* keep_running, int* msgid, FILE* db) {
	if (*msgid < 0) {
		perror("msgget");
		exit(1);
	}

	// create sessions list
	Sessions sessions;

	while (*keep_running) {
		Message msg;

		// receive message from client
		ssize_t msg_size = msgrcv(*msgid, &msg, sizeof(msg), 31, 0);
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
			if (msg.mtext.header.type == typeCodes[i].code) {
				printf("Received %s request from %s at %s: %s\n",
					   typeCodes[i].name, msg.mtext.header.sender,
					   msg.mtext.header.time, msg.mtext.body);
				break;
			}
		}

		if (msg.mtext.header.type = 0) {
			// extract client id from message
			key_t clientKey = msg.mtext.body;

			int sessionQueue;
			int sessionRunning;
			// create session for new client
			int sessionKey =
				openSession(&sessionRunning, &sessionQueue, clientKey);
			// on Error return 500 to client
			if (sessionKey == 500) {
				// send response message
				char* receiver = msg.mtext.header.sender;

				msgInit(&msg, 13, 1, "server", receiver, 500,
						"Internal Server Error (Session)");

				// connect to client queue
				int clientQueue = msgget(clientKey, 0666 | IPC_CREAT);
				// send response message
				msgsnd(clientQueue, &msg, sizeof(msg), 0);
				continue;
			} else {
				// add session to sessions list
				Session session = { sessionRunning, sessionKey, sessionQueue };
				addSession(&sessions, session);
			}
		}

		// check if the message is a register request
		if (msg.mtext.header.type == 11) {
			// check if the user is already registered
			if (isRegistered(msg.mtext.header.sender)) {
				// send response message
				msg.mtext.header.type       = 1;
				msg.mtext.header.statusCode = 409;
				msgsnd(*msgid, &msg, sizeof(msg), 0);
				continue;
			}
			// register the user
			registerUser(msg.mtext.header.sender, msg.mtext.body);
		}
	}
}

int openSession(int* sessionRunning, int* sessionQueue, key_t clientKey) {
	// create the message queue for session, then in session connect to
	// clientQueue create random session key based on client key
	key_t sessionKey = ftok(clientKey, rand());
	// create session queue
	*sessionQueue   = msgget(sessionKey, 0666 | IPC_CREAT);
	*sessionRunning = 1;
	// create session subprocess
	if (fork() == 0) {
		// send session key to client
		//  connect to client queue
		int clientQueue = msgget(clientKey, 0666 | IPC_CREAT);
		Message msg;
		msgInit(&msg, 21, 1, "session", msg.mtext.header.sender, 200,
				sessionKey);
		// send response message
		msgsnd(clientQueue, &msg, sizeof(msg), 0);

		// serve session
		session(sessionRunning, sessionKey, *sessionQueue);

	} else {
		return sessionKey;
	}
	*sessionRunning = 0;
	return 500;
}

void addSession(Sessions* sessions, Session session) {
	// increase sessions list size
	sessions->size++;
	// allocate memory for new session
	sessions->sessions =
		realloc(sessions->sessions, sessions->size * sizeof(Session));
	// add new session to sessions list
	sessions->sessions[sessions->size - 1] = session;
}

void removeSession(Sessions* sessions, key_t sessionKey) {
	// find session in sessions list
	for (int i = 0; i < sessions->size; i++) {
		if (sessions->sessions[i].sessionKey == sessionKey) {
			// remove session from sessions list
			for (int j = i; j < sessions->size - 1; j++) {
				sessions->sessions[j] = sessions->sessions[j + 1];
			}
			// decrease sessions list size
			sessions->size--;
			// reallocate memory for sessions list
			sessions->sessions =
				realloc(sessions->sessions, sessions->size * sizeof(Session));
			break;
		}
	}
}