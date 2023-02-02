#include "serverQueue.h"

/**
 * Main server loop. It waits for messages, and then processes them.
 *
 * @param keep_running a pointer to a variable that will be set to 0 when the
 * server should stop running.
 * @param msgid The message queue identifier.
 * @param db The name of the database file.
 */
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

		// receive message from client or session
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
			if (openSession(&sessionRunning, &sessionQueue, clientID,
							clientSeed, &sessionKey, &sessionPID) == 200) {
				// add session to sessions list
				printf(
					"Session created for %s with PID %d and Queue %d and Key %d and running %d\n",
					clientID, sessionPID, sessionQueue, sessionKey,
					sessionRunning);

				srand(time(NULL));
				Session session = { "", sessionRunning, sessionKey,
									sessionQueue, sessionPID };
				strcpy(session.clientID, clientID);
				addSession(&sessions, &session);

			} else {
				// on Error return 500 to client
				// send response message
				char* receiver = msg.mtext.header.sender;

				msgInit(&msg, 13, 1, "server", receiver, 500,
						"Internal Server Error (Session)");

				// connect to client queue
				int clientQueue = msgget(clientKey, 0666 | IPC_CREAT);
				// send response message
				msgsnd(clientQueue, &msg, sizeof(msg), 0);
				continue;
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

			char receiver[32];
			strcpy(receiver, msg.mtext.header.sender);

			char sender[32];
			strcpy(sender, msg.mtext.header.sender);

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
		// Log out user
		else if (msg.mtext.header.type == 12) {
			// message format: ClientID;
			char* messageBody = msg.mtext.body;
			char* clientID    = strtok(messageBody, ";");

			char receiver[32];
			strcpy(receiver, msg.mtext.header.sender);
			char sender[32];
			strcpy(sender, msg.mtext.header.sender);
			// connect to session queue
			int sessionQueue = getSessionQueue(&sessions, receiver);

			// status check
			int status = isSessionRunning(&sessions, clientID) == 200 &&
								 searchData(db, clientID) == 200 ?
							 200 :
							 404;

			if (status == 200) {
				// send response message
				msgInit(&msg, 12, 12, "server", sender, 200, "OK");

				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				removeSession(&sessions, clientID);
				continue;
			} else if (status == 404) {
				// send response message
				msgInit(&msg, 12, 12, "server", sender, 404,
						"Not Found (User does not exist)");

				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else {
				// internal server error
				// send response message
				msgInit(&msg, 12, 12, "server", receiver, 500,
						"Internal Server Error (Logout)");

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
		// Get list of active users
		else if (msg.mtext.header.type == 13) {
			// message format: ClientID;
			char* messageBody = msg.mtext.body;
			char* clientID    = strtok(messageBody, ";");

			// check if client is logged in
			int status = isSessionRunning(&sessions, clientID);

			// if client is logged in
			if (status == 200) {
				// get list of active users
				char* activeUsers = getOnlineUsers(sessions);

				// if active users is empty
				if (activeUsers == NULL) {
					// send response message
					msgInit(&msg, 12, 13, "server", clientID, 204,
							"No Active Users");

					// connect to session queue
					int sessionQueue = getSessionQueue(&sessions, clientID);
					// send response message
					msgsnd(sessionQueue, &msg, sizeof(msg), 0);
					continue;
				}

				// send response message
				msgInit(&msg, 12, 13, "server", clientID, 200, activeUsers);

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, clientID);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else if (status == 404) {
				// send response message
				msgInit(&msg, 12, 13, "server", clientID, 404,
						"Not Found (User does not exist)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, clientID);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else {
				// internal server error
				// send response message
				msgInit(&msg, 12, 13, "server", clientID, 500,
						"Internal Server Error (Get Active Users)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, clientID);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
			}
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

/**
 * It takes a username, password, and database file name as input, and returns a
 * decryption key and a status code
 *
 * @param username the username of the user
 * @param password the password to be encrypted
 * @param key the decryption key
 * @param db the database file to use
 *
 * @return The return value is the HTTP status code.
 */
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

/**
 * It generates a random key of length 16
 *
 * @return A pointer to a char array.
 */
char* generateKey() {
	// generate random key
	char* key = malloc(16 * sizeof(char));
	for (int i = 0; i < 16; i++) {
		key[i] = (char)(rand() % 26 + 65);
	}
	return key;
}

/**
 * It checks if the username exists, if it does, it gets the user data, gets the
 * user password, compares the passwords, and returns the appropriate status
 * code
 *
 * @param username The username of the user trying to login.
 * @param password The password the user entered
 * @param key the key that will be used to encrypt the data
 * @param db the database file to use
 *
 * @return The status code of the login attempt.
 */
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

char* getOnlineUsers(Sessions sessions) {
	// get string of inline clientIDs seperated by ;
	char* onlineUsers = malloc(1000 * sizeof(char));
	for (int i = 0; i < sessions.size; i++) {
		strcat(onlineUsers, sessions.sessions[i].clientID);
		strcat(onlineUsers, ";");
	}
	return onlineUsers;
}