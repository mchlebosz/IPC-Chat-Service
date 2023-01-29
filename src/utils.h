#pragma once

#include <string.h>
#include <stdbool.h>

unsigned long hash(const char *str);

int createSessonKey(int clientId, int sessionSeed);