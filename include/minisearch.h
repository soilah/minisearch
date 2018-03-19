#ifndef __MINISEARCH_H__
#define __MINISEARCH_H__

#include <argp.h>

char *docname;
unsigned int k;
unsigned int Map_Size;
unsigned int numOfWords;

struct minisearh_args {
	char *docname;
	unsigned int k;
	unsigned int Map_Size;
	unsigned int numOfWords;
};


#define MINISEARCH_MAX_QUERIES 10
typedef enum {
	SEARCH = 0,
	DF,
	TF,
	EXIT
} cmd_t;


struct command {
	cmd_t cmd_id;
	char *args[MINISEARCH_MAX_QUERIES];
	size_t nr_args;
};

struct command *minisearch_get_cmd(void);
void minisearch_free_cmd(struct command *);
void getArgs(int, char **);

#endif /* __MINISEARCH_H__ */
