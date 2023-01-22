#include "communication.h"

// vars
const map_entry_t typeCodes[TYPE_CODES] = {
	{ 10, "Login" },
	{ 11, "Register" },
	{ 12, "Logout" },
	{ 13, "Get list of users" },
	{ 14, "Get list of messages" },
	{ 15, "Get list of groups" },
	{ 24, "Send a message" },
	{ 23, "Send a request to add a user to the contact list" },
	{ 25, "Send a request to add a user to a group" },
	{ 26, "Send a request to create a group" },
	{ 34, "Receive a message" }
};
char* getTypeCode(int code) {
	for (int i = 0; i < TYPE_CODES; i++) {
		if (typeCodes[i].code == code) {
			return typeCodes[i].name;
		}
	}
	return "NULL";
}
const map_entry_t statusCodes[STATUS_CODES] = {
	{ 100, "Continue" },
	{ 101, "Switching Protocols" },
	{ 200, "OK" },
	{ 201, "Created" },
	{ 202, "Accepted" },
	{ 203, "Non-Authoritative Information" },
	{ 204, "No Content" },
	{ 205, "Reset Content" },
	{ 206, "Partial Content" },
	{ 400, "Bad Request" },
	{ 401, "Unauthorized" },
	{ 403, "Forbidden" },
	{ 404, "Not Found" },
	{ 408, "Request Timeout" },
	{ 409, "Conflict" },
	{ 410, "Gone" },
	{ 411, "Length Required" },
	{ 413, "Request Entity Too Large" },
	{ 418, "I'm a teapot" },
	{ 429, "Too Many Requests" },
	{ 440, "Login timeout" },
	{ 500, "Internal Server Error" },
	{ 501, "Not Implemented" },
	{ 503, "Service Unavailable" }
};

char* getStatusCode(int code) {
	for (int i = 0; i < STATUS_CODES; i++) {
		if (statusCodes[i].code == code) {
			return statusCodes[i].name;
		}
	}
	return "NULL";
}

void serve(int* keep_running, int* msgid) {
	if (msgid < 0) {
		perror("msgget");
		exit(1);
	}

	while (keep_running) {
		Message msg;

		ssize_t msg_size = msgrcv(msgid, &msg, sizeof(msg), 0, 0);
		if (msg_size < 0) {
			perror("msgrcv");
			continue;
		}
		if (msg_size > sizeof(Message)) {
			printf(
				"Error: message size exceeds buffer size, discarding message\n");
			continue;
		}
		// handle the received message
		for (int i = 0; i < TYPE_CODES; i++) {
			if (msg.header.type == typeCodes[i].code) {
				printf("Received %s request from %s at %s: %s\n",
					   typeCodes[i].name, msg.header.sender, msg.header.time,
					   msg.body);
				break;
			}
		}

		// send response message
		msg.header.type       = 34;
		msg.header.statusCode = 200;
		msgsnd(msgid, &msg, sizeof(msg), 0);
	}
}