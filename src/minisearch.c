#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "minisearch.h"
#include "structs.h"
#include "assert.h"


static size_t parse_argument(struct command *cmd, size_t id)
{
	assert(id < MINISEARCH_MAX_QUERIES);

	char *token = strtok(NULL, " \n");
	if (!token)
		return 0;

	size_t len = strlen(token);
	cmd->args[id] = malloc(len + 1);
	if (!cmd->args[id]) {
		perror("Could not allocate memory for command argument");
		exit(1);
	}
	strncpy(cmd->args[id], token, len);
	cmd->args[id][len] = '\0';

	return 1;
}

struct command *minisearch_get_cmd(void)
{
	char *input = NULL, *token = NULL;
	size_t length = 0;
	struct command *cmd;
	ssize_t ret;

	cmd = malloc(sizeof(struct command));
	if (!cmd) {
		perror("Could not allocate memory for command");
		exit(1);
	}

	while (1) {
		printf("> ");
		ret = getline(&input, &length, stdin);
		if (ret == -1) {
			perror("Error reading input line");
			exit(1);
		}

		/* find command */
		token = strtok(input, " \n");
		if (!token) {
			printf("Malformed command!\n");
			free(input);
			continue;
		}

		if (!strncmp(token, "search", 6)) {
			cmd->cmd_id = SEARCH;
		} else if (!strncmp(token, "df", 2)) {
			cmd->cmd_id = DF;
		} else if (!strncmp(token, "tf", 2)) {
			cmd->cmd_id = TF;
		} else if (!strncmp(token, "exit", 4)) {
			cmd->cmd_id = EXIT;
		} else {
			printf("Unknown command!\n");
			free(input);
			continue;
		}

		/* We don't need to parse the arguments
		 * for an EXIT command */
		if (cmd->cmd_id == EXIT) {
			cmd->nr_args = 0;
			free(input);
			return cmd;
		}

		/* DF command has at most one argument */
		if (cmd->cmd_id == DF) {
			cmd->nr_args = parse_argument(cmd, 0);
			free(input);
			return cmd;
		}

		/* TF command has exactly two arguments */
		if (cmd->cmd_id == TF) {
			cmd->nr_args = 0;
			cmd->nr_args += parse_argument(cmd, 0);
			cmd->nr_args += parse_argument(cmd, 1);

			if (cmd->nr_args != 2) {
				printf("TF command requires two arguments\n");
				free(input);
				continue;
			}

			free(input);
			return cmd;
		}


		/* At this point it has to be a SEARCH command */
		assert(cmd->cmd_id == SEARCH);

		/* SEARCH command takes at least one and at most
		 * MINISEARCH_MAX_QUERIES arguments */
		for (cmd->nr_args = 0; cmd->nr_args < MINISEARCH_MAX_QUERIES; cmd->nr_args++) {
			if (!parse_argument(cmd, cmd->nr_args))
				break;
		}

		if (!cmd->nr_args) {
			printf("SEARCH command requires at least one argument\n");
			free(input);
			continue;
		}

		printf("search[%lu]: ", cmd->nr_args);
		for (size_t i = 0; i < cmd->nr_args; ++i) {
			printf("%s ", cmd->args[i]);
		}
		printf("\n");

		free(input);
		return cmd;
	}
}


void minisearch_free_cmd(struct command *cmd)
{
	size_t i;

	for (i = 0; i < cmd->nr_args; ++i)
		free(cmd->args[i]);

	free(cmd);
}

// Calculates the score of each Document
scoreArray *score(ResArray *result_array,map *docmap, unsigned int queryNum){
	unsigned int i, j,N=Map_Size,count,D;
	double avgdl = (double)numOfWords/(double)N, sum;
	int *n = malloc(sizeof(int)*queryNum);
	postingList *plist=NULL;
	for(i=0; i<queryNum; i++){
		count=0;
		postingList *temp = result_array[i].plist;
		if(result_array[i].query){
			while(temp){
				count++;
				temp = temp->next;
			}
		}
		n[i]=count;
	}
	scoreArray *head_score = NULL;
	for (i=0; i<queryNum; i++){
		if(result_array[i].query){
			plist = result_array[i].plist;
			while(plist){
				count=0;
				D=1;
				for(count=0; count<strlen(docmap[plist->id].document); count++){
					if(isspace(docmap[plist->id].document[count]) && (!isspace(docmap[plist->id].document[count+1])))
						D++;
				}
				sum = 0;
				for(j=0; j<queryNum; j++){
					if(!exists(head_score,plist->id)){
						postingList *temp = result_array[j].plist;
						while(temp){
							if(plist->id == temp->id){
								sum+=log10((N-n[j] + 0.5)/(n[j]+0.5))*(((double)temp->freq*2.2)/((double)temp->freq + 1.2*(0.25+0.75*((double)D/avgdl))));
								break;
							}
							temp=temp->next;
						}
					}
				}
				if(!exists(head_score,plist->id))
					addscore(&head_score,plist->id, sum);
				plist = plist->next;
			}
		}
	}
	free(n);
	return head_score;
}


