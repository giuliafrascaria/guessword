#define _XOPEN_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>





//structs
struct pwd_hash {
	char * pwd;
	char * hash;
};

struct user_info {
	char * lowercase_name;
	char * lowercase_name_hash;
	char * uppercase_name;
	char * uppercase_name_hash;
};

//function definitions
void parse_top_250(char * salt, struct pwd_hash ** array);

char * do_pwd_hash(char *pwd, char *salt);

//let the cracking begin
int main (int argc, char ** argv)
{
	//check args
	if (argc != 3)
	{
		printf("usage: ./guessword path_to_pwd path_to_shadow\n");
		exit(EXIT_FAILURE);
	}


	//open files
	FILE * pwd = fopen(argv[1], "r");
	if (pwd == NULL)
	{
		printf("error opening pwd file, check the path\n");
		exit(EXIT_FAILURE);
	}
	FILE * shd = fopen(argv[2], "r");
	if (shd == NULL)
	{
		printf("error opening shd file, check the path\n");
		exit(EXIT_FAILURE);
	}



	//retrieve salt from shadow file
	char *lineptr = NULL;
	size_t n = 0;
	ssize_t len = getline(&lineptr, &n, shd);
	if (len < 0)
	{
		printf("error reading shadow file for salt\n");
		exit(EXIT_FAILURE);
	}



	char * salt = malloc(6*sizeof(char));
	if (salt == NULL)
	{
		printf("error allocating salt\n");
		exit(EXIT_FAILURE);
	}
	memcpy(salt, &lineptr[7], 5);
	printf("%s\n", salt);
	free(lineptr);


	//open dictionaries and create all necessary data structures
	struct pwd_hash *pwd_hashes_250 = malloc(250 * sizeof(struct pwd_hash));
	parse_top_250(salt, &pwd_hashes_250);

	for (int i = 0; i < 250; i++)
	{
		printf("%s %s\n", (pwd_hashes_250[i]).hash, (pwd_hashes_250[i]).pwd);
	}


	//continue this part


	//check against user password file
	len = 0;
	fseek(shd, 0, SEEK_SET);


	printf("starting to check\n");
	while (len != -1)
	{
		char *lineptr = NULL;
		size_t n = 0;
		len = getline(&lineptr, &n, shd);

		char * user = malloc(6*sizeof(char));
		if (user == NULL)
		{
			printf("error allocating user\n");
			exit(EXIT_FAILURE);
		}

		char * hash = malloc(32*sizeof(char));
		if (hash == NULL)
		{
			printf("error allocating user\n");
			exit(EXIT_FAILURE);
		}

		user = memcpy(user, &lineptr[0], 6);
		hash = memcpy(hash, &lineptr[7], 28);
		//printf("%s %s\n", user, hash);

		//printf("starting to check\n");
		for (int i = 0; i < 250; i++)
		{
			if(strcmp(hash, (pwd_hashes_250[i]).hash) == 0)
			{
				printf("%s:%s\n", user, (pwd_hashes_250[i]).pwd);
			}
			//qprintf("%s %s\n", (pwd_hashes_250[i]).hash, (pwd_hashes_250[i]).pwd);
		}
	}



	exit(EXIT_SUCCESS);
}

void parse_top_250(char * salt, struct pwd_hash ** array)
{
	//open file
	FILE * top250 = fopen("top250.txt", "r");
	if (top250 == NULL)
	{
		printf("error opening top250\n");
		exit(EXIT_FAILURE);
	}

	//passwords are the last element of the line
	ssize_t len = 0;
	int index = 0;
	//struct pwd_hash * array = malloc(250 * sizeof(struct pwd_hash));

	while(len >= 0)
	{
		char *lineptr = NULL;
		size_t n = 0;
		len = getline(&lineptr, &n, top250);
		//printf("%s", lineptr);

		//get the password from the line
		char * token = NULL;
		if (len > 0)
		{

			token = strtok(lineptr, "\t");
			token = strtok(NULL, "\t");
			token = strtok(NULL, "\t");
			token = strtok(NULL, "\n");
			printf("%s\n", token);

			//compute pwd hash and return

			(*array)[index].pwd = malloc(32 * sizeof(char));
			(*array)[index].pwd = strcpy((*array)[index].pwd, token);
			//(*array)[index].pwd = token;
			(*array)[index].hash = malloc(32 * sizeof(char));
			char * hash = do_pwd_hash(token, salt);
			(*array)[index].hash = strcpy((*array)[index].hash, hash);

		}


		index++;
		free(lineptr);
	}

}

char * do_pwd_hash(char * pwd, char * salt)
{
	//char *enc = "$encrypted";
	char *final_salt = malloc(sizeof(salt) + 1);

	final_salt = strncpy(final_salt, salt, 5);
	final_salt[5] = '\0';
	//strcat(final_salt, "\0");

	printf("password %s\n", pwd);
	printf("salt %s\n", salt);

	char * hash = crypt(pwd, salt);
	printf("%s\n", hash);
	return hash;

}
