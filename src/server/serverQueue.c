#include "serverQueue.h"

#include "dbHandler.h"
#include "session.h"

void serve(int* keep_running, int* msgid, char* db) {
	if (*msgid < 0) {
		perror("msgget");
		exit(1);
	}

	// create sessions list
	Sessions sessions;
	sessions.size     = 0;
	sessions.sessions = NULL;

	while (*keep_running) {
		Message msg;

		// receive message from client
		if (receiveMessage(msgid, &msg, 31) != 200) {
			continue;
		}

		// New Connection
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
			key_t sessionKey;
			int sessionPID;
			// on Error return 500 to client
			if (openSession(&sessionRunning, &sessionQueue, clientID,
							clientSeed, &sessionKey, &sessionPID) == 500) {
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
				Session session = { clientID, sessionRunning, sessionKey,
									sessionQueue, sessionPID };
				addSession(&sessions, session);

				printf("Session added for client %s with sessionKey: %d\n",
					   clientID, sessionKey);
			}
		}

		// Receive Message from Session
		if (receiveMessage(msgid, &msg, 21) != 200) {
			continue;
		}
		// Register User
		if (msg.mtext.header.type == 11) {
			// extract username and password from message
			// message format: username;password;
			char* messageBody = msg.mtext.body;
			char* username    = strtok(messageBody, ";");
			char* password    = strtok(NULL, ";");

			char* decryptKey;

			// register user
			int status = registerUser(username, password, &decryptKey, db);
			// check status
			if (status == 409) {
				// send response message
				char* receiver = msg.mtext.header.sender;

				msgInit(&msg, 12, 1, "server", receiver, status,
						"Conflict (User already exists)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, receiver);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else if (status == 200) {
				// send response message
				// message format: login;password;decryptKey;
				char* receiver = msg.mtext.header.sender;

				char* body = malloc(100 * sizeof(char));
				// add decrypt key to message body
				sprintf(body, "%s;%s;%s;", username, password, decryptKey);

				msgInit(&msg, 12, 1, "server", receiver, status, body);

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, receiver);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else {
				// internal server error
				// send response message
				char* receiver = msg.mtext.header.sender;
				msgInit(&msg, 12, 1, "server", receiver, 500,
						"Internal Server Error (Register)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, receiver);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
			}
		}
		// Login User
		if (msg.mtext.header.type == 10) {
			// extract username and password from message
			// message format: username;password;
			char* messageBody = msg.mtext.body;
			char* username    = strtok(messageBody, ";");
			char* password    = strtok(NULL, ";");

			char* decryptKey;

			// login user
			int status = loginUser(username, password, &decryptKey, db);
			// check status
			if (status == 401) {
				// send response message
				char* receiver = msg.mtext.header.sender;

				msgInit(&msg, 12, 1, "server", receiver, status,
						"Unauthorized (Wrong username or password)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, receiver);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else if (status == 200) {
				// send response message
				// message format: login;password;decryptKey;
				char* receiver = msg.mtext.header.sender;

				char* body = malloc(100 * sizeof(char));
				// add decrypt key to message body
				sprintf(body, "%s;%s;%s;", username, password, decryptKey);

				msgInit(&msg, 12, 1, "server", receiver, status, body);

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, receiver);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else if (status == 404) {
				// send response message
				char* receiver = msg.mtext.header.sender;

				msgInit(&msg, 12, 1, "server", receiver, status,
						"Not Found (User does not exist)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, receiver);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else {
				// internal server error
				// send response message
				char* receiver = msg.mtext.header.sender;
				msgInit(&msg, 12, 1, "server", receiver, 500,
						"Internal Server Error (Login)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, receiver);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
			}
		}
		// Send Message to User
		if (msg.mtext.header.type == 24) {
			// extract message body
			// message format: message;
			char* messageBody = msg.mtext.body;
			char* sender      = msg.mtext.header.sender;
			char* receiver    = msg.mtext.header.receiver;

			// check if receiver exists
			int status = searchData(db, receiver);
			if (status == 200) {
				// send message to receiver
				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, receiver);
				// send response message
				msgInit(&msg, 12, 34, sender, receiver, 200, messageBody);

				msgsnd(sessionQueue, &msg, sizeof(msg), 0);

				continue;
			} else if (status == 404) {
				// internal server error - error opening database
				//  send response message to sender
				msgInit(&msg, 12, 1, "server", sender, 500,
						"Internal Server Error (Send Message)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, sender);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
			} else if (status == 204) {
				// user not found
				// send response message to sender
				msgInit(&msg, 12, 1, "server", sender, 404,
						"Not Found (User does not exist)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, sender);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
			}

			// transfer message to session
		}
	}
	// close all sessions
	for (int i = 0; i < sessions.size; i++) {
		// end session queues
		msgctl(sessions.sessions[i].sessionQueue, IPC_RMID, NULL);
		printf("Session %d queue closed\n", sessions.sessions[i].sessionQueue);
		// end session processes
		kill(sessions.sessions[i].sessionPID, SIGKILL);
		printf("Session %d ended\n", sessions.sessions[i].sessionPID);
	}
	printf("Closing server\n");

	// remove all dynamic memory
	free(sessions.sessions);
}

int registerUser(char* username, char* password, char** key, char* db) {
	// check if username is already taken
	if (searchData(db, username) == 200) {
		return 409;
	} else {
		// generate decryption key
		*key = generateKey();
		// encrypt password
		// char* encryptedPassword = encrypt(password, key);
		// add user to database
		// format: username;password;key;
		char* data;
		data = strcat(username, ";");
		data = strcat(data, password);
		data = strcat(data, ";");
		data = strcat(data, *key);
		data = strcat(data, ";");
		addData(db, data);
		return 200;
	}
}

char* generateKey() {
	// generate random key
	char* key = malloc(16 * sizeof(char));
	for (int i = 0; i < 16; i++) {
		key[i] = (char)(rand() % 26 + 65);
	}
	return key;
}

int loginUser(char* username, char* password, char** key, char* db) {
	// check if username exists
	if (searchData(db, username) == 200) {
		// get user data
		char* data = malloc(100 * sizeof(char));
		int status = getData(db, username, &data);

		if (status != 200) {
			return status;
		}

		// get user password
		char* userPassword = strtok(data, ";");
		userPassword       = strtok(NULL, ";");
		// get user key
		*key = strtok(NULL, ";");
		// compare passwords
		if (strcmp(password, userPassword) == 0) {
			return 200;
		} else {
			return 401;
		}
	} else {
		return 404;
	}
}