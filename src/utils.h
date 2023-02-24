#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>

#define notifyChatBeginMsg "<\0\0\n#\0>"

#define notifyChatEndMsg "<\0$\0\n\0>"

unsigned long hash(const char *str);

int createSessonKey(int clientId, int sessionSeed);

int scanfInt(void);

void setChildPID(int pid);

int getChildPID(void);

bool startsWith(const char* s, const char* flag);

bool isCommand(const char* s);

bool isChatBeginMsg(const char* msg);

bool isChatEndMsg(const char* msg);

char* trim(char* s);