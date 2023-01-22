#include "communication.h"

// login
Message APILogin(char* username, char* password) {
	// send username and password to server
	// receive response from server
	// if response is success, return username auth token
	// else return error code
	Message response;
	response.mtext.header.type = 10;
	return response;
}
// register
Message APIRegister(char* username, char* password) {
	// send username and password to server
	// receive response from server
	// if response is success, return username auth token
	// else return error code

	Message response;
	response.mtext.header.type = 11;

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
