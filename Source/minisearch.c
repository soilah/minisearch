#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "minisearch.h"
#include "structs.h"


// Used to get arguments from command line
void getArgs(int argc, char* argv[]){
    int i;
    for (i=1; i<argc; i++){
        if(!strcmp(argv[i], "-i")){
            docname = malloc(sizeof(char)*(strlen(argv[i+1])+1));
            strcpy(docname, argv[i+1]);
        }
        else if(!strcmp(argv[i], "-k"))
            k = atoi(argv[i+1]);
    }
    if(k>10){
        printf("K should be less than 10. Progam will exit now.\n");
        exit(1);
    }
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
void print_results(ResArray *result_array, scoreArray *score_array, map *Docmap){
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
        int blank_count;
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
    postingList *temp;
    while((temp = plist) != NULL){
        plist = plist->next;
        free(temp);
    }
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
void Csearch(map *Docmap, node *root){
    char *input=NULL, *token=NULL, *command=NULL;
    node *found = NULL;
    unsigned int i;
    size_t length=0;
    scoreArray *score_array = NULL;
    printf("/");
    getline(&input, &length, stdin);
    // command is always bigger than 1 letter.
    while(strlen(input)<2){
        printf("/");
        getline(&input, &length, stdin);
    }
    queryNum=0;
    for(i=0; i<strlen(input); i++){
        if(isspace(input[i])){
            queryNum++;
            while(isspace(input[i]))
                i++;
        }
    }
    queryNum--;
    // Queries must count less than 10.
    if(queryNum > 10){
        while(queryNum > 10){
            printf("Too many queries\n");
            printf("/");
            getline(&input, &length, stdin);
            while(strlen(input)<2){
                printf("/");
                getline(&input, &length, stdin);
            }
            queryNum=0;
            for(i=0; i<strlen(input); i++){
                if(isspace(input[i]))
                    queryNum++;
            }
            queryNum--;
        }
    }
    token = strtok(input," \n");
    command = malloc(strlen(token)+1);
    strcpy(command,token);
    while(strcmp(command,"exit")){
        // SEARCH
        if(!strcmp(command,"search")){
            // A struct that holds the queries and their posting lists.
            ResArray *result_array = malloc(sizeof(ResArray)*queryNum);
            for(i=0; i<queryNum; i++){
                result_array[i].query = NULL;
                result_array[i].plist = NULL;
            }
            unsigned int j=0;
            while(token){
                token = strtok(NULL,"  \n");
                if(token){
                    found = searchTrie(root,token);
                    if(found){
                        push(result_array, j, token, found->posting);
                    }
                }
                j++;
            }
            // A list that holds each document id with its score, sorted by score.
            score_array = score(result_array,Docmap, queryNum);
            print_results(result_array,score_array, Docmap);
            free(score_array);
            for(i=0; i<queryNum; i++){
                if(result_array[i].query)
                    free(result_array[i].query);
            }
            free(result_array);
        }
        // DF
        else if(!strcmp(command,"df")){
            token = strtok(NULL, " \n");
            // If no arg given
            if(!token){
                printf("no arg\n");
                df(root->child, NULL);
            }
            // If arg given
            else{
                found = searchTrie(root,token);
                if(found){
                    i=0;
                    postingList *plist=found->posting;
                    while(plist){
                        i++;
                        plist = plist->next;
                    }
                    printf("%s %d\n",token, i);
                }
                else
                    printf("Query not found.\n");
            }

        }
        // TF
        else if(!strcmp(command,"tf")){
            token = strtok(NULL, " ");
            int id = atoi(token);
            token = strtok(NULL, " \n");
            tf(root,id,token);
        }
        printf("/");
        getline(&input, &length, stdin);
        while(strlen(input)<2){
            printf("/");
            getline(&input, &length, stdin);
        }
        queryNum=0;
        for(i=0; i<strlen(input); i++){
            if(isspace(input[i])){
                queryNum++;
                while(isspace(input[i]))
                    i++;
            }
        }
        queryNum--;
        if(queryNum > 10){
            while(queryNum > 10){
                printf("Too many queries\n");
                printf("/");
                getline(&input, &length, stdin);
                while(strlen(input)<2){
                    printf("/");
                    getline(&input, &length, stdin);
                }
                queryNum=0;
                for(i=0; i<strlen(input); i++){
                    if(isspace(input[i]))
                        queryNum++;
                }
                queryNum--;
            }
        }
        token = strtok(input, " \n");
        command = malloc(strlen(token)+1);
        strcpy(command,token);
    }
    free(command);
}



