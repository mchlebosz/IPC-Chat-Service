#include "dbHandler.h"

// include FILE
#include <stdio.h>

/**
 * If the file doesn't exist, create it, then open it in the specified mode.
 *
 * @param filename the name of the file to open
 * @param mode r - read, w - write, a - append, r+ - read/write, w+ -
 * read/write, a+ - read/append
 *
 * @return A pointer to a file.
 */
FILE* openDB(const char* filename, const char* mode) {
	FILE* file = fopen(filename, mode);
	if (file == NULL) {
		// create file if it doesn't exist
		file          = fopen(filename, "w");
		cJSON* root   = cJSON_CreateObject();
		cJSON* users  = cJSON_CreateArray();
		cJSON* groups = cJSON_CreateArray();
		cJSON_AddItemToObject(root, "Users", users);
		cJSON_AddItemToObject(root, "Groups", groups);
		char* json = cJSON_Print(root);
		fprintf(file, "%s", json);
		free(json);
		cJSON_Delete(root);

		fclose(file);
		file = fopen(filename, mode);
	}
	return file;
}

// Function to read data from a file

/**
 * It reads the contents of a file into a string
 *
 * @param filename The name of the file to read.
 *
 * @return A pointer to the first element of the array.
 */
char* readFile(const char* filename) {
	FILE* fp = fopen(filename, "r");
	if (!fp) {
		printf("Error opening file\n");
		return NULL;
	}

	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	rewind(fp);

	char* buffer = (char*)malloc((size + 1) * sizeof(char));
	if (!buffer) {
		printf("Error allocating memory\n");
		fclose(fp);
		return NULL;
	}

	size_t result = fread(buffer, 1, size, fp);
	if (result != size) {
		printf("Error reading file\n");
		free(buffer);
		fclose(fp);
		return NULL;
	}

	buffer[size] = '\0';
	fclose(fp);
	return buffer;
}

/**
 * It opens a file, writes data to it, and closes it
 *
 * @param filename the name of the file to write to
 * @param data the data to write to the file
 *
 * @return The status code of the file write operation.
 */
int writeFile(const char* filename, const char* data) {
	// overwrite file
	FILE* fp = fopen(filename, "w");
	if (!fp) {
		printf("Error opening file\n");
		return 404;
	}

	fprintf(fp, "%s", data);
	fclose(fp);
	return 200;
}

/**
 * It reads the file, parses the JSON, and then copies the data from the JSON
 * into a User struct
 *
 * @param file_name the name of the file to read from
 * @param user a pointer to a User struct
 * @param name The name of the user to get
 *
 * @return the status code of the operation.
 */
