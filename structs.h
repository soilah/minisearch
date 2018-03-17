#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// MAP

typedef struct map{
    unsigned int id;
    char* document;
    
}map;

void createMap(map*,FILE*);

void insert(map*, unsigned int, char*);

//TRIE

struct trieNode;
struct postingList;

typedef struct trieNode{
    char letter;
    struct trieNode *child;
    struct trieNode *next;
    char end_of_word;
    struct postingList *posting;

}node;


node *create_node(char);

void insertNode(node*, char*, unsigned int id);

node *search(node*, char);

node *searchTrie(node*,char*);


// Posting List

typedef struct postingList{
    unsigned int id;
    unsigned int freq;
    struct postingList *next;

}postingList;


postingList *createList(unsigned int);

void updateList(postingList *, unsigned int);

// Result array . This is used to help print results

typedef struct ResArray{
    char *query;
    postingList *plist;

}ResArray;

void push(ResArray*,unsigned int, char*, postingList*);


// Score array. Used to store score with document


typedef struct scoreArray{
    double score;
    unsigned int docId;
    struct scoreArray *next;
    struct scoreArray *prev;

}scoreArray;

char exists(scoreArray*,unsigned int);

void addscore(scoreArray**,unsigned int, double);

// Array that holds letters in order to print words in df mode

typedef struct charArray{
    char letter;
    struct charArray *next;

}charArray;

void insertChar(charArray**,char);

void printWord(charArray*);

void removeChar(charArray**);
