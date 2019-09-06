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
	char * surname;
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


//global data
#define PTHREAD_N 8
pthread_t threads[PTHREAD_N];
pthread_t dict_threads[PTHREAD_N];
struct pwd_hash * dictionary_array[PTHREAD_N];
volatile int count;
pthread_mutex_t count_mutex;
//int partition_sizes[8] = {32686, 32687, 32687, 32686, 32686, 32686, 32686, 32680};
//int partition_sizes[8] = {43194, 43194, 43194, 43194, 43194, 43194, 43194, 43194};
volatile int partition_sizes[8] = {58396, 58396, 58396, 58396, 58396, 58396, 58396, 60387};

//function definitions
void parse_top_250(char * salt, struct pwd_hash ** array);
char * do_pwd_hash(char *pwd, char *salt);
char * check_surname_patterns(char * surname, char * hash, char *salt);
char * check_name_patterns(char * name, char * hash, char * salt);
char * test_name_year(char *name, char *salt, char *hash);
char * leet_sub(char *name, char letter);
void * create_dictionary_partition(void *arg);
void * crack_partition(void * arg);
void join_threads(void);
void join_threads_dict(void);
void count_increment();


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


		char * surname = strtok_r(NULL, ",", &buffer);
		int s_len = strlen(surname);
		for (int i = 0; i < s_len; i++)
		{
			//printf("here\n");
			if (strcmp(&surname[i], " ") == 0)
			{
				//double name, save only one;
				surname = strtok_r(NULL, " ", &buffer);
			}
		}
		printf("%s\n", surname);
		node->surname = malloc(40 * sizeof(char));
		node->surname = strcpy(node->surname, surname);

		node->hash = malloc(28*sizeof(char));
		node->hash = memcpy(node->hash, &shdline[7], 28);

		node->found = 0;
		node->uid = uid%8;
		uid++;
		//printf("%d:%s:%s:%s\n", node->uid, node->username, node->hash, node->full_name);

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

	for(current = head; current ; current=current->next)
	{
		//for each user check name-dependant stuff
		char *success = check_name_patterns(current->full_name, current->hash, salt);
		if (success != NULL)
		{
			printf("%s:%s\n", current->username, success);
			fflush(stdout);
			current->found = 1;
			count_increment();
			matches++;
		}
		else
		{
			char *success = check_surname_patterns(current->surname, current->hash, salt);
			if (success != NULL)
			{
				printf("%s:%s\n", current->username, success);
				fflush(stdout);
				current->found = 1;
				count_increment();
				matches++;
			}
		}

  }

	//open dictionaries and create all necessary data structures
	//struct pwd_hash *pwd_hashes_250 = malloc(478 * sizeof(struct pwd_hash));
	//struct pwd_hash *pwd_hashes_250 = malloc(502 * sizeof(struct pwd_hash));
	//struct pwd_hash *pwd_hashes_250 = malloc(10022 * sizeof(struct pwd_hash));

	struct pwd_hash *pwd_hashes_250 = malloc(261490 * sizeof(struct pwd_hash));

	dictionary_array[0] = malloc(partition_sizes[0] * sizeof(struct pwd_hash));

	dictionary_array[1] = malloc(partition_sizes[1] * sizeof(struct pwd_hash));
	dictionary_array[2] = malloc(partition_sizes[2] * sizeof(struct pwd_hash));
	dictionary_array[3] = malloc(partition_sizes[3] * sizeof(struct pwd_hash));
	dictionary_array[4] = malloc(partition_sizes[4] * sizeof(struct pwd_hash));
	dictionary_array[5] = malloc(partition_sizes[5] * sizeof(struct pwd_hash));
	dictionary_array[6] = malloc(partition_sizes[6] * sizeof(struct pwd_hash));
	dictionary_array[7] = malloc(partition_sizes[7] * sizeof(struct pwd_hash));
	//parse_top_250(salt, &pwd_hashes_250);

	char *filename[8] = {"d11.txt", "d22.txt", "d33.txt", "d44.txt", "d55.txt", "d66.txt", "d77.txt", "d88.txt"};

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

	printf("finished parsing dictionaries\n");
	//return 0;

