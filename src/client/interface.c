#include "interface.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32

#else
	#include <termios.h>
#endif

#include "chat.h"
#include "communication.h"

#define MAXPW 32
/* read a string from fp into pw masking keypress with mask char.
getpasswd will read upto sz - 1 chars into pw, null-terminating
the resulting string. On success, the number of characters in
pw are returned, -1 otherwise.
*/
ssize_t getpasswd(char** pw, size_t sz, int mask, FILE* fp) {
	if (!pw || !sz || !fp) return -1; /* validate input   */
#ifdef MAXPW
	if (sz > MAXPW) sz = MAXPW;
#endif

	if (*pw == NULL) { /* reallocate if no address */
		void* tmp = realloc(*pw, sz * sizeof **pw);
		if (!tmp) return -1;
		memset(tmp, 0, sz); /* initialize memory to 0   */
		*pw = (char*)tmp;
	}

	size_t idx = 0; /* index, number of chars in read   */
	int c      = 0;

	struct termios old_kbd_mode; /* orig keyboard settings   */
	struct termios new_kbd_mode;

	if (tcgetattr(0, &old_kbd_mode)) { /* save orig settings   */
		fprintf(stderr, "%s() error: tcgetattr failed.\n", __func__);
		return -1;
	} /* copy old to new */
	memcpy(&new_kbd_mode, &old_kbd_mode, sizeof(struct termios));

	new_kbd_mode.c_lflag &= ~(ICANON | ECHO); /* new kbd flags */
	new_kbd_mode.c_cc[VTIME] = 0;
	new_kbd_mode.c_cc[VMIN]  = 1;
	if (tcsetattr(0, TCSANOW, &new_kbd_mode)) {
		fprintf(stderr, "%s() error: tcsetattr failed.\n", __func__);
		return -1;
	}

	/* read chars from fp, mask if valid char specified */
	while (((c = fgetc(fp)) != '\n' && c != EOF && idx < sz - 1) ||
		   (idx == sz - 1 && c == 127)) {
		if (c != 127) {
			if (31 < mask && mask < 127) /* valid ascii char */
				fputc(mask, stdout);
			(*pw)[idx++] = c;
		} else if (idx > 0) { /* handle backspace (del)   */
			if (31 < mask && mask < 127) {
				fputc(0x8, stdout);
				fputc(' ', stdout);
				fputc(0x8, stdout);
			}
			(*pw)[--idx] = 0;
		}
	}
	(*pw)[idx] = 0; /* null-terminate   */

	/* reset original keyboard  */
	if (tcsetattr(0, TCSANOW, &old_kbd_mode)) {
		fprintf(stderr, "%s() error: tcsetattr failed.\n", __func__);
		return -1;
	}

	if (idx == sz - 1 && c != '\n') /* warn if pw truncated */
		fprintf(stderr, " (%s() warning: truncated at %zu chars.)\n", __func__,
				sz - 1);

	return idx; /* number of chars in passwd    */
}

void clientLogin(void) {
	char username[30];
	char password[30];
	printf("Username: ");
	if (scanf("%s", username) != 1) {
		printf("Invalid username\n");
		return;
	}
	printf("Password: ");
	if (scanf("%s", password) != 1) {
		printf("Invalid password\n");
		return;
	}
	// send username and password to server
	// receive response from server
	// if response is success, show chat interface
	// else, show error message
	// printf("%s", getStatusCode(200));

	Message response = APILogin(username, password);
	if (response.mtext.header.type == 0 && response.mtext.header.statusCode != 200) {
		printf("Login failed\n");
	} else {
		show_chat_interface(response.mtext.body);
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
	APICreateConnection();
	// username
	while (1) {
		// reset username to empty string
		memset(username, 0, sizeof(username));

		printf("Username: ");
		if (scanf("%s", username) != 1) {
			printf("%s", USERNAME_ERROR);
			continue;
		}
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
		if (scanf("%s", password) != 1) {
			printf("%s", PASSWORD_ERROR);
			continue;
		}
		printf("Confirm password: ");
		if (scanf("%s", confirmPassword) != 1) {
			printf("%s", PASSWORD_ERROR);
			continue;
		}

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
	if (response.mtext.header.statusCode != 200) {
		printf("Register failed\n");
	} else {
		response = APILogin(username, password);
		if (response.mtext.header.type == 0 && response.mtext.header.statusCode != 200) {
			printf("Login failed\n");
		} else {
			show_chat_interface(response.mtext.body);
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
		if (scanf("%d", &option) != 1) {
			printf("Invalid option. Please try again.\n");
			continue;
		}
		switch (option) {
			case 1: clientLogin(); break;
			case 2: clientRegister(); break;
			case 3: exit(0); break;
			default: printf("Invalid option. Please try again.\n"); break;
		}
	}
}