// Prints results in search command
void print_results(ResArray *result_array, size_t queryNum, scoreArray *score_array, map *Docmap){
	// GETTING TERMINAL SIZE
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	scoreArray *temp = score_array;
	postingList *plist=NULL;
	unsigned int i=0,j,pos, count=0;
	char *temp_sent=NULL, *sentence=NULL, inits[25];
	// sentence holds the whole document to be printed, temp_sent holds the
	// blanks and "^"s to be printed below the sentence and inits holds
	// the string to be printed at the beginning of each sentence wich is
	// the score, document id etc.
	while(temp && count<k){
		sprintf(inits,"%d.( %d) [%f] ",i,temp->docId,temp->score);
		j = 0;
		char found=0;
		for(j=0; j<queryNum; j++){
			plist = result_array[j].plist;
			while(plist){
				if(plist->id == temp->docId){
					sentence = malloc(strlen(Docmap[plist->id].document) + 1);
					strcpy(sentence,Docmap[plist->id].document);
					found=1;
					break;
				}
				plist=plist->next;
			}
			if(found)
				break;
		}
		if(!sentence)
			break;
		temp_sent= malloc(strlen(sentence)+1);
		strcpy(temp_sent,sentence);
		for(j=0; j<queryNum; j++){
			if (!result_array[j].query)
				continue;
			char *sub=NULL;
			sub=strstr(sentence,result_array[j].query);
			while(sub!=NULL){
				// This checks if query is part of another word at the beggining
				while(sub[strlen(result_array[j].query)] != ' ' && strcmp(sub,result_array[j].query) && sub[strlen(result_array[j].query)] != '\t'){
					sub++;
					sub = strstr(sub,result_array[j].query);
					if(!sub)
						break;
				}
				if(!sub)
					break;
				// This checks if query is part of another word at the end
				if(strcmp(sub,sentence)){
					while(!(isspace(sentence[strlen(sentence)-strlen(sub)-1]))){
						sub++;
						sub = strstr(sub,result_array[j].query);
						if(!sub)
							break;
					}
				}
				if(!sub)
					break;
				if(strlen(result_array[j].query)==1 && (sub[strlen(result_array[j].query)] != ' ' || sub[strlen(result_array[j].query)] != '\t')){
					while(!isspace(*sub))
						sub++;
					sub = strstr(sub,result_array[j].query);
				}

				pos=strlen(temp_sent)-strlen(sub);
				while(temp_sent[pos] != ' ' && pos < strlen(temp_sent)){
					temp_sent[pos]='^';
					pos++;
				}
				sub += strlen(result_array[j].query);
				sub = strstr(sub,result_array[j].query);
			}
		}
		pos=0;
		while(pos < strlen(temp_sent)){
			if (temp_sent[pos] != '^')
				temp_sent[pos] = ' ';
			pos++;
		}
		size_t blank_count;
		unsigned int times_to_print = strlen(sentence)/(w.ws_col-strlen(inits)), times_printed=0;
		// This is the case that the document fits into terminal screen.
		if(strlen(sentence) < w.ws_col){
			blank_count=0;
			printf("%s",inits);
			printf("%s\n",sentence);
			while(blank_count++ < strlen(inits))
				printf(" ");
			printf("%s\n",temp_sent);
		}
		// In this case document does not fit into terminal screen.
		else{
			char *str1 = malloc(w.ws_col-strlen(inits)+1), *str2 = malloc(w.ws_col-strlen(inits)+1);
			memset(str1,0,w.ws_col-strlen(inits)+1);
			memset(str2,0,w.ws_col-strlen(inits)+1);
			strncpy(str1,sentence,w.ws_col-strlen(inits));
			strncpy(str2,temp_sent,w.ws_col-strlen(inits));
			printf("%s",inits);
			printf("%s\n",str1);
			blank_count = 0;
			while(blank_count++ < strlen(inits))
				printf(" ");
			printf("%s\n",str2);
			sentence+=w.ws_col - strlen(inits);
			temp_sent+=w.ws_col - strlen(inits);
			while(times_printed < times_to_print){
				strncpy(str1,sentence,w.ws_col-strlen(inits));
				strncpy(str2,temp_sent,w.ws_col-strlen(inits));
				blank_count=0;
				while(blank_count++ < strlen(inits))
					printf(" ");
				printf("%s\n",str1);
				blank_count = 0;
				while(blank_count++ < strlen(inits))
					printf(" ");
				printf("%s\n",str2);
				sentence+=w.ws_col-strlen(inits);
				temp_sent+=w.ws_col-strlen(inits);
				times_printed++;
			}
			free(str1);
			free(str2);
		}
		temp = temp->next;
		i++;
		count++;
	}
}


