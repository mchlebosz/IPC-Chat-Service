#include "dbHandler.h"

#include <stdio.h>
#include <string.h>

FILE* openFile(const char* filename, const char* mode) {
	FILE* file = fopen(filename, mode);
	if (file == NULL) {
		// create file if it doesn't exist
		file = fopen(filename, "w");
		fclose(file);
		file = fopen(filename, mode);
	}
	return file;
}

// Function to add data to a file
void addData(const char* file_name, const char* data) {
	FILE* file = fopen(file_name, "a");    // open file in append mode
	if (file == NULL) {
		printf("Error opening file: %s\n", file_name);
		return;
	}

	fprintf(file, "%s\n", data);    // write data to file
	fclose(file);                   // close file
	printf("Data added to file: %s\n", data);
}

// Function to remove data from a file
void removeData(const char* file_name, const char* data) {
	char buffer[1024];                     // buffer to store contents of file
	FILE* file = fopen(file_name, "r");    // open file in read mode
	if (file == NULL) {
		printf("Error opening file: %s\n", file_name);
		return;
	}

	FILE* temp = fopen("temp.txt", "w");    // create temporary file
	if (temp == NULL) {
		printf("Error creating temporary file\n");
		return;
	}

	while (fgets(buffer, 1024, file) != NULL) {
		if (strstr(buffer, data) == NULL) {    // check if data is in buffer
			fputs(buffer, temp);               // write buffer to temporary file
		}
	}

	fclose(file);                     // close original file
	fclose(temp);                     // close temporary file
	remove(file_name);                // remove original file
	rename("temp.txt", file_name);    // rename temporary file to original file
	printf("Data removed from file: %s\n", data);
}

// Function to search for data in a file
int searchData(const char* file_name, const char* data) {
	char buffer[1024];
	FILE* file = fopen(file_name, "r");
	if (file == NULL) {
		printf("Error opening file: %s\n", file_name);
		return 404;
	}

	int line_num = 1;
	while (fgets(buffer, 1024, file) != NULL) {
		if (strstr(buffer, data) != NULL) {    // check if data is in buffer
			printf("Data found in line %d: %s\n", line_num, buffer);
			return 200;
		}
		line_num++;
	}
	fclose(file);
	return 204;
}

// Function to get data from a file
int getData(const char* file_name, const char* data, char** buffer) {
	FILE* file = fopen(file_name, "r");
	if (file == NULL) {
		printf("Error opening file: %s\n", file_name);
		return 404;
	}
	while (fgets(*buffer, 1024, file) != NULL) {
		if (strstr(*buffer, data) != NULL) {    // check if data is in buffer
			fclose(file);
			return 200;
		}
	}
	fclose(file);
	return 204;
}