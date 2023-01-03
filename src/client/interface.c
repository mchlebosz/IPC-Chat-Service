#include "interface.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chat.h"
#include "communication.h"

void clientLogin(void) {
	char username[30];
	char password[30];
	printf("Username: ");
	scanf("%s", username);
	printf("Password: ");
	scanf("%s", password);
	// send username and password to server
	// receive response from server
	// if response is success, show chat interface
	// else, show error message
	printf("%s", getStatusCode(200));

	Message response = APILogin(username, password);
	if (response.header.type == 0 && response.header.statusCode != 200) {
		printf("Login failed\n");
	} else {
		show_chat_interface(response.body);
	}
}

short MAX_CHARACTERS = 29;
short MIN_CHARACTERS = 8;

char* USERNAME_ERROR =
	"Username must contain at least 8 characters, maximum of 29 characters.\n";
char* PASSWORD_ERROR =
	"Password must contain at least 8 characters, maximum of 29 characters,\n ";

// check if username is valid
// if not, show error message
// return 0 if username is valid
// return 1 if username is invalid
short checkUsername(char* username) {
	// check if contains at least MIN characters, MAX characters
	if (username == NULL) {
		return 1;
	}
	int length = strlen(username);
	if (length < MIN_CHARACTERS || length > MAX_CHARACTERS) {
		return 2;
	}
	// check if contains only alphanumeric characters
	for (int i = 0; i < length; i++) {
		if (!isalnum(username[i])) {
			return 3;
		}
	}
	return 0;
}

// check if password is valid length
// check if password contains only alphanumeric characters or special
// characters check if password and confirm password are the same if not,
// show error message return 0 if password is valid
short checkPassword(char* password, char* confirmPassword) {
	// check if contains at least MIN characters, MAX characters
	if (password == NULL) {
		return 1;
	}
	int length = strlen(password);
	if (length < MIN_CHARACTERS || length > MAX_CHARACTERS) {
		return 2;
	}
	// check if contains only viable characters
	for (int i = 0; i < length; i++) {
		if (!isgraph(password[i])) {
			return 3;
		}
	}
	// check if password and confirm password are the same
	if (strcmp(password, confirmPassword) != 0) {
		return 4;
	}

	return 0;
}

void clientRegister(void) {
	char username[30];
	char password[30];
	char confirmPassword[30];
	// username
	while (1) {
		// reset username to empty string
		memset(username, 0, sizeof(username));

		printf("Username: ");
		scanf("%s", username);
		// check if username is valid
		// if not, show error message
		if (checkUsername(username) != 0) {
			printf("%s", USERNAME_ERROR);
		} else {
			break;
		}
		// TODO check if username is already taken
	}

	// password
	while (1) {
		// reset password and confirm password to empty string
		memset(password, 0, sizeof(password));
		memset(confirmPassword, 0, sizeof(confirmPassword));

		printf("%s", PASSWORD_ERROR);
		printf("Password: ");
		scanf("%s", password);
		printf("Confirm password: ");
		scanf("%s", confirmPassword);

		// check if password is valid
		// check if password and confirm password are the same
		// if not, show error message
		if (checkUsername(password) == 0) {
			break;
		} else if (checkUsername(password) == 4) {
			printf(
				"Password and confirm password are not the same. Please try again.\n");
		} else {
			printf("%s", PASSWORD_ERROR);
		}
	}

	// send username and password to server
	// receive response from server
	// if response is success, show chat interface
	// else, show error message
	Message response = APIRegister(username, password);
	if (response.header.statusCode != 200) {
		printf("Register failed\n");
	} else {
		response = APILogin(username, password);
		if (response.header.type == 0 && response.header.statusCode != 200) {
			printf("Login failed\n");
		} else {
			show_chat_interface(response.body);
		}
	}
}

void showInterface(void) {
	printf("Welcome\n");
	while (1) {
		printf("1. Login\n");
		printf("2. Register\n");
		printf("3. Exit\n");
		printf("Please choose one of the options: ");
		int option;
		scanf("%d", &option);
		switch (option) {
			case 1: clientLogin(); break;
			case 2: clientRegister(); break;
			case 3: exit(0); break;
			default: printf("Invalid option. Please try again.\n"); break;
		}
	}
}