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
}

void createId(int sessionId) {
	char name[20];
	while (sessionId > 9999999) sessionId /= 2;
	sprintf(name, "u%1u-%03u-%03ur", rand() % 10, sessionId / 10000, sessionId % 10000);
	name[15] = 0;
	memcpy(id, name, 16);
}

int APICreateConnection(void) {
	if (sessionQueue != -1) APILogout();
	int sessionId = abs(((rand() & ((1 << 28) - 1)) ^ (rand() & ((1 << 15) - 1))) >> 7);

	createId(sessionId);

	int clientKey 	  = (key = hash(id));
	int clientQueueID = msgget(clientKey, 0666 | IPC_CREAT);

	Message message;
	char data[256];
	sprintf(data, "%s;%d;", id, sessionId); // write queue name to body
	msgInit(&message, 31, 0, id, "server", 200, data);

	key_t serverKey = hash("server");

	int serverSessionId = msgget(serverKey, 0666 | IPC_CREAT);
	msgsnd(serverSessionId, &message, sizeof(message), 0);

	Message response;
	msgrcv(clientQueueID, &response, sizeof(Message), 23, 0);

	msgctl(clientQueueID, IPC_RMID, NULL);

	if (response.mtext.header.type != 1) {
		printf("Connection failed\n");
		return -1;
	}

	strtok(response.mtext.body, ";"); // skip first arg
	int sessionSeed = atoi(strtok(NULL, ";"));
	int sessionKey  = createSessonKey(sessionId, sessionSeed);

	sessionQueue = msgget(sessionKey, 0666 | IPC_CREAT);
	//printf("Connected to queue: %d, with key=%d successfully\n", sessionQueue, sessionKey);
	//printf("q: %d\nk: %d\nid: %s\n", sessionQueue, key, id);
	return sessionQueue;
}

// login
Message APILogin(const char* username, const char* password) {
	// send username and password to server
	// receive response from server
	// if response is success, return username auth token
	// else return error code
	char user[32], pswd[32];
	strcpy(user, username);
	strcpy(pswd, password);
	if (sessionQueue == -1) APICreateConnection();

	char data[128];
	memset(data, 0, sizeof(data));
	sprintf(data, "%s;%s;", user, pswd);
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
	char data[32];
	strcpy(data, id);

	Message message, response;
	msgInit(&message, 32, 12, id, "session", 200, data);
	msgsnd(sessionQueue, &message, sizeof(message), 0);

	if (msgrcv(sessionQueue, &response, sizeof(Message), 23, 0) < 0)
		response.mtext.header.statusCode = 500;

	return response;
}

Message APIGetOnlineUsers(void) {
    Message message, response;
	msgInit(&message, 32, 13, id, "session", 200, "");
	msgsnd(sessionQueue, &message, sizeof(message), 0);

	if (msgrcv(sessionQueue, &response, sizeof(Message), 23, 0) < 0)
		response.mtext.header.statusCode = 500;

	return response;
}

Message APIBeginChat(const char* username) {
	Message message, response;
	msgInit(&message, 32, 24, id, username, 200, "");
	const char data[] = notifyChatBeginMsg;
	memcpy(message.mtext.body, data, sizeof(data));
	msgsnd(sessionQueue, &message, sizeof(message), 0);

	if (msgrcv(sessionQueue, &response, sizeof(Message), 23, 0) < 0)
		response.mtext.header.statusCode = 500;

	msgInit(&message, 13, 100, id, id, 200, username); // inform receive loop about current chatter
	msgsnd(sessionQueue, &message, sizeof(message), 0);

	return response;
}

Message APIEndChat(const char* username) {
	Message message, response;
	msgInit(&message, 32, 24, id, username, 200, "");
	const char data[] = notifyChatEndMsg;
	memcpy(message.mtext.body, data, sizeof(data));
	msgsnd(sessionQueue, &message, sizeof(message), 0);

	if (msgrcv(sessionQueue, &response, sizeof(Message), 23, 0) < 0)
		response.mtext.header.statusCode = 500;

	msgInit(&message, 13, 101, id, id, 200, username); // inform receive loop about current chatter
	msgsnd(sessionQueue, &message, sizeof(message), 0);

	return response;
}

Message APIChatSendMessage(char *data, const char* receiverName) {
    Message message, response;
	msgInit(&message, 32, 24, id, receiverName, 200, trim(data));
	msgsnd(sessionQueue, &message, sizeof(Message), 0);
	if (msgrcv(sessionQueue, &response, sizeof(Message), 23, 0) < 0)
		response.mtext.header.statusCode = 500;
	return response;
}

Message APIChatReceiveMessage(void) {
    Message response;
	if (msgrcv(sessionQueue, &response, sizeof(Message), 13, 0) < 0)
		response.mtext.header.statusCode = 500;
	return response;
}

int APIGetSessionQueueId() {
    return sessionQueue;
}
