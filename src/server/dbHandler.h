// Handle file with database

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// file handler
FILE* openFile(const char* fileName, const char* mode);

void addData(const char* fileName, const char* data);

void removeData(const char* fileName, const char* data);

void searchData(const char* fileName, const char* data);
