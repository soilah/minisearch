char *docname;
unsigned int k, Map_Size, numOfWords, queryNum;

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