int getUserByName(const char* file_name, User* user, char* name) {
	const char* json_str = readFile(file_name);
	cJSON *json, *item, *new_item;

	json = cJSON_Parse(json_str);
	if (!json) {
		printf("Error parsing json\n");
		return 500;
	}
	// go to users array
	item = cJSON_GetObjectItemCaseSensitive(json, "Users");
	if (!cJSON_IsArray(item)) {
		printf("Error getting users array\n");
		return 500;
	}
	// find user
	for (int i = 0; i < cJSON_GetArraySize(item); i++) {
		cJSON* user_json = cJSON_GetArrayItem(item, i);
		if (cJSON_IsObject(user_json)) {
			new_item = cJSON_GetObjectItemCaseSensitive(user_json, "name");
			if (cJSON_IsString(new_item) && (new_item->valuestring != NULL)) {
				if (strcmp(new_item->valuestring, name) == 0) {
					// parse all data from user to struct
					// get id
					new_item =
						cJSON_GetObjectItemCaseSensitive(user_json, "id");
					if (cJSON_IsNumber(new_item)) {
						user->id = new_item->valueint;
					}

					// get name
					new_item =
						cJSON_GetObjectItemCaseSensitive(user_json, "name");
					if (cJSON_IsString(new_item) &&
						(new_item->valuestring != NULL)) {
						strcpy(user->name, new_item->valuestring);
					}

					// get password
					new_item =
						cJSON_GetObjectItemCaseSensitive(user_json, "password");
					if (cJSON_IsString(new_item) &&
						(new_item->valuestring != NULL)) {
						strcpy(user->password, new_item->valuestring);
					}

					// get groups
					new_item =
						cJSON_GetObjectItemCaseSensitive(user_json, "groups");
					if (cJSON_IsArray(new_item)) {
						for (int j = 0; j < cJSON_GetArraySize(new_item); j++) {
							cJSON* group_json = cJSON_GetArrayItem(new_item, j);
							if (cJSON_IsObject(group_json)) {
								new_item = cJSON_GetObjectItemCaseSensitive(
									group_json, "id");
								if (cJSON_IsNumber(new_item)) {
									user->groups[j] = new_item->valueint;
								}
							}
						}
						// group count
						user->groupsCount = cJSON_GetArraySize(new_item);
					}

					// friends
					new_item =
						cJSON_GetObjectItemCaseSensitive(user_json, "friends");
					if (cJSON_IsArray(new_item)) {
						for (int j = 0; j < cJSON_GetArraySize(new_item); j++) {
							cJSON* friend_json =
								cJSON_GetArrayItem(new_item, j);
							if (cJSON_IsObject(friend_json)) {
								new_item = cJSON_GetObjectItemCaseSensitive(
									friend_json, "id");
								if (cJSON_IsNumber(new_item)) {
									user->friends[j] = new_item->valueint;
								}
							}
						}
						// friend count
						user->friendsCount = cJSON_GetArraySize(new_item);
					}

					// public key
					new_item = cJSON_GetObjectItemCaseSensitive(user_json,
																"publicKey");
					if (cJSON_IsString(new_item) &&
						(new_item->valuestring != NULL)) {
						strcpy(user->publicKey, new_item->valuestring);
					}
					return 200;
				}
			}
		}
	}
	return 404;
}

/**
 * It reads the file, parses the JSON, finds the user with the given id, and
 * then copies all the data from the JSON to the user struct
 *
 * @param file_name the name of the file to read from
 * @param user a pointer to a User struct
 * @param id The id of the user to get.
 *
 * @return the status code of the operation.
 */
int getUserById(const char* file_name, User* user, int id) {
	const char* json_str = readFile(file_name);
	cJSON *json, *item;

	user = malloc(sizeof(User));

	json = cJSON_Parse(json_str);
	if (!json) {
		printf("Error parsing json\n");
		return 500;
	}
	// go to users array
	item = cJSON_GetObjectItemCaseSensitive(json, "Users");
	if (!cJSON_IsArray(item)) {
		printf("Error getting users array\n");
		return 500;
	}
	// find user
	for (int i = 0; i < cJSON_GetArraySize(item); i++) {
		cJSON* user_json = cJSON_GetArrayItem(item, i);
		if (cJSON_IsObject(user_json)) {
			item = cJSON_GetObjectItemCaseSensitive(user_json, "id");
			if (cJSON_IsNumber(item)) {
				if (item->valueint == id) {
					// parse all data from user to struct
					// get id
					item = cJSON_GetObjectItemCaseSensitive(user_json, "id");
					if (cJSON_IsNumber(item)) {
						user->id = item->valueint;
					}

					// get name
					item = cJSON_GetObjectItemCaseSensitive(user_json, "name");
					if (cJSON_IsString(item) && (item->valuestring != NULL)) {
						strcpy(user->name, item->valuestring);
					}

					// get password
					item =
						cJSON_GetObjectItemCaseSensitive(user_json, "password");
					if (cJSON_IsString(item) && (item->valuestring != NULL)) {
						strcpy(user->password, item->valuestring);
					}

					// get groups
					item =
						cJSON_GetObjectItemCaseSensitive(user_json, "groups");
					if (cJSON_IsArray(item)) {
						for (int j = 0; j < cJSON_GetArraySize(item); j++) {
							cJSON* group_json = cJSON_GetArrayItem(item, j);
							if (cJSON_IsObject(group_json)) {
								item = cJSON_GetObjectItemCaseSensitive(
									group_json, "id");
								if (cJSON_IsNumber(item)) {
									user->groups[j] = item->valueint;
								}
							}
						}
						// group count
						user->groupsCount = cJSON_GetArraySize(item);
					}

					// friends
					item =
						cJSON_GetObjectItemCaseSensitive(user_json, "friends");
					if (cJSON_IsArray(item)) {
						for (int j = 0; j < cJSON_GetArraySize(item); j++) {
							cJSON* friend_json = cJSON_GetArrayItem(item, j);
							if (cJSON_IsObject(friend_json)) {
								item = cJSON_GetObjectItemCaseSensitive(
									friend_json, "id");
								if (cJSON_IsNumber(item)) {
									user->friends[j] = item->valueint;
								}
							}
						}
						// friend count
						user->friendsCount = cJSON_GetArraySize(item);
					}

					// public key
					item = cJSON_GetObjectItemCaseSensitive(user_json,
															"publicKey");
					if (cJSON_IsString(item) && (item->valuestring != NULL)) {
						strcpy(user->publicKey, item->valuestring);
					}

					return 200;
				}
			}
		}
	}
	return 404;
}

