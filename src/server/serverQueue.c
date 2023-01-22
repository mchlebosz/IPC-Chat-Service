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

		if (msg.mtext.header.type == 0) {
			// extract client id and seed from message
			// message format: clientID;clientSeed;
			char* messageBody = msg.mtext.body;
			char* clientID    = strtok(messageBody, ";");
			int clientSeed    = atoi(strtok(NULL, ";"));

			key_t clientKey = ftok(clientID, clientSeed);

			int sessionQueue;
			int sessionRunning;
			// create session for new client
			int sessionKey = openSession(&sessionRunning, &sessionQueue,
										 clientID, clientSeed);
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
	}
}

int openSession(int* sessionRunning, int* sessionQueue, char* clientID,
				int clientSeed) {
	// create the message queue for session, then in session connect to
	// clientQueue create random session key based on client key
	int sessionSeed  = rand();
	key_t sessionKey = ftok(clientID, sessionSeed);
	// create session queue
	*sessionQueue   = msgget(sessionKey, 0666 | IPC_CREAT);
	*sessionRunning = 1;
	// create session subprocess
	if (fork() == 0) {
		// send session key to client
		//  connect to client queue
		key_t clientKey = ftok(clientID, clientSeed);
		int clientQueue = msgget(clientKey, 0666 | IPC_CREAT);
		Message msg;
		// send response message
		// message format: clientID;sessionSeed;
		char* MessageBody = strcat(clientID, ";");
		char stringSessionSeed[10];
		sprintf(stringSessionSeed, "%d", sessionSeed);
		MessageBody = strcat(MessageBody, stringSessionSeed);
		MessageBody = strcat(MessageBody, ";");

		msgInit(&msg, 21, 1, "session", msg.mtext.header.sender, 200,
				MessageBody);
		// send response message
		msgsnd(clientQueue, &msg, sizeof(msg), 0);

		// serve session
		session(sessionRunning, sessionKey);

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