//------------------------------------------------------------------------------------------------------------------

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
	//crack_partition((void *) &args[0]);
	join_threads();
	printf("found %d\n", count);
	printf("the end\n");

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


char * check_surname_patterns(char * surname, char * hash, char *salt)
{
	//lowercase
	surname[0] = tolower(surname[0]);
	//printf("%s\n", name);
	char * test_hash1 = do_pwd_hash(surname, salt);
	if (strcmp(test_hash1, hash) == 0)
	{
		return surname;
	}
	else
	{
		ssize_t namesize = strlen(surname);
		for (int i = 0; i < namesize; i++)
		{
			surname[i] = toupper(surname[i]);
		}
		char * test_hash2 = do_pwd_hash(surname, salt);
		if (strcmp(test_hash2, hash) == 0)
		{
			return surname;
		}
	}
	return NULL;
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
			}
			char * test_hash3 = test_name_year(name, salt, hash);
			if( test_hash3 != NULL)
			{
				return test_hash3;
			}
			else
			{
				//leet substitutions
				char * test_a = leet_sub(name, 'a');
				if (test_a != NULL)
				{
					char * test_hash4 = do_pwd_hash(test_a, salt);
					if (strcmp(test_hash4, hash) == 0)
					{
						return test_a;
					}
				}
				else
				{
					char * test_s = leet_sub(name, 's');
					if (test_s != NULL)
					{
						char * test_hash5 = do_pwd_hash(test_s, salt);
						if (strcmp(test_hash5, hash) == 0)
						{
							return test_s;
						}
					}
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
	char *years[44] = {"1960", "1961", "1962", "1963", "1964", "1965", "1966", "1967", "1968", "1969", "1970", "1971", "1972", "1973", "1974", "1975", "1976", "1977", "1978", "1979", "1980",
										"60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "80", "xor", "zorz"};

	for (int i = 0; i < 44; i ++)
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


char * leet_sub(char *name, char letter)
{
	int len = strlen(name);
	switch (letter)
	{
		case 'a':
			if(strchr(name, letter) != NULL)
			{
				for (int i = 0; i < len; i++)
				{
					if (strcmp(&name[i], "a") == 0)
					{
						strcpy(&name[i], "^");
						return name;
					}
				}
			}
			return NULL;
		case 's':
			if(strchr(name, letter) != NULL)
			{
				for (int i = 0; i < len; i++)
				{
					if (strcmp(&name[i], "s") == 0)
					{
						strcpy(&name[i], "$");
						return name;
					}
				}
			}
			return NULL;
		default:
			return NULL;
	}

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
	//struct pwd_hash * table = a.hash_table;
	struct user_details * current;

	int found = 0;

	//printf("my tid %d\n", a.tid);
	for(current = head; current ; current=current->next)
	{
		//for each user that is still missing, check all hash table entries
		found = 0;
		for (int i = 0; i < 8; i++)
		{
			if ((current->found == 0) && (current->uid == a.tid) && (found == 0))
			{
				//printf("here\n");
				for (int j = 0; j < partition_sizes[i]; j++)
				{
					//printf("dict %d, pos %d, %s\n", i, j, dictionary_array[i][j].pwd);
					if( dictionary_array[i][j].hash  != NULL)
					{
						if(strcmp(current->hash, dictionary_array[i][j].hash) == 0)
						{
							//use mutex and fflush buffer
							printf("%s:%s\n", current->username, dictionary_array[i][j].pwd);
							fflush(stdout);
							count_increment();

							found = 1;
							break;
						}
					}
				}
			}
			if (found == 1)
			{
				break;
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

void count_increment()
{
	 pthread_mutex_lock(&count_mutex);
	 count++;
	 if ((count % 300) == 0)
	 printf("\n\n%d\n\n", count);
	 pthread_mutex_unlock(&count_mutex);
}
