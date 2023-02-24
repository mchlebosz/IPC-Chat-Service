#define _GNU_SOURCE
#define __USE_GNU
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "communication.h"
#include "interface.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#ifdef _WIN32
	#include <winsock2.h>
	#include <windows.h>
#else
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <unistd.h>
#endif    //

/**
 * It sets the global variable `keep_running` to 0
 *
 * @param signum The signal number.
 */
void sigint_handler(int signum) {
	APILogout();
	pid_t cp = getChildPID();
	kill(cp, SIGTERM);
}

int main(int argc, char *argv[]) {
	srand(time(NULL));
	signal(SIGINT, sigint_handler);
	APIStart();
	showInterface();

	return 0;
}
