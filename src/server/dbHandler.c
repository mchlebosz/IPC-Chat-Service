#include "dbHandler.h"

#include <stdio.h>
#include <string.h>

/**
 * If the file doesn't exist, create it, then open it in the specified mode.
 *
 * @param filename the name of the file to open
 * @param mode r - read, w - write, a - append, r+ - read/write, w+ -
 * read/write, a+ - read/append
 *
 * @return A pointer to a file.
 */
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
/**
 * It opens a file in append mode, writes data to it, and closes the file
 *
 * @param file_name The name of the file to open.
 * @param data The data to be added to the file.
 *
 * @return the number of characters in the string.
 */
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
/**
 * It opens the file in read mode, creates a temporary file, reads the contents
 * of the original file into a buffer, checks if the data to be removed is in
 * the buffer, and if it is, it writes the buffer to the temporary file. If the
 * data is not in the buffer, it does not write the buffer to the temporary
 * file. After the file has been read, the original file is closed, the
 * temporary file is closed, the original file is removed, and the temporary
 * file is renamed to the original file
 *
 * @param file_name The name of the file to remove data from.
 * @param data The data to be removed from the file.
 *
 * @return the number of times the data was found in the file.
 */
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
/**
 * It opens a file, reads it line by line, and checks if the data is in the
 * line. If it is, it prints the line number and the line itself
 *
 * @param file_name The name of the file to search in.
 * @param data The data to search for in the file.
 *
 * @return the status code of the search.
 */
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
/**
 * It opens a file, reads it line by line, and checks if the line contains the
 * data we're looking for. If it does, it returns 200, otherwise it returns 204
 *
 * @param file_name the name of the file to be opened
 * @param data the data to be searched for
 * @param buffer a pointer to a pointer to a char. This is because we want to be
 * able to change the value of the pointer, not just what it points to.
 *
 * @return The status code of the request.
 */
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