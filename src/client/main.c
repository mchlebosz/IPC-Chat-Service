#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "communication.h"
#include "interface.h"

#ifdef _WIN32
	#include <winsock2.h>
	#include <windows.h>
#else

	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <unistd.h>
#endif    //

int main(int argc, char *argv[]) {
	srand(time(NULL));
	APIStart();
	showInterface();

	return 0;
}
