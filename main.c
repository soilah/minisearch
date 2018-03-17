#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "structs.h"
#include "minisearch.h"
#include <math.h>
#include <sys/ioctl.h>
#include <unistd.h>


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
}



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
    return head_score;
}



void print_results(ResArray *result_array, scoreArray *score_array, map *Docmap){
    // GETTING TERMINAL SIZE
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    scoreArray *temp = score_array;
  //  ResArray *temp2 = result_array;
    postingList *plist=NULL;
    unsigned int i=0,j,pos, count=0;
    char *temp_sent=NULL, *sentence=NULL, inits[25];
    while(temp && count<k){
        sprintf(inits,"%d.( %d) [%f] ",i,temp->docId,temp->score);
        j = 0;
        char found=0;
        while(result_array[j].query){
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
            j++;
        }
        temp_sent= malloc(strlen(sentence)+1);
        strcpy(temp_sent,sentence);
        j=0;
        while(result_array[j].query){
            char *sub=NULL;
            sub=strstr(sentence,result_array[j].query);
            while(sub!=NULL){
                while(sub[strlen(result_array[j].query)] != ' ' && strcmp(sub,result_array[j].query) && sub[strlen(result_array[j].query)] != '\t'){
                    sub++;
                    sub = strstr(sub,result_array[j].query);
                    if(!sub)
                        break;
                }
                if(!sub)
                    break;
                while(!(isspace(sentence[strlen(sentence)-strlen(sub)-1]))){
                    sub++;
                    sub = strstr(sub,result_array[j].query);
                    if(!sub)
                        break;
                }
                if(!sub)
                    break;
                pos=strlen(temp_sent)-strlen(sub);
                while(temp_sent[pos] != ' ' && pos < strlen(temp_sent)){
                    temp_sent[pos]='^';
                    pos++;
                }
                sub += strlen(result_array[j].query);
                sub = strstr(sub,result_array[j].query);
            }
            j++;
        }
        pos=0;
        while(pos < strlen(temp_sent)){
            if (temp_sent[pos] != '^')
                temp_sent[pos] = ' ';
            pos++;
        }
        int blank_count;
        unsigned int times_to_print = strlen(sentence)/(w.ws_col-strlen(inits)), times_printed=0;
        if(strlen(sentence) < w.ws_col){
            blank_count=0;
            printf("%s",inits);
            printf("%s\n",sentence);
            while(blank_count++ < strlen(inits))
                printf(" ");
            printf("%s\n",temp_sent);
        }
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
            sentence+=w.ws_col;
            temp_sent+=w.ws_col;
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

void Csearch(map *Docmap, node *root){
    char *input=NULL, *token=NULL, *command=NULL;
    node *found = NULL;
    unsigned int i, queryNum;
    size_t length=0;
    scoreArray *score_array = NULL;
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
    token = strtok(input," \n");
    command = malloc(strlen(token)+1);
    strcpy(command,token);
    while(strcmp(command,"exit")){
        // SEARCH
        if(!strcmp(command,"search")){
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
            score_array = score(result_array,Docmap, queryNum);
            print_results(result_array,score_array, Docmap);
            for(i=0; i<queryNum; i++){
                free(result_array[i].query);
                free(result_array[j].plist);
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
        queryNum=0;
        for(i=0; i<strlen(input); i++){
            if(isspace(input[i]))
            queryNum++;
        }
        queryNum--;
        token = strtok(input, " \n");
        command = malloc(strlen(token)+1);
        strcpy(command,token);
    }
}


int main(int argc, char* argv[]){
    getArgs(argc, argv);
    FILE* doc = fopen(docname, "r");
    char *line=NULL, *token=NULL;
    size_t length;
    Map_Size=0;
    numOfWords=0;
    while(getline(&line, &length, doc) != -1)
        Map_Size++;
    // MAP CREATION AND INITIALIZATION
    map *Docmap = malloc(sizeof(map)*Map_Size);
    int i;
    for(i=0; i<Map_Size; i++){
        Docmap[i].id=0;
        Docmap[i].document=NULL;
    }
    createMap(Docmap,doc);
    // TRIE CREATION
    node *root = create_node('*');
    rewind(doc);
    unsigned int x=1;
    while(getline(&line, &length, doc) != -1){
        token = strtok(line," ");
        unsigned int id = atoi(token);
        if(Map_Size/10*x == id){
            printf("%d%%",x*10);
            if(x != 10)
                printf("--");
            else
                printf("\n");
            x++;
        }
        while(token){
            token = strtok(NULL, "  \n");
            if (token){
                curr_word = malloc(strlen(token)+1);
                strcpy(curr_word,token);
                insertNode(root,token,id);
                numOfWords++;
            }
        }
    }
    Csearch(Docmap,root);
}


