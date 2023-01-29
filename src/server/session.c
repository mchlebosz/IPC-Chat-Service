#include "session.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../utils.h"
#include "../message.h"

void session(int* keep_running, key_t sessionKey) {
	// connect to session queue
	int serverQueue = msgget(hash("server"), 0666);
	int sessionQueue = msgget(sessionKey, 0666 | IPC_CREAT);
	printf("queue %d ready! key=%d\n", sessionQueue, sessionKey);
	Message msg;
	while (*keep_running) {
		// receive message from client
		int status = receiveMessage(&sessionQueue, &msg, 32);
		// print status
		printf("Status: %d\n", status);
		// place to handle messages from client
		//  pass message to server with msg.mtype = 21
		msg.mtype = 21;
		msgsnd(serverQueue, &msg, sizeof(msg), 0);

		// receive message from server
		status = receiveMessage(&sessionQueue, &msg, 12);
		printf("Status: %d\n", status);
		// place to handle messages from server
		//  pass message to client with msg.mtype = 23
		msg.mtype = 23;
		msgsnd(sessionQueue, &msg, sizeof(msg), 0);
	}
}

int openSession(int* sessionRunning, int* sessionQueue, char* clientID,
				int clientSeed, key_t* sessionKey, int* sessionPID) {
	// create the message queue for session, then in session connect to
	// clientQueue create random session key based on client key
	int sessionSeed = rand();
	*sessionKey     = createSessonKey(clientSeed, sessionSeed);
	// create session queue
	*sessionQueue   = msgget(*sessionKey, 0666 | IPC_CREAT);
	*sessionRunning = 1;
	// create session subprocess
	*sessionPID = fork();
	if (*sessionPID == 0) {
		// send session key to client
		//  connect to client queue
		key_t clientKey = hash(clientID);
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
		session(sessionRunning, *sessionKey);

	} else {
		return 200;
	}
	*sessionRunning = 0;
	return 500;
}

void addSession(Sessions* sessions, Session* session) {
	// increase sessions list size
	sessions->size++;
	// allocate memory for new session
	sessions->sessions =
		realloc(sessions->sessions, sessions->size * sizeof(Session));
	// add new session to sessions list
	sessions->sessions[sessions->size - 1] = *session;
}

void removeSession(Sessions* sessions, char* sessionID) {
	// find session in sessions list
	for (int i = 0; i < sessions->size; i++) {
		if (sessions->sessions[i].sessionID == sessionID) {
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

int getSessionQueue(Sessions* sessions, const char* sessionID) {
	// find session in sessions list
	for (int i = 0; i < sessions->size; i++) {
		printf("id: '%s', dst: '%s'\n", sessions->sessions[i].sessionID,
			   sessionID);
		if (strcmp(sessions->sessions[i].sessionID, sessionID) == 0) {
			return sessions->sessions[i].sessionQueue;
		}
	}
	return 404;
}