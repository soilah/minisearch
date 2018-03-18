#include "structs.h"
#include "minisearch.h"
#include <ctype.h>

// MAP FUNCTIONS

void insert(map* docMap, unsigned int ident, char* doc){
    docMap[ident].id = ident;
    docMap[ident].document = malloc((strlen(doc)+1));
    strcpy(docMap[ident].document, doc);
}


void createMap(map *Docmap, FILE* doc){
    unsigned int offset = 0, count=0, length=1, id;
    char *token = NULL, *line = NULL, c;
    if (doc == NULL){
        printf("Error mapping documents (File error).\n");
        exit(1);
    }
    rewind(doc);
    c = fgetc(doc);
    while( c != EOF){
        if (c=='\n'){
            line = malloc(sizeof(char)*(length+1));
            fseek(doc, offset, SEEK_SET);
            fgets(line, length+1, doc);
            // Get id
            token = strtok(line, " \t");
            id = atoi(token);
            if (!offset){
                if (id){
                    printf("Error at document file. Will Exit now.\n");
                    exit(1);
                }
            }
            // If ids are not in order
            if (count != id){
                printf("Error at document file. Will Exit now.\n");
                exit(1);
            }
            // Get document
            token = strtok(NULL, "\n");
            // If document starts with spaces or tabs
            while (isspace(*token))
                token++;
            // Insert id and document in map
            insert(Docmap,id,token);
            offset += length;
            length=1;
            count++;
        }
        else
            length++;
        c = fgetc(doc);
    }
}


// TRIE FUNCTIONS


node *create_node(char l){
    node *trieNode = malloc(sizeof(node));
    trieNode->letter = l;
    trieNode->child = NULL;
    trieNode->next = NULL;
    trieNode->end_of_word=0;
    trieNode->posting = NULL;
    return trieNode;
}


node  *createTrie(){
    node *root = create_node('\0');
    return root;
}

// Searches given char at current level (orizontia)

node  *search(node *level, char key){
    node *cur = level;
    node *found = NULL;
    while (cur != NULL){
        if (cur->letter == key){
            found = cur;
            break;
        }
        cur = cur->next;
    }
    return found;
}

// Returns a pointer to trie node if found, NULL if not
node* searchTrie(node *root, char* query){
    node *temp = root;
    temp = search(temp->child,query[0]);
    if(temp && strlen(query)>1)
        return searchTrie(temp, ++query);
    else if(temp && strlen(query)==1 && temp->end_of_word)
        return temp;
    else
        return NULL;
}




void insertNode(node *level, char* word, unsigned int id){
    node *new=NULL, *temp=level, *temp2=NULL;
    // If node to insert is the root node...
    if (level->letter == '\0' && level->child == NULL){
        while(strlen(word)){
            level->child = create_node(word[0]);
            level = level->child;
            if(strlen(word) == 1){
                level->end_of_word = 1;
                level->posting = createList(id);
            }
            word++;
        }
    }
    else{
        temp2 = search(level->child, word[0]);
        if (temp2 && strlen(word) > 1)
            insertNode(temp2,++word,id);
        // If word exists, update its posting list.
        else if(temp2 && strlen(word) == 1){
            if(temp2->posting)
                updateList(temp2->posting,id);
            else{
                temp2->posting = createList(id);
                temp2->end_of_word=1;
            }
        }
        else{
            new = create_node(word[0]);
            if (strlen(word) == 1){
                new->end_of_word = 1;
                new->posting = createList(id);
            }
            if (level->child)
                temp = level->child;
            if (temp->letter < word[0]){
                while(temp->letter < word[0]){
                    if (temp->next){
                        if(temp->next->letter > word[0]){
                            new->next = temp->next;
                            temp->next = new;
                        }
                    }
                    else
                        temp->next = new;
                    temp = temp->next;
                }
            }
            else{
                new->next = level->child;
                level->child = new;
                temp = new;
            }
            while(strlen(++word)){
                temp->child = create_node(word[0]);
                temp = temp->child;
                if(strlen(word)==1){
                    temp->end_of_word = 1;
                    temp->posting = createList(id);
                }
            }
        }
    }
}



// POSTING LIST FUNCTIONS

postingList *createList(unsigned int id){
    postingList *post = malloc(sizeof(postingList));
    post->id = id;
    post->freq = 1;
    post->next = NULL;
    return post;
}

void updateList(postingList *post, unsigned int id){
    postingList *temp = post;
    while(temp){
        if(temp->id == id){
            temp->freq++;
            break;
        }
        // Case id does not exist yet, create it.
        if(!temp->next){
            temp->next = createList(id);
            break;
        }
        temp = temp->next;
    }
}




// RESULT LIST FUNCTIONS

void push(ResArray *array, unsigned int pos, char *q, postingList *pl){
    array[pos].query = malloc(strlen(q)+1);
    strcpy(array[pos].query,q);
    array[pos].plist = pl;
}




// SCORE LIST FUNCTIONS

char exists(scoreArray *array, unsigned int id){
 //   scoreArray *temp = array;
    char exists=0;
    if(!array)
        return 0;
    while(array){
        if(array->docId == id){
            exists=1;
            break;
        }
        array = array->next;
    }
    return exists;
}

void addscore(scoreArray **array, unsigned int id, double score){
    scoreArray *temp = malloc(sizeof(scoreArray)), *head = *array;
    temp->docId = id;
    temp->score = score;
    temp->next = NULL;
    temp->prev = NULL;
    if(!(*array)){
        *array = temp;
    }
    else{
        while(score < (*array)->score){
            if(!(*array)->next)
                break;
            *array = (*array)->next;
        }
        if((*array)->score > score){
            (*array)->next = temp;
            temp->prev = *array;
        }
        else{
            if((*array)->prev){
                temp->prev = (*array)->prev;
                (*array)->prev->next = temp;
                (*array)->prev=temp;
                temp->next=(*array);
            }
            else{
                (*array)->prev = temp;
                temp->next = *array;
                temp->prev = NULL;
            }
            if(!(temp)->prev)
                head = temp;

        }
        *array=head;
    }
}
// Letter list functions

void insertChar(charArray **letter_array, char letter){
    charArray *temp=*letter_array, *new=NULL;
    if(!temp){
        *letter_array = malloc(sizeof(charArray));
        (*letter_array)->letter=letter;
        (*letter_array)->next=NULL;
    }
    else{
        while(temp->next)
            temp=temp->next;
        new = malloc(sizeof(charArray));
        temp->next = new;
        new->letter=letter;
        new->next=NULL;
    }
}

void printWord(charArray *letter_array){
    charArray *temp = letter_array;
    while(temp){
        printf("%c",temp->letter);
        temp = temp->next;
    }
}

void removeChar(charArray **letter_array){
    charArray *temp=*letter_array;
    if(!temp)
        return;
    if(!temp->next){
        free(temp);
        *letter_array = NULL;
    }
    else{
        while(temp->next->next)
            temp = temp->next;
        free(temp->next);
        temp->next = NULL;
    }

}
