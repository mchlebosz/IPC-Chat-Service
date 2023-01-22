#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct Message {
	struct header {
		short type;
		char sender[20];
		char receiver[20];
		char time[20];
		short statusCode;
	} header;
	char body[1000];
} Message;
