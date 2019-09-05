#define _XOPEN_SOURCE

#define _GNU_SOURCE
#include <unistd.h>        /* See feature_test_macros(7) */
#include <crypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>


//structs
struct pwd_hash {
	char * pwd;
	char * hash;
};

struct user_details {
	char * username;
	char * full_name;
	char * hash;
	int found;
	int uid;
	struct user_details * next;
};

struct thread_args_dict {
	char * filename;
	char * salt;
	int tid;
};

struct thread_args {
	int tid;
	struct user_details * user_list;
	struct pwd_hash * hash_table;
};

#define PTHREAD_N 8
pthread_t threads[PTHREAD_N];
pthread_t dict_threads[PTHREAD_N];
struct pwd_hash * dictionary_array[PTHREAD_N];

//function definitions
void parse_top_250(char * salt, struct pwd_hash ** array);
char * do_pwd_hash(char *pwd, char *salt);
char * check_name_patterns(char * name, char * hash, char * salt);
char * test_name_year(char *name, char *salt, char *hash);
void * create_dictionary_partition(void *arg);
void * crack_partition(void * arg);
void join_threads(void);
void join_threads_dict(void);


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
	char *lineptr = malloc(32 * sizeof(char));
	lineptr = fgets(lineptr, 32, shd);
	if (lineptr == NULL)
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
	fseek(shd, 0, SEEK_SET);



	//create user list
	struct user_details * head, *current;
	head = NULL;
	current = NULL;

	int uid = 0;

	char * pwdline = malloc(128 * sizeof(char));
	char * shdline = malloc(64 * sizeof(char));
	while (fgets(pwdline, 128, pwd))
	{
		shdline = fgets(shdline, 64, shd);

		struct user_details * node = malloc(sizeof(struct user_details));
		node->username = malloc(6*sizeof(char));
		node->username = memcpy(node->username, &pwdline[0], 6);

		node->full_name = malloc(20 * sizeof(char));
		//node->full_name = strtok(&(pwdline[19]), ",");
		char *buffer = malloc(32*sizeof(char));
		strtok_r(pwdline, ":", &buffer);
		strtok_r(NULL, ":", &buffer);
		strtok_r(NULL, ":", &buffer);
		strtok_r(NULL, ":", &buffer);
		char * name = strtok_r(NULL, " ", &buffer);

		node->full_name = strcpy(node->full_name, name);

		node->hash = malloc(28*sizeof(char));
		node->hash = memcpy(node->hash, &shdline[7], 28);

		node->found = 0;
		node->uid = uid%8;
		uid++;
		printf("%d:%s:%s:%s\n", node->uid, node->username, node->hash, node->full_name);

		if(head == NULL){
      current = head = node;
    } else{
      current = current->next = node;
    }

	}


	//read user names and test nameYYYY first, before going to threads
	//delete found users from worklist
	int matches = 0;
	printf("starting to check\n");
/*
	for(current = head; current ; current=current->next)
	{
		//for each user check name-dependant stuff
		char *success = check_name_patterns(current->full_name, current->hash, salt);
		if (success != NULL)
		{
			printf("%s:%s\n", current->username, success);
			current->found = 1;
			matches++;
		}
  }*/

	//open dictionaries and create all necessary data structures
	//struct pwd_hash *pwd_hashes_250 = malloc(478 * sizeof(struct pwd_hash));
	//struct pwd_hash *pwd_hashes_250 = malloc(502 * sizeof(struct pwd_hash));
	//struct pwd_hash *pwd_hashes_250 = malloc(10022 * sizeof(struct pwd_hash));


	struct pwd_hash *pwd_hashes_250 = malloc(261490 * sizeof(struct pwd_hash));

	dictionary_array[0] = malloc(32686 * sizeof(struct pwd_hash));
	dictionary_array[1] = malloc(32687 * sizeof(struct pwd_hash));
	dictionary_array[2] = malloc(32687 * sizeof(struct pwd_hash));
	dictionary_array[3] = malloc(32687 * sizeof(struct pwd_hash));
	dictionary_array[4] = malloc(32687 * sizeof(struct pwd_hash));
	dictionary_array[5] = malloc(32687 * sizeof(struct pwd_hash));
	dictionary_array[6] = malloc(32687 * sizeof(struct pwd_hash));
	dictionary_array[7] = malloc(32680 * sizeof(struct pwd_hash));
	//parse_top_250(salt, &pwd_hashes_250);

	char *filename[8] = {"d1.txt", "d2.txt", "d3.txt", "d4.txt", "d5.txt", "d6.txt", "d7.txt", "d8.txt"};

	printf("starting the dictionary threads\n");
	struct thread_args_dict args_dict[PTHREAD_N];

	for (size_t i = 0; i < PTHREAD_N; i++) {
			args_dict[i].tid = i;
			args_dict[i].salt = salt;
			args_dict[i].filename = filename[i];
			int success = pthread_create(&dict_threads[i], NULL, create_dictionary_partition, &args_dict[i]);
			if (success != 0)
			{
				printf("pthread_create fail\n");
				exit(EXIT_FAILURE);
			}
	}
	join_threads_dict();

	for (int i = 0; i < 32686; i++)
	{
		printf("%s %s\n", dictionary_array[0][i].hash, dictionary_array[0][i].pwd);
	}
	for (int i = 0; i < 32687; i++)
	{
		printf("%s %s\n", dictionary_array[1][i].hash, dictionary_array[1][i].pwd);
	}
	printf("the end\n");
	return 0;

