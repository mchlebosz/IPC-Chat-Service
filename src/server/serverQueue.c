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
		if (receiveMessageNoWait(msgid, &msg, 31) != 200) {
			if (receiveMessageNoWait(msgid, &msg, 21) != 200) {
				usleep(1500);
				continue;
			}
		}

		// New Connection
		if (msg.mtext.header.type == 0) {
			// extract client id and seed from message
			// message format: clientID;clientSeed;
			char* messageBody = msg.mtext.body;
			char* clientID    = strtok(messageBody, ";");
			int clientSeed    = atoi(strtok(NULL, ";"));

			key_t clientKey = hash(clientID);

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

				// add session to sessions list
				printf(
					"Session created for %s with PID %d and Queue %d and Key %d and running %d\n",
					clientID, sessionPID, sessionQueue, sessionKey,
					sessionRunning);

				srand(time(NULL));
				Session session = { "", sessionRunning, sessionKey,
									sessionQueue, sessionPID };
				strcpy(session.sessionID, clientID);
				addSession(&sessions, &session);

			}
		}
		// Receive Message from Session
		
		// Register User
		else if (msg.mtext.header.type == 11) {
			// extract username and password from message
			// message format: username;password;
			char* messageBody = msg.mtext.body;
			char* username    = strtok(messageBody, ";");
			char* password    = strtok(NULL, ";");

			char* decryptKey;

			char receiver[32]; strcpy(receiver, msg.mtext.header.sender);

			char sender[32]; strcpy(sender, msg.mtext.header.sender);

			int sessionQueue = getSessionQueue(&sessions, receiver);

			printf("registering %s with pswd %s...!\n", username, password);

			// register user
			int status = registerUser(username, password, &decryptKey, db);
			printf("register status: %d\n", status);
			// check status
			if (status == 409) {
				// send response message
				printf("session queue: %d\n", sessionQueue);
				msgInit(&msg, 12, 1, "server", receiver, 409,
						"Conflict (User already exists)");
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else if (status == 200) {
				// send response message
				// message format: login;password;decryptKey;
				char body[128];
				// add decrypt key to message body
				sprintf(body, "%s;%s;%s;", username, password, decryptKey);
				msgInit(&msg, 12, 1, "server", receiver, 200, body);

				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else {
				// internal server error
				// send response message
				msgInit(&msg, 12, 1, "server", receiver, 500,
						"Internal Server Error (Register)");

				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
			}
		}
		// Login User
		else if (msg.mtext.header.type == 10) {
			// extract username and password from message
			// message format: username;password;
			char* messageBody = msg.mtext.body;
			char* username    = strtok(messageBody, ";");
			char* password    = strtok(NULL, ";");

			char* decryptKey;

			char receiver[32]; 
			strcpy(receiver, msg.mtext.header.sender);
			char sender[32]; 
			strcpy(sender, msg.mtext.header.sender);
			// connect to session queue
			int sessionQueue = getSessionQueue(&sessions, receiver);

			// login user
			int status = loginUser(username, password, &decryptKey, db);
			// check status
			if (status == 401) {
				// send response message
				msgInit(&msg, 12, 1, "server", sender, 401,
						"Unauthorized (Wrong username or password)");

				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else if (status == 200) {
				// send response message
				// message format: login;password;decryptKey;
				char body[100];
				// add decrypt key to message body
				sprintf(body, "%s;%s;%s;", username, password, decryptKey);

				
				msgInit(&msg, 12, 1, "server", sender, 200, body);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else if (status == 404) {
				// send response message
				msgInit(&msg, 12, 1, "server", sender, 404,
						"Not Found (User does not exist)");

				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else {
				// internal server error
				// send response message
				msgInit(&msg, 12, 1, "server", receiver, 500,
						"Internal Server Error (Login)");

				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
			}
		}
		// Send Message to User
		else if (msg.mtext.header.type == 24) {
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
		char data[128];
		sprintf(data, "%s;%s;%s;", username, password, *key);

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