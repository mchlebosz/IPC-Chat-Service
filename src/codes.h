#pragma once

typedef struct {
	int code;
	char* name;
} map_entry_t;

// struct for message that contains the following:
// header:
//  - type:
#define TYPE_CODES 13
extern const map_entry_t typeCodes[TYPE_CODES];
//  - sender
//  - receiver
//  - time
//  - error code:
#define STATUS_CODES 24
extern const map_entry_t statusCodes[STATUS_CODES];
// body

// vars

// char* getTypeCode(int code);
// char* getStatusCode(int code);