//------------------------------------------------------------------------------------------------------------------
	FILE * top250 = fopen("dictionary.txt", "r");
	if (top250 == NULL)
	{
		printf("error opening top250\n");
		exit(EXIT_FAILURE);
	}
	int i = 0;
	char *passwd = malloc(32 * sizeof(char));
	while (passwd != NULL)
	{
		passwd = fgets(passwd, 32, top250);
		if (passwd != NULL)
		{
			char * token = strtok(passwd, "\n");
			//printf("%s - %s\n", token, salt);
			//char *hash = crypt(token, salt);
			//printf("hash %s\n", hash);

			(pwd_hashes_250[i]).pwd = malloc(32 * sizeof(char));
			(pwd_hashes_250[i]).pwd = strcpy((pwd_hashes_250[i]).pwd, token);
			//(*array)[index].pwd = token;
			(pwd_hashes_250[i]).hash = malloc(32 * sizeof(char));
			char * hash = do_pwd_hash(token, salt);
			(pwd_hashes_250[i]).hash = strcpy((pwd_hashes_250[i]).hash, hash);
		}
		i++;

	}
	//continue this part
//------------------------------------------------------------------------------------------------------------------

	//check against top250 passwords
	//len = 0;
	printf("starting the crack threads\n");
	struct thread_args args[PTHREAD_N];

	for (size_t i = 0; i < PTHREAD_N; i++) {
			args[i].tid = i;
			args[i].user_list = head;
			args[i].hash_table = pwd_hashes_250;
			int success = pthread_create(&threads[i], NULL, crack_partition, &args[i]);
			if (success != 0)
			{
				printf("pthread_create fail\n");
				exit(EXIT_FAILURE);
			}
	}
	join_threads();
	printf("the end\n");
	return 0;


	printf("starting to check\n");

	for(current = head; current ; current=current->next)
	{
		//for each user check top250
		//printf("working on %s, %s\n", current->username, current->full_name);
		if (current->found == 0)
		{
			//for (int i = 0; i < 10022; i++)
			for (int i = 0; i < 261490; i++)
			{
					if(strcmp(current->hash, (pwd_hashes_250[i]).hash) == 0)
					{
						printf("%s:%s\n", current->username, (pwd_hashes_250[i]).pwd);
						matches++;
						break;
					}
			}
		}
  }
	printf("found %d\n", matches);
	//exit(EXIT_SUCCESS);

	exit(EXIT_SUCCESS);
}


char * do_pwd_hash(char * pwd, char * salt)
{
	//char *enc = "$encrypted";
	char *final_salt = malloc(sizeof(salt) + 1);

	final_salt = strncpy(final_salt, salt, 5);
	final_salt[5] = '\0';
	//strcat(final_salt, "\0");

	char * hash = crypt(pwd, salt);
	//printf("%s\n", hash);
	return hash;

}

char * do_pwd_hash_reentrant(char * pwd, char * salt)
{
	//char *enc = "$encrypted";
	char *final_salt = malloc(sizeof(salt) + 1);

	final_salt = strncpy(final_salt, salt, 5);
	final_salt[5] = '\0';
	//strcat(final_salt, "\0");

	struct crypt_data data;
	data.initialized = 0;
	char * hash = crypt_r(pwd, salt, &data);
	//printf("%s\n", hash);
	return hash;

}


char * check_name_patterns(char * name, char * hash, char * salt)
{
	//lowercase
	name[0] = tolower(name[0]);
	//printf("%s\n", name);
	char * test_hash1 = do_pwd_hash(name, salt);
	if (strcmp(test_hash1, hash) == 0)
	{
		return name;
	}
	else
	{
		ssize_t namesize = strlen(name);
		for (int i = 0; i < namesize; i++)
		{
			name[i] = toupper(name[i]);
		}
		char * test_hash2 = do_pwd_hash(name, salt);
		if (strcmp(test_hash2, hash) == 0)
		{
			return name;
		}
		else
		{
			for (int i = 0; i < namesize; i++)
			{
				name[i] = tolower(name[i]);
				char * test_hash3 = test_name_year(name, salt, hash);
				if( test_hash3 != NULL)
				{
					return test_hash3;
				}
			}
		}
	}
	return NULL;
	//uppercase

	//birth year
}

char * test_name_year(char *name, char *salt, char *hash)
{
	char *years[42] = {"1960", "1961", "1962", "1963", "1964", "1965", "1966", "1967", "1968", "1969", "1970", "1971", "1972", "1973", "1974", "1975", "1976", "1977", "1978", "1979", "1980",
										"60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "80"};

	for (int i = 0; i < 42; i ++)
	{

		char *new_combination = malloc(20 * sizeof(char));
		new_combination = strcpy(new_combination, name);
		new_combination = strcat(new_combination, years[i]);

		char *test_hash = do_pwd_hash(new_combination, salt);
		if (strcmp(test_hash, hash) == 0)
		{
			return new_combination;
		}
	}
	return NULL;
}


void * create_dictionary_partition(void *arg)
{
	struct thread_args_dict a = *(struct thread_args_dict *) arg;

	FILE * f = fopen(a.filename, "r");
	if (f == NULL)
	{
		printf("error opening top250\n");
		exit(EXIT_FAILURE);
	}
	int i = 0;
	char *passwd = malloc(32 * sizeof(char));
	while (passwd != NULL)
	{
		passwd = fgets(passwd, 32, f);
		if (passwd != NULL)
		{
			char * token = strtok(passwd, "\n");
			//printf("%s - %s\n", token, salt);
			//char *hash = crypt(token, salt);
			//printf("hash %s\n", hash);

			(dictionary_array[a.tid])[i].pwd = malloc(32 * sizeof(char));
			(dictionary_array[a.tid])[i].pwd = strcpy((dictionary_array[a.tid])[i].pwd, token);
			//(*array)[index].pwd = token;
			(dictionary_array[a.tid])[i].hash = malloc(32 * sizeof(char));
			char * hash = do_pwd_hash_reentrant(token, a.salt);
			(dictionary_array[a.tid])[i].hash = strcpy((dictionary_array[a.tid])[i].hash, hash);
		}
		i++;

	}
	return NULL;
}


void * crack_partition(void * arg)
{
	struct thread_args a = *(struct thread_args *) arg;

	struct user_details * head = a.user_list;
	struct pwd_hash * table = a.hash_table;
	struct user_details * current;

	for(current = head; current ; current=current->next)
	{
		//for each user that is still missing, check all hash table entries
		if ((current->found == 0) && (current->uid == a.tid))
		{
			for (int i = 0; i < 261490; i++)
			{
					if(strcmp(current->hash, (table[i]).hash) == 0)
					{
						//use mutex and fflush buffer
						printf("%s:%s\n", current->username, (table[i]).pwd);
						break;
					}
			}
		}

  }
	return NULL;
}

void join_threads(void)
{
    for (size_t i = 0; i < PTHREAD_N; i++)
        if (pthread_join(threads[i], NULL) != 0)
				{
            printf("pthread_join failed\n");
						exit(EXIT_FAILURE);
				}
}

void join_threads_dict(void)
{
    for (size_t i = 0; i < PTHREAD_N; i++)
        if (pthread_join(dict_threads[i], NULL) != 0)
				{
            printf("pthread_join failed\n");
						exit(EXIT_FAILURE);
				}
}