/**
 * It reads a JSON file, parses it, and then stores the data in a struct
 *
 * @param file_name The name of the file to read from.
 * @param users a pointer to an array of User structs
 * @param count the number of users in the array
 *
 * @return the status code of the operation.
 */
int getAllUsers(const char* file_name, User** users, int* count) {
	// get all users
	// max 100 users

	const char* json_str = readFile(file_name);
	cJSON *json, *item;

	json = cJSON_Parse(json_str);
	if (!json) {
		printf("Error parsing json\n");
		return 500;
	}

	// go to users array
	item = cJSON_GetObjectItemCaseSensitive(json, "Users");
	if (!cJSON_IsArray(item)) {
		printf("Error getting users array");
		return 500;
	}

	// get count
	*count = cJSON_GetArraySize(item);

	*users = malloc(sizeof(User) * (*count));

	// add all users to array
	for (int i = 0; i < cJSON_GetArraySize(item); i++) {
		cJSON* user_json = cJSON_GetArrayItem(item, i);
		if (cJSON_IsObject(user_json)) {
			// parse all data from user to struct
			// get id
			item = cJSON_GetObjectItemCaseSensitive(user_json, "id");
			if (cJSON_IsNumber(item)) {
				(*users)[i].id = item->valueint;
			}

			// get name
			item = cJSON_GetObjectItemCaseSensitive(user_json, "name");
			if (cJSON_IsString(item) && (item->valuestring != NULL)) {
				strcpy((*users)[i].name, item->valuestring);
			}

			// get password
			item = cJSON_GetObjectItemCaseSensitive(user_json, "password");
			if (cJSON_IsString(item) && (item->valuestring != NULL)) {
				strcpy((*users)[i].password, item->valuestring);
			}

			// get groups
			item = cJSON_GetObjectItemCaseSensitive(user_json, "groups");
			if (cJSON_IsArray(item)) {
				for (int j = 0; j < cJSON_GetArraySize(item); j++) {
					cJSON* group_json = cJSON_GetArrayItem(item, j);
					if (cJSON_IsObject(group_json)) {
						item =
							cJSON_GetObjectItemCaseSensitive(group_json, "id");
						if (cJSON_IsNumber(item)) {
							(*users)[i].groups[j] = item->valueint;
						}
					}
				}
				// group count
				(*users)[i].groupsCount = cJSON_GetArraySize(item);
			}

			// friends
			item = cJSON_GetObjectItemCaseSensitive(user_json, "friends");
			if (cJSON_IsArray(item)) {
				for (int j = 0; j < cJSON_GetArraySize(item); j++) {
					cJSON* friend_json = cJSON_GetArrayItem(item, j);
					if (cJSON_IsObject(friend_json)) {
						item =
							cJSON_GetObjectItemCaseSensitive(friend_json, "id");
						if (cJSON_IsNumber(item)) {
							(*users)[i].friends[j] = item->valueint;
						}
					}
				}
				// friend count
				(*users)[i].friendsCount = cJSON_GetArraySize(item);
			}

			// public key
			item = cJSON_GetObjectItemCaseSensitive(user_json, "publicKey");
			if (cJSON_IsString(item) && (item->valuestring != NULL)) {
				strcpy((*users)[i].publicKey, item->valuestring);
			}
		}
	}
	return 200;
}

/**
 * It reads the file, parses the JSON, finds the group with the given name, and
 * then copies all the data from the JSON into the group struct
 *
 * @param file_name the name of the file to read from
 * @param group a pointer to a Group struct
 * @param name the name of the group
 *
 * @return the status code of the operation.
 */
int getGroupByName(const char* file_name, Group* group, char* name) {
	const char* json_str = readFile(file_name);
	cJSON *json, *item;

	json = cJSON_Parse(json_str);
	if (!json) {
		printf("Error parsing json\n");
		return 500;
	}

	// go to groups array
	item = cJSON_GetObjectItemCaseSensitive(json, "Groups");
	if (!cJSON_IsArray(item)) {
		printf("Error getting groups array");
		return 500;
	}

	// find group
	for (int i = 0; i < cJSON_GetArraySize(item); i++) {
		cJSON* group_json = cJSON_GetArrayItem(item, i);
		if (cJSON_IsObject(group_json)) {
			// get name
			item = cJSON_GetObjectItemCaseSensitive(group_json, "name");
			if (cJSON_IsString(item) && (item->valuestring != NULL)) {
				if (strcmp(item->valuestring, name) == 0) {
					// parse all data from group to struct
					// get id
					item = cJSON_GetObjectItemCaseSensitive(group_json, "id");
					if (cJSON_IsNumber(item)) {
						group->id = item->valueint;
					}

					// get name
					item = cJSON_GetObjectItemCaseSensitive(group_json, "name");
					if (cJSON_IsString(item) && (item->valuestring != NULL)) {
						strcpy(group->name, item->valuestring);
					}

					// get users
					item =
						cJSON_GetObjectItemCaseSensitive(group_json, "users");
					if (cJSON_IsArray(item)) {
						for (int j = 0; j < cJSON_GetArraySize(item); j++) {
							cJSON* user_json = cJSON_GetArrayItem(item, j);
							if (cJSON_IsObject(user_json)) {
								item = cJSON_GetObjectItemCaseSensitive(
									user_json, "id");
								if (cJSON_IsNumber(item)) {
									group->users[j] = item->valueint;
								}
							}
						}
						// user count
						group->usersCount = cJSON_GetArraySize(item);
					}

					// public key
					item = cJSON_GetObjectItemCaseSensitive(group_json,
															"publicKey");
					if (cJSON_IsString(item) && (item->valuestring != NULL)) {
						strcpy(group->publicKey, item->valuestring);
					}

					// private key
					item = cJSON_GetObjectItemCaseSensitive(group_json,
															"privateKey");
					if (cJSON_IsString(item) && (item->valuestring != NULL)) {
						strcpy(group->privateKey, item->valuestring);
					}

					return 200;
				}
			}
		}
	}
	return 404;
}

/**
 * It reads the file, parses the JSON, finds the group with the given id, and
 * then copies all the data from the JSON into the group struct
 *
 * @param file_name the name of the file to read from
 * @param group a pointer to a Group struct
 * @param id the id of the group to get
 *
 * @return the status code of the operation.
 */
