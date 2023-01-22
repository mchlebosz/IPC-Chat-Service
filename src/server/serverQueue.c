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
				Session session = { clientID, sessionRunning, sessionKey,
									sessionQueue };
				addSession(&sessions, session);
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

int getSessionQueue(Sessions* sessions, char* sessionID) {
	// find session in sessions list
	for (int i = 0; i < sessions->size; i++) {
		if (sessions->sessions[i].sessionID == sessionID) {
			return sessions->sessions[i].sessionQueue;
		}
	}
	return 404;
}

int receiveMessage(int* msgid, Message* msg, long permittedType) {
	ssize_t msg_size = msgrcv(*msgid, msg, sizeof(msg), permittedType, 0);
	if (msg_size < 0) {
		perror("msgrcv");
		return 500;
	}
	if (msg_size > sizeof(Message)) {
		printf("Error: message size exceeds buffer size, discarding message\n");
		return 413;
	}
	// handle the received message
	for (int i = 0; i < TYPE_CODES; i++) {
		if (msg->mtext.header.type == typeCodes[i].code) {
			printf("Received %s request from %s at %s: %s\n", typeCodes[i].name,
				   msg->mtext.header.sender, msg->mtext.header.time,
				   msg->mtext.body);
			break;
		}
	}
	return 200;
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