#pragma once

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#include "../message.h"

typedef struct {
	int code;
	char* name;
} map_entry_t;

// struct for message that contains the following:
// header:
//  - type:
#define TYPE_CODES 11
extern const map_entry_t typeCodes[TYPE_CODES];
//  - sender
//  - receiver
//  - time
//  - error code:
#define STATUS_CODES 24
extern const map_entry_t statusCodes[STATUS_CODES];
// body

void serve(int* keep_running, int* msgid);