int getGroupById(const char* file_name, Group* group, int id) {
	const char* json_str = readFile(file_name);
	cJSON *json, *item;

	json = cJSON_Parse(json_str);
	if (!json) {
		printf("Error parsing json\n");
		return 500;
	}

	// go to groups array
	item = cJSON_GetObjectItemCaseSensitive(json, "Groups");
	if (!cJSON_IsArray(item)) {
		printf("Error getting groups array");
		return 500;
	}

	// find group
	for (int i = 0; i < cJSON_GetArraySize(item); i++) {
		cJSON* group_json = cJSON_GetArrayItem(item, i);
		if (cJSON_IsObject(group_json)) {
			// get id
			item = cJSON_GetObjectItemCaseSensitive(group_json, "id");
			if (cJSON_IsNumber(item)) {
				if (item->valueint == id) {
					// parse all data from group to struct
					// get id
					item = cJSON_GetObjectItemCaseSensitive(group_json, "id");
					if (cJSON_IsNumber(item)) {
						group->id = item->valueint;
					}

					// get name
					item = cJSON_GetObjectItemCaseSensitive(group_json, "name");
					if (cJSON_IsString(item) && (item->valuestring != NULL)) {
						strcpy(group->name, item->valuestring);
					}

					// get users
					item =
						cJSON_GetObjectItemCaseSensitive(group_json, "users");
					if (cJSON_IsArray(item)) {
						for (int j = 0; j < cJSON_GetArraySize(item); j++) {
							cJSON* user_json = cJSON_GetArrayItem(item, j);
							if (cJSON_IsObject(user_json)) {
								item = cJSON_GetObjectItemCaseSensitive(
									user_json, "id");
								if (cJSON_IsNumber(item)) {
									group->users[j] = item->valueint;
								}
							}
						}
						// user count
						group->usersCount = cJSON_GetArraySize(item);
					}

					// public key
					item = cJSON_GetObjectItemCaseSensitive(group_json,
															"publicKey");
					if (cJSON_IsString(item) && (item->valuestring != NULL)) {
						strcpy(group->publicKey, item->valuestring);
					}

					// private key
					item = cJSON_GetObjectItemCaseSensitive(group_json,
															"privateKey");
					if (cJSON_IsString(item) && (item->valuestring != NULL)) {
						strcpy(group->privateKey, item->valuestring);
					}

					return 200;
				}
			}
		}
	}
	return 404;
}

/**
 * It reads a JSON file, parses it, and stores the data in a Group struct
 *
 * @param file_name the name of the file to read from
 * @param groups a pointer to an array of groups
 * @param count the number of groups in the array
 *
 * @return a pointer to an array of groups.
 */
int getAllGroups(const char* file_name, Group** groups, int* count) {
	const char* json_str = readFile(file_name);
	cJSON *json, *item;

	json = cJSON_Parse(json_str);
	if (!json) {
		printf("Error parsing json\n");
		return 500;
	}

	// go to groups array
	item = cJSON_GetObjectItemCaseSensitive(json, "Groups");
	if (!cJSON_IsArray(item)) {
		printf("Error getting groups array");
		return 500;
	}

	// get count
	*count = cJSON_GetArraySize(item);
	// allocate memory for groups
	*groups = (Group*)malloc(*count * sizeof(Group));

	// add all groups to array
	for (int i = 0; i < *count; i++) {
		cJSON* group_json = cJSON_GetArrayItem(item, i);
		if (cJSON_IsObject(group_json)) {
			// get id
			item = cJSON_GetObjectItemCaseSensitive(group_json, "id");
			if (cJSON_IsNumber(item)) {
				(*groups)[i].id = item->valueint;
			}

			// get name
			item = cJSON_GetObjectItemCaseSensitive(group_json, "name");
			if (cJSON_IsString(item) && (item->valuestring != NULL)) {
				strcpy((*groups)[i].name, item->valuestring);
			}

			// get users
			item = cJSON_GetObjectItemCaseSensitive(group_json, "users");
			if (cJSON_IsArray(item)) {
				for (int j = 0; j < cJSON_GetArraySize(item); j++) {
					cJSON* user_json = cJSON_GetArrayItem(item, j);
					if (cJSON_IsObject(user_json)) {
						item =
							cJSON_GetObjectItemCaseSensitive(user_json, "id");
						if (cJSON_IsNumber(item)) {
							(*groups)[i].users[j] = item->valueint;
						}
					}
				}
				// user count
				(*groups)[i].usersCount = cJSON_GetArraySize(item);
			}

			// public key
			item = cJSON_GetObjectItemCaseSensitive(group_json, "publicKey");
			if (cJSON_IsString(item) && (item->valuestring != NULL)) {
				strcpy((*groups)[i].publicKey, item->valuestring);
			}

			// private key
			item = cJSON_GetObjectItemCaseSensitive(group_json, "privateKey");
			if (cJSON_IsString(item) && (item->valuestring != NULL)) {
				strcpy((*groups)[i].privateKey, item->valuestring);
			}
		}
	}
	return 200;
}

