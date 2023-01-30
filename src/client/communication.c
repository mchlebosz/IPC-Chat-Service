#include "communication.h"

int sessionQueue;
char id[16];
int key;

void APIStart(void) {
	sessionQueue = -1;
	memset(id, -1, 16);
	key = -1;
}

int APICreateConnection(void) {
	int sessionId = abs(((rand() & ((1 << 28) - 1)) ^ (rand() & ((1 << 15) - 1))) >> 7);

	char name[16] = {0};
	int tmp = sessionId;
	while (tmp > 9999999) tmp /= 2;
	sprintf(name, "u%1d-%03d-%04dr", rand() % 10, tmp / 10000, tmp % 10000);
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


	msgctl(clientQueueID, IPC_RMID, NULL);
	if (response.mtext.header.type == 1) {
		printf("Establishing connection...\n");
	} else {
		printf("Connection failed\n");
		return -1;
	}

	strtok(response.mtext.body, ";"); // skip first arg
	int sessionSeed = atoi(strtok(NULL, ";"));
	int sessionKey  = createSessonKey(sessionId, sessionSeed);

	sessionQueue = msgget(sessionKey, 0666 | IPC_CREAT);
	printf("Connected to queue: %d, with key=%d successfully\n", sessionQueue, sessionKey);

	return sessionQueue;
}

// login
Message APILogin(const char* username, const char* password) {
	// send username and password to server
	// receive response from server
	// if response is success, return username auth token
	// else return error code
	if (sessionQueue == -1) APICreateConnection();
	printf("trying to log in...\n");
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

	printf("[msg]\n  status: %d\n  sender: %s\n  receiver: %s\n  body: %s\n[/msg]\n",
		response.mtext.header.statusCode, response.mtext.header.sender, 
		response.mtext.header.receiver, response.mtext.body
		);

	return response;
}
// register
Message APIRegister(const char* username, const char* password) {
	if (sessionQueue == -1) APICreateConnection();
	printf("register queue: %d\n", sessionQueue);
	// send username and password to server
	// receive response from server
	// if response is success, return username auth token
	// else return error code

	char data[128];
	sprintf(data, "%s;%s;", username, password);

	Message message;
	msgInit(&message, 32, 11, id, "session", 200, data);
	msgsnd(sessionQueue, &message, sizeof(message), 0);
	printf("trying to register...\n");
	Message response;
	for (int timer = 0; msgrcv(sessionQueue, &response, sizeof(Message), 23, 0) < 0; timer++) {
		usleep(1500);
		if (timer == 50) {
			response.mtext.header.statusCode = 500;
			break;
		}
	}

	// print message data
	printf("[msg]\n  status: %d\n  sender: %s\n  receiver: %s\n  body: %s\n[/msg]\n",
		response.mtext.header.statusCode, response.mtext.header.sender, 
		response.mtext.header.receiver, response.mtext.body
		);


	return response;
}
// logout
Message APILogout(char* username) {
	// send username to server
	// receive response from server
	// if response is success, show login interface
	// else, show error message
	Message response;
	response.mtext.header.type = 12;

	return response;
}
