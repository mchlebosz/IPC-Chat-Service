#include "serverQueue.h"

#include "dbHandler.h"

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
				Session session = { "",           sessionRunning, sessionKey,
									sessionQueue, sessionPID,     -1 };
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
				User* user  = NULL;
				int status1 = getUserByName(db, &user, username);
				// Asociate clientID with session and UserID
				int status2 = setSessionUserID(&sessions, receiver, user->id);

				if (status1 != 200 || status2 != 200) {
					// internal server error
					// send response message
					msgInit(&msg, 12, 1, "server", receiver, 500,
							"Internal Server Error (Register)");

					// send response message
					msgsnd(sessionQueue, &msg, sizeof(msg), 0);
					continue;
				}

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

			char* decryptKey = malloc(sizeof(char) * 100);

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
				User* user  = NULL;
				int status1 = getUserByName(db, &user, username);
				// Asociate clientID with session and UserID
				int status2 = setSessionUserID(&sessions, receiver, user->id);

				if (status1 != 200 || status2 != 200) {
					// internal server error
					// send response message
					msgInit(&msg, 12, 1, "server", receiver, 500,
							"Internal Server Error (Register)");

					// send response message
					msgsnd(sessionQueue, &msg, sizeof(msg), 0);

					continue;
				}

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
			// client id (to log out): sender
			char receiver[32];
			char sender[32];
			strcpy(receiver, msg.mtext.header.sender);
			strcpy(sender, msg.mtext.header.sender);
			char* clientID = sender;
			// connect to session queue
			int sessionQueue = getSessionQueue(&sessions, sender);

			int userID;
			int status = getSessionUserID(&sessions, clientID, &userID);

			// status check
			status = status == 200 && getUserById(db, NULL, userID) == 200 ?
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
			char messageBody[1000];
			char receiverUsername[32];
			char senderClientId[32];
			memcpy(messageBody, msg.mtext.body, 1000);
			strcpy(receiverUsername, msg.mtext.header.receiver);
			strcpy(senderClientId, msg.mtext.header.sender);

			int status = 200;

			// find sender username
			char* senderUsername = getUsernamebyClientID(sessions, senderClientId, db);

			if (senderUsername == NULL) status = 500;

			// get receiver session clientID
			char* receiverClientID =
				getClientIDbyUsername(sessions, receiverUsername, db);

			if (receiverClientID == NULL && status == 200) status = 404;
			// check if receiver is online
			if (isSessionRunning(&sessions, receiverClientID) != 200 &&
				status == 200)
				status = 404;
			// check if receiver exists
			int userID;
			if (getSessionUserID(&sessions, receiverClientID, &userID) != 200)
				status = 404;
			else if (getUserById(db, NULL, userID) != 200)
				status = 404;

			if (status == 200) {
				// send message to receiver
				// connect to session queue
				int receiverSessionQueue = getSessionQueue(&sessions, receiverClientID);
				// send response messa ge
				msgInit(&msg, 13, 34, senderUsername, receiverUsername, 200,
						messageBody);

				memcpy(msg.mtext.body, messageBody, 1000);

				msgsnd(receiverSessionQueue, &msg, sizeof(msg), 0);

				int sessionQueue = getSessionQueue(&sessions, senderClientId);

				msgInit(&msg, 12, 24, "server", senderClientId, 200, "OK");
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);

				puts("Message delivered!");
				continue;
			} else if (status == 404) {
				// user not found
				// send response message to sender
				msgInit(&msg, 12, 1, "server", senderClientId, 404,
						"Not Found (User does not exist)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, senderClientId);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
			} else {
				// internal server error - error opening database
				//  send response message to sender
				msgInit(&msg, 12, 1, "server", senderClientId, 500,
						"Internal Server Error (Send Message)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, senderClientId);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
			}
			// transfer message to session
		}
		// Get list of active users
		else if (msg.mtext.header.type == 13) {
			// message format:
			char sender[20];
			strcpy(sender, msg.mtext.header.sender);

			// check if client is logged in
			int status = isSessionRunning(&sessions, sender);

			// if client is logged in
			if (status == 200) {
				// get list of active users
				char* activeUsers = getOnlineUsersID(sessions, db);

				// if active users is empty
				if (activeUsers == NULL) {
					// send response message
					msgInit(&msg, 12, 13, "server", sender, 204,
							"No Active Users");

					// connect to session queue
					int sessionQueue = getSessionQueue(&sessions, sender);
					// send response message
					msgsnd(sessionQueue, &msg, sizeof(msg), 0);
					continue;
				}

				// send response message
				msgInit(&msg, 12, 13, "server", sender, 200, activeUsers);

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, sender);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else if (status == 404) {
				// send response message
				msgInit(&msg, 12, 13, "server", sender, 404,
						"Not Found (User does not exist)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, sender);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else {
				// internal server error
				// send response message
				msgInit(&msg, 12, 13, "server", sender, 500,
						"Internal Server Error (Get Active Users)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, sender);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
			}
		}
		// Add user to group
		else if (msg.mtext.header.type == 25) {
			// message format: groupname;username
			char messageBody[1000];
			char sender[20];
			strcpy(messageBody, msg.mtext.body);
			strcpy(sender, msg.mtext.header.sender);

			char* groupName = strtok(messageBody, ";");
			char* username  = strtok(NULL, ";");

			// check if user exists
			User* user = NULL;
			int status = getUserByName(db, &user, username);

			// check if group exists
			Group* group    = malloc(sizeof(Group));
			int isGroupCode = getGroupByName(db, group, groupName);

			// if client is logged in and group exists
			if (status == 200 && isGroupCode == 200) {
				// add user to group
				group->usersCount++;
				group->users[group->usersCount - 1] = user->id;

				// update group in database
				setGroup(db, group);

				char* usersString = malloc(10000);
				for (int i = 0; i < group->usersCount; i++) {
					// group->users[i] is userID from int to string
					char userID[10] = { 0 };
					sprintf(userID, "%d", group->users[i]);
					strcat(usersString, userID);
					strcat(usersString, ";");
				}

				// send response message
				msgInit(&msg, 12, 25, "server", sender, 200, usersString);

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, sender);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else if (status == 404) {
				// send response message
				msgInit(&msg, 12, 25, "server", sender, 404,
						"Not Found (User does not exist)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, sender);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else if (isGroupCode == 404) {
				// send response message
				msgInit(&msg, 12, 25, "server", sender, 404,
						"Not Found (Group does not exist)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, sender);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else {
				// internal server error
				// send response message
				msgInit(&msg, 12, 25, "server", sender, 500,
						"Internal Server Error (Add to Group)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, sender);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
			}
		}
		// Show available groups 15
		else if (msg.mtext.header.type == 15) {
			// message format:
			char* sender = msg.mtext.header.sender;

			// get list of all groups
			Group* groups    = malloc(10000 * sizeof(Group));
			int* groupsCount = malloc(sizeof(int));
			int status       = getAllGroups(db, &groups, groupsCount);

			// if group exists
			if (status == 200) {
				char* groupsString = malloc(*groupsCount * 100);
				for (int i = 0; i < *groupsCount; i++) {
					strcat(groupsString, groups[i].name);
					strcat(groupsString, ";");
				}

				// send response message
				msgInit(&msg, 12, 15, "server", sender, 200, groupsString);

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, sender);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else if (status == 404) {
				// send response message
				msgInit(&msg, 12, 15, "server", sender, 404,
						"Not Found (Groups does not exist)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, sender);
				// send response message
				msgsnd(sessionQueue, &msg, sizeof(msg), 0);
				continue;
			} else {
				// internal server error
				// send response message
				msgInit(&msg, 12, 15, "server", sender, 500,
						"Internal Server Error (Show Groups)");

				// connect to session queue
				int sessionQueue = getSessionQueue(&sessions, sender);
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
	int status = getUserByName(db, NULL, username);
	if (status == 200) {
		return 409;
	} else if (status == 404) {
		// generate decryption key
		*key = generateKey();
		// encrypt password
		// char* encryptedPassword = encrypt(password, key);
		// add user to database
		// format: username;password;key;

		// generate id randomly with seed based on username
		int userID = 0;
		for (int i = 0; i < strlen(username); i++) {
			userID += username[i];
		}
		srand(userID);
		userID = rand() % 1000000;

		User* newUser = malloc(sizeof(User));
		newUser->id   = userID;
		strcpy(newUser->name, username);
		strcpy(newUser->password, password);
		strcpy(newUser->publicKey, *key);
		newUser->friendsCount = 0;
		newUser->groupsCount  = 0;

		if (addUser(db, newUser) == 200) {
			return 200;
		} else {
			return 500;
		}
	}
	return 500;
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
	// get user data
	User* loggedUser = NULL;
	int status       = getUserByName(db, &loggedUser, username);
	printf("status:%s %d\n", username, status);

	if (status != 200) {
		return status;
	}

	// get user key
	strcpy(*key, loggedUser->publicKey);
	// compare passwords
	status = strcmp(password, loggedUser->password);
	if (status == 0) {
		return 200;
	} else {
		return 401;
	}
}

/**
 * It takes a Sessions struct and a database name, and returns a string of all
 * the online users' names, seperated by semicolons
 *
 * @param sessions The sessions array
 * @param db the database file
 *
 * @return A string of online users seperated by ;
 */
char* getOnlineUsersID(Sessions sessions, const char* db) {
	// get string of inline clientIDs seperated by ;
	char* onlineUsers = malloc(1000 * sizeof(char));
	memset(onlineUsers, 0, 1000);
	for (int i = 0; i < sessions.size; i++) {

		char username[34];
		memset(username, 0, 34);

		User* user = NULL;
		getUserById(db, &user, sessions.sessions[i].userLoggedInID);
		sprintf(username, "%s;", user->name);

		strcat(onlineUsers, username);

		free(user);
	}
	return onlineUsers;
}

/**
 * It takes a Sessions struct and returns a string of all the online clientIDs
 * seperated by a semicolon
 *
 * @param sessions The sessions struct that contains all the sessions.
 *
 * @return A string of all the online clientIDs seperated by ;
 */
char* getOnlineClientID(Sessions sessions) {
	// get string of inline clientIDs seperated by ;
	char* onlineUsers = malloc(1000 * sizeof(char));
	for (int i = 0; i < sessions.size; i++) {
		char clientID[33];
		memset(clientID, 0, 33);
		sprintf(clientID, "%s;", sessions.sessions[i].clientID);
		strcat(onlineUsers, clientID);
	}
	return onlineUsers;
}

/**
 * It gets the username of the user logged in by the clientID
 *
 * @param sessions the sessions array
 * @param clientID the client ID of the client that sent the message
 * @param db the database file name
 *
 * @return A pointer to a char
 * Question: What is the return type?
 * Answer: char*
 * Question: What is the name of the function?
 * Answer: getUsernamebyClientID
 * Question: What are the parameters?
 * Answer: Sessions sessions, char* clientID, const char* db
 * Question: What is the parameter type?
 * Answer: Sessions, char*, const char
 */
char* getUsernamebyClientID(Sessions sessions, char* clientID, const char* db) {
	for (int i = 0; i < sessions.size; i++) {
		if (strcmp(sessions.sessions[i].clientID, clientID) == 0) {
			// find username in database
			User* user = NULL;
			if (getUserById(db, &user, sessions.sessions[i].userLoggedInID) != 200) 
				return NULL;
			char* username = (char*)malloc(32);
			strcpy(username, user->name);
			free(user);
			return username;
		}
	}
	return NULL;
}

/**
 * It gets the client ID of a user by their username
 *
 * @param sessions the sessions array
 * @param username the username of the user you want to get the client id of
 * @param db the database file
 *
 * @return A pointer to a char.
 */
char* getClientIDbyUsername(Sessions sessions, char* username, const char* db) {
	// get user id
	User* user;
	getUserByName(db, &user, username);
	// get client id
	for (int i = 0; i < sessions.size; i++) {
		if (sessions.sessions[i].userLoggedInID == user->id) {
			free(user);
			return sessions.sessions[i].clientID;
		}
	}
	return NULL;
}