/**
 * It reads the json file, parses it, adds a new user to the json file, and
 * writes the json file
 *
 * @param file_name the name of the file to read from
 * @param user the user to be added
 *
 * @return The return value is the status code of the operation.
 */
int addUser(const char* file_name, User* user) {
	// read file
	const char* json_str = readFile(file_name);
	cJSON *json, *item;

	json = cJSON_Parse(json_str);
	if (!json) {
		printf("Error parsing json\n");
		return 500;
	}

	// go to users array
	item = cJSON_GetObjectItemCaseSensitive(json, "Users");
	if (!cJSON_IsArray(item)) {
		printf("Error getting users array");
		return 500;
	}

	// create new user
	cJSON* new_user = cJSON_CreateObject();
	cJSON_AddNumberToObject(new_user, "id", user->id);
	cJSON_AddStringToObject(new_user, "name", user->name);
	cJSON_AddStringToObject(new_user, "password", user->password);
	cJSON_AddStringToObject(new_user, "publicKey", user->publicKey);
	// group array
	cJSON* groups = cJSON_CreateIntArray(user->groups, user->groupsCount);
	cJSON_AddItemToObject(new_user, "groups", groups);

	// friends
	cJSON* friends = cJSON_CreateIntArray(user->friends, user->friendsCount);
	cJSON_AddItemToObject(new_user, "friends", friends);

	// add user to array
	cJSON_AddItemToArray(item, new_user);

	// write to file
	writeFile(file_name, cJSON_Print(json));

	return 200;
}

int addGroup(const char* file_name, Group* group) {
	// read file
	const char* json_str = readFile(file_name);
	cJSON *json, *item;

	json = cJSON_Parse(json_str);
	if (!json) {
		printf("Error parsing json\n");
		return 500;
	}

	// go to groups array
	item = cJSON_GetObjectItemCaseSensitive(json, "Groups");
	if (!cJSON_IsArray(item)) {
		printf("Error getting groups array");
		return 500;
	}

	// create new group
	cJSON* new_group = cJSON_CreateObject();
	cJSON_AddNumberToObject(new_group, "id", group->id);
	cJSON_AddStringToObject(new_group, "name", group->name);
	cJSON_AddStringToObject(new_group, "description", group->description);
	cJSON_AddStringToObject(new_group, "publicKey", group->publicKey);
	cJSON_AddStringToObject(new_group, "privateKey", group->privateKey);

	// users array
	cJSON* users = cJSON_CreateIntArray(group->users, group->usersCount);
	cJSON_AddItemToObject(new_group, "users", users);

	// add group to array
	cJSON_AddItemToArray(item, new_group);

	// write to file
	writeFile(file_name, cJSON_Print(json));

	return 200;
}