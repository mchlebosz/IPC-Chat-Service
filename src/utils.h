#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

unsigned long hash(const char *str);

int createSessonKey(int clientId, int sessionSeed);

int scanfInt(void);