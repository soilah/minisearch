#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "structs.h"
#include "minisearch.h"
#include <math.h>
#include <sys/ioctl.h>
#include <unistd.h>



int main(int argc, char* argv[]){
    // Getting arguments given in command line
    getArgs(argc, argv);
    FILE* doc = fopen(docname, "r");
    char *line=NULL, *token=NULL;
    size_t length;
    Map_Size=0;
    numOfWords=0;
    // Counting the number of the lines of the file
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
    node *root = create_node('\0');
    rewind(doc);
    while(getline(&line, &length, doc) != -1){
        token = strtok(line," ");
        unsigned int id = atoi(token);
        while(token){
            token = strtok(NULL, "  \n");
            if (token){
                insertNode(root,token,id);
                numOfWords++;
            }
        }
    }
    // Search, df, tf function
    Csearch(Docmap,root);
    // Freeing structures and closing file
    free_trie(root);
    for(i=0; i<Map_Size; i++)
        free(Docmap[i].document);
    free(Docmap);
    fclose(doc);
}


