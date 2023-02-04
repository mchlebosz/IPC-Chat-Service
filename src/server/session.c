#include "session.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// include random
#include <time.h>

#include "../message.h"
#include "../utils.h"

/**
 * It receives messages from the client, passes them to the server, receives
 * messages from the server, and passes them to the client
 *
 * @param keep_running a pointer to a variable that is set to 0 when the program
 * should exit
 * @param sessionKey the key of the session queue
 */
void session(int* keep_running, key_t sessionKey) {
	// connect to session queue
	int serverQueue  = msgget(hash("server"), 0666);
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

/**
 * It creates a session for a client, and returns the session key to the client
 *
 * @param sessionRunning a pointer to an integer that is used to determine if
 * the session is running or not.
 * @param sessionQueue the queue for the session
 * @param clientID the client's ID
 * @param clientSeed the seed used to generate the client's key
 * @param sessionKey the key for the session queue
 * @param sessionPID the PID of the session process
 *
 * @return The return value is the status code of the session.
 */
int openSession(int* sessionRunning, int* sessionQueue, char* clientID,
				int clientSeed, key_t* sessionKey, int* sessionPID) {
	// create the message queue for session, then in session connect to
	// clientQueue create random session key based on client key
	srand(time(0));

	// To prevent overflow make modulo of X
	int sessionSeed = rand() % 1000000;

	*sessionKey = createSessonKey(clientSeed, sessionSeed);

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
		char* MessageBody = malloc(1000 * sizeof(char));
		sprintf(MessageBody, "%s;%d;", clientID, sessionSeed);

		msgInit(&msg, 23, 1, "session", clientID, 200, MessageBody);
		// send response message
		msgsnd(clientQueue, &msg, sizeof(msg), 0);

		// serve session
		session(sessionRunning, *sessionKey);

	} else {
		printf(
			"Session created for: %s, with PID: %d, and Queue: %d, and Key: %d, with seed: %d, and running is:%d\n",
			clientID, *sessionPID, *sessionQueue, *sessionKey, sessionSeed,
			*sessionRunning);
		return 200;
	}
	*sessionRunning = 0;
	return 500;
}

/**
 * It adds a session to the sessions list
 *
 * @param sessions a pointer to a Sessions struct
 * @param session The session to add to the sessions list.
 */
void addSession(Sessions* sessions, Session* session) {
	// increase sessions list size
	sessions->size++;
	// allocate memory for new session
	sessions->sessions =
		realloc(sessions->sessions, sessions->size * sizeof(Session));
	// add new session to sessions list
	sessions->sessions[sessions->size - 1] = *session;
}

/**
 * It removes a session from the sessions list
 *
 * @param sessions a pointer to the Sessions struct
 * @param clientID The client ID of the session to be removed.
 */
void removeSession(Sessions* sessions, char* clientID) {
	// find session in sessions list
	for (int i = 0; i < sessions->size; i++) {
		if (sessions->sessions[i].clientID == clientID) {
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

/**
 * It takes a pointer to a
 * `Sessions` struct and a `clientID` string, and returns the queue number of
 * the session with the given `clientID`
 *
 * @param sessions a pointer to the Sessions struct
 * @param clientID the client ID of the session
 *
 * @return The session queue of the session with the given clientID.
 */
int getSessionQueue(Sessions* sessions, const char* clientID) {
	// find session in sessions list
	for (int i = 0; i < sessions->size; i++) {
		printf("id: '%s', dst: '%s'\n", sessions->sessions[i].clientID,
			   clientID);
		if (strcmp(sessions->sessions[i].clientID, clientID) == 0) {
			return sessions->sessions[i].sessionQueue;
		}
	}
	return 404;
}

/**
 * It checks if a session is running by checking if the clientID is in the
 * sessions list
 *
 * @param sessions a pointer to the Sessions struct
 * @param clientID the client ID of the client that is trying to connect
 *
 * @return The status code of the session.
 */
int isSessionRunning(Sessions* sessions, const char* clientID) {
	// find session in sessions list
	for (int i = 0; i < sessions->size; i++) {
		if (strcmp(sessions->sessions[i].clientID, clientID) == 0) {
			return 200;
		}
	}
	return 404;
}

/**
 * It takes a sessions list, a clientID, and a pointer to an int, and it sets
 * the int to the userID of the user logged in to the session with the given
 * clientID
 *
 * @param sessions a pointer to the Sessions struct
 * @param clientID The client ID of the session you want to get the user ID of.
 * @param userID the userID of the user that is logged in
 *
 * @return the userID of the user logged in to the session with the given
 * clientID.
 */
int getSessionUserID(Sessions* sessions, const char* clientID, int* userID) {
	// find session in sessions list
	for (int i = 0; i < sessions->size; i++) {
		if (strcmp(sessions->sessions[i].clientID, clientID) == 0) {
			*userID = sessions->sessions[i].userLoggedInID;
			return 200;
		}
	}
	return 404;
}

/**
 * It finds the session with the given clientID and sets the userID to the given
 * userID
 *
 * @param sessions pointer to the Sessions struct
 * @param clientID The client ID of the session you want to set the user ID for.
 * @param userID the userID of the user that is logged in
 *
 * @return the userID of the user logged in to the session.
 */
int setSessionUserID(Sessions* sessions, const char* clientID, int userID) {
	// find session in sessions list
	for (int i = 0; i < sessions->size; i++) {
		if (strcmp(sessions->sessions[i].clientID, clientID) == 0) {
			sessions->sessions[i].userLoggedInID = userID;
			return 200;
		}
	}
	return 404;
}