#include "communication.h"

int sessionQueue;
char id[16];
int key;

void debug_message(Message* message) {
	printf("[msg]\n  status: %d\n  sender: %s\n  receiver: %s\n  body: %s\n[/msg]\n",
		message->mtext.header.statusCode, message->mtext.header.sender, 
		message->mtext.header.receiver, message->mtext.body
		);
}

void APIStart(void) {
	sessionQueue = -1;
	memset(id, 0, 16);
	key = -1;
	//APICreateConnection();
}

int APICreateConnection(void) {
	int sessionId = abs(((rand() & ((1 << 28) - 1)) ^ (rand() & ((1 << 15) - 1))) >> 7);

	char name[16] = {0};
	int tmp = sessionId;
	while (tmp > 9999999) tmp /= 2;
	sprintf(name, "u%1d-%03d-%03dr", rand() % 10, tmp / 10000, tmp % 10000);
	name[15] = 0;
	strcpy(id, name);

	int clientKey 	  = (key = hash(name));
	int clientQueueID = msgget(clientKey, 0666 | IPC_CREAT);

	Message msg;
	char data[128];
	sprintf(data, "%s;%d;", name, sessionId); // write queue name to body
	msgInit(&msg, 31, 0, id, "server", 200, data);

	key_t serverKey = hash("server");

	int serverSessionId = msgget(serverKey, 0666 | IPC_CREAT);
	msgsnd(serverSessionId, &msg, sizeof(msg), 0);

	Message response;
	msgrcv(clientQueueID, &response, sizeof(Message), 23, 0);

	if (response.mtext.header.type == 1) {
		printf("Establishing connection...\n");
	} else {
		printf("Connection failed\n");
		return -1;
	}
	msgctl(clientQueueID, IPC_RMID, NULL);

	strtok(response.mtext.body, ";"); // skip first arg
	int sessionSeed = atoi(strtok(NULL, ";"));
	int sessionKey  = createSessonKey(sessionId, sessionSeed);

	sessionQueue = msgget(sessionKey, 0666 | IPC_CREAT);
	printf("Connected to queue: %d, with key=%d successfully\n", sessionQueue, sessionKey);

	printf("q: %d\nk: %d\nid: %s\n", sessionQueue, key, id);

	return sessionQueue;
}

// login
Message APILogin(const char* username, const char* password) {
	// send username and password to server
	// receive response from server
	// if response is success, return username auth token
	// else return error code
	if (sessionQueue == -1) APICreateConnection();

	char data[128];
	memset(data, 0, sizeof(data));
	sprintf(data, "%s;%s;", username, password);
	Message message;
	msgInit(&message, 32, 10, id, "session", 200, data);

	msgsnd(sessionQueue, &message, sizeof(message), 0);

	Message response;
	for (int timer = 0; msgrcv(sessionQueue, &response, sizeof(Message), 23, 0) < 0; timer++) {
		usleep(1500);
		if (timer == 50) {
			response.mtext.header.statusCode = 500;
			break;
		}
	}

	// debug_message(&response);

	return response;
}
// register
Message APIRegister(const char* username, const char* password) {
	char user[32], pswd[32];
	strcpy(user, username);
	strcpy(pswd, password);
	if (sessionQueue == -1) APICreateConnection();
	printf("register queue: %d\n", sessionQueue);
	// send username and password to server
	// receive response from server
	// if response is success, return username auth token
	// else return error code

	char data[128];
	sprintf(data, "%s;%s;", user, pswd);

	Message message;
	msgInit(&message, 32, 11, id, "session", 200, data);
	msgsnd(sessionQueue, &message, sizeof(message), 0);

	Message response;
	for (int timer = 0; msgrcv(sessionQueue, &response, sizeof(Message), 23, 0) < 0; timer++) {
		usleep(1500);
		if (timer == 50) {
			response.mtext.header.statusCode = 500;
			break;
		}
	}

	// print message data
	// debug_message(&response);

	return response;
}
// logout
Message APILogout() {
	// send username to server
	// receive response from server
	// if response is success, show login interface
	// else, show error message
	char data[32];
	strcpy(data, id);

	Message message;
	msgInit(&message, 32, 12, id, "session", 200, data);
	msgsnd(sessionQueue, &message, sizeof(message), 0);

	Message response;
	for (int timer = 0; msgrcv(sessionQueue, &response, sizeof(Message), 23, 0) < 0; timer++) {
		usleep(1500);
		if (timer == 50) {
			response.mtext.header.statusCode = 500;
			break;
		}
	}

	// debug_message(&response);

	return response;
}

Message APIGetOnlineUsers(void)
{
    Message message;
	msgInit(&message, 32, 13, id, "session", 200, "");
	msgsnd(sessionQueue, &message, sizeof(message), 0);
	printf("getting online users...\n");
	Message response;
	for (int timer = 0; msgrcv(sessionQueue, &response, sizeof(Message), 23, 0) < 0; timer++) {
		usleep(1500);
		if (timer == 50) {
			response.mtext.header.statusCode = 500;
			break;
		}
	}
	// print message data
	debug_message(&response);

	return response;
}

Message APIBeginChat(const char *username)
{
    Message onlineUsers = APIGetOnlineUsers();
	char* body = onlineUsers.mtext.body;
	const char* next = strtok(body, ";");
	
	// check if username is correct
	bool found = false;
	while (next) {
		if (strcmp(next, body) == 0) {
			found = true;
			break;
		}
		next = strtok(NULL, ";");
	}

	if (!found) {
		Message errorMsg;
		errorMsg.mtext.header.statusCode = 500;
		return errorMsg;
	}

	Message message;
	message.mtype = 0;
	return message;
}