// Df function searches trie recursively and uses a list of letters
// in order to print the word
void df(node *level, charArray* letter_array){
	node *temp = level;
	if(temp)
		insertChar(&letter_array,temp->letter);
	if(temp->child)
		df(temp->child,letter_array);
	if(temp->end_of_word){
		printWord(letter_array);
		int i=0;
		postingList *plist = temp->posting;
		while(plist){
			i++;
			plist = plist->next;
		}
		printf(" %d\n", i);
	}
	removeChar(&letter_array);
	if(temp->next)
		df(temp->next,letter_array);
}


// tf function
void tf(node *root, unsigned int id, char *word){
	node *found = NULL;
	found = searchTrie(root, word);
	if (found){
		printf("found\n");
		postingList *plist = found->posting;
		while(plist->id != id)
			plist=plist->next;
		if(!plist)
			printf("Word does not exist in document with id %d\n",id);
		else
			printf("%d %s %d\n",id,word,found->posting->freq);
	}
	else
		printf("Word not found!\n");
}


void free_postingList(postingList *plist){
	if (plist->next)
		free_postingList(plist->next);

	free(plist);
}

void free_trie(node *root){
	if(root->next)
		free_trie(root->next);
	if(root->child)
		free_trie(root->child);
	if(root->posting)
		free_postingList(root->posting);
	free(root);
}

// minisearch function, takes commands and calls search, df, tf and prints messages
// if wrong argumens are given.
void Csearch(map *Docmap, node *root)
{
	node *found = NULL;
	unsigned int i;
	scoreArray *score_array = NULL;
	struct command *cmd;

	cmd = minisearch_get_cmd();

	while(cmd->cmd_id != EXIT) {
		if(cmd->cmd_id == SEARCH) {
			assert(cmd->nr_args > 0 && cmd->nr_args <= MINISEARCH_MAX_QUERIES);

			// A struct that holds the queries and their posting lists.
			ResArray *result_array = malloc(sizeof(ResArray) * cmd->nr_args);
			for(i = 0; i < cmd->nr_args; i++){
				result_array[i].query = NULL;
				result_array[i].plist = NULL;
			}

			size_t j;
			for (j = 0; j < cmd->nr_args; ++j) {
				found = searchTrie(root, cmd->args[j]);
				if(found)
					push(result_array, j, cmd->args[j], found->posting);
			}

			// A list that holds each document id with its score, sorted by score.
			score_array = score(result_array, Docmap, cmd->nr_args);
			print_results(result_array, cmd->nr_args, score_array, Docmap);
			free(score_array);

			for(i = 0; i < cmd->nr_args; i++) {
				if(result_array[i].query)
					free(result_array[i].query);
			}
			free(result_array);
		} else if(cmd->cmd_id == DF) {
			assert(cmd->nr_args < 2);

			if(cmd->nr_args == 0) {
				printf("DF: no arguments\n");
				df(root->child, NULL);
			} else {
				found = searchTrie(root, cmd->args[0]);
				if(found) {
					i=0;
					postingList *plist = found->posting;
					while(plist){
						i++;
						plist = plist->next;
					}
					printf("%s %d\n", cmd->args[0], i);
				} else {
					printf("DF: Query not found.\n");
				}
			}
		} else if (cmd->cmd_id == TF) {
			assert(cmd->nr_args == 2);

			int id = atoi(cmd->args[0]);
			tf(root, id, cmd->args[1]);
		}

		minisearch_free_cmd(cmd);
		cmd = minisearch_get_cmd();
	}
	minisearch_free_cmd(cmd);
}

const char *argp_program_version = "version 0.1";
const char *argp_program_bug_address = "sdi1500173@di.uoa.gr";

static int parse_opt(int key, char *arg, struct argp_state *state)
{
	struct minisearh_args *a = (struct minisearh_args *) state->input;
	switch (key) {
		case 'k':
			if (k > 10) {
				argp_failure(state, 1, 0, "K can be at maximum 10");
				break;
			}
			a->k = atol(arg);
			break;
		case 'i':
			a->docname = arg;
			break;
		case ARGP_KEY_ARG:
			argp_failure(state, 1, 0, "too many arguments");
			break;
		case ARGP_KEY_END:
			if (!a->docname) {
				argp_error(state, "You need to provide a filename");
			}

			if (!a->k) {
				argp_error(state, "You need to provide a value for k");
			}
			break;
	}

	return 0;
}

static struct argp_option options[] = {
	{ "input-file", 'i', "filename", 0, "Name of the input file", 0 },
	{ "k-factor", 'k', "K", 0, "K (K <= 10)", 0 },
	{ 0 },
};

static struct argp argp = {
	options,
	parse_opt,
	0,
	"A minisearch application",
	NULL,
	NULL,
	NULL
};

void getArgs(int argc, char **argv)
{
	struct minisearh_args *args = malloc(sizeof (struct minisearh_args));
	if (!args) {
		perror("Could not allocate memory of command line arguments");
		exit(1);
	}

	args->docname = NULL;
	args->k = 0;
	argp_parse(&argp, argc, argv, 0, 0, args);
	docname = args->docname;
	k = args->k;
	Map_Size = args->Map_Size;
	numOfWords = args->numOfWords;
}
