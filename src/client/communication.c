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
char* getTypeCode(int key) {
	for (int i = 0; i < TYPE_CODES; i++) {
		if (typeCodes[i].key == key) {
			return typeCodes[i].value;
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

char* getStatusCode(int key) {
	for (int i = 0; i < STATUS_CODES; i++) {
		if (statusCodes[i].key == key) {
			return statusCodes[i].value;
		}
	}
	return "NULL";
}

// login
Message APILogin(char* username, char* password) {
	// send username and password to server
	// receive response from server
	// if response is success, return username auth token
	// else return error code
	Message response;
	return response;
}
// register
Message APIRegister(char* username, char* password) {
	// send username and password to server
	// receive response from server
	// if response is success, return username auth token
	// else return error code
	Message response;
	return response;
}
// logout
Message APILogout(char* username) {
	// send username to server
	// receive response from server
	// if response is success, show login interface
	// else, show error message
	Message response;
	return response;
}
