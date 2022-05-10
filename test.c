#include <string.h> 
#include <unistd.h>

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <assert.h>

typedef struct LinkedList{
    int val;
    int size;
    struct LinkedList *next;
    struct LinkedList *nodes;
    sem_t node_sem;
} LinkedList;


LinkedList *newNode(int val){
    LinkedList* node = (LinkedList*) malloc(sizeof(LinkedList));
    node->val = val;
    node->size = 1;
    node->next = NULL;
    node->nodes = NULL;
    sem_init( &(node->node_sem), 0, 1);
    return node;
}

LinkedList *readLinkedList(){
    LinkedList *head = newNode(-1);
    head->nodes = newNode(-1);
    int n;
    scanf("%d\n", &n);
    LinkedList *ptr = head;
    for(int i = 0 ; i < n ; i++){
        ptr->next = newNode(-1);
        scanf("%d ", &(ptr->next->val));
        ptr->next->nodes = newNode(-1);
        ptr = ptr->next;
        head->size++;
    }
    return head;
}

void printList(LinkedList *ll){
    while(ll != NULL){
        printf("%d ",ll->val);
        ll = ll->next;
    }
    printf("\n");
}

void freeList(LinkedList *ll){
    if (ll == NULL) return;
    freeList(ll->nodes);
    freeList(ll->next);
    free(ll);
}

LinkedList *listFind(LinkedList *head, int target){
    LinkedList *ll = head;
    while(ll->next != NULL){
        if (ll->next->val == target) return ll->next;
        ll = ll->next;
    }
    return NULL;
}

LinkedList *listDelete(LinkedList *head, int target){
    LinkedList *ll = head;
    while(ll->next != NULL){
        if(ll->next->val == target){
            LinkedList *ptr = ll->next->next;
            freeList(ll->next->nodes);
            free(ll->next);
            ll->next = ptr;
            return ll;
        }
        ll = ll->next;
    }
    return NULL;
}

void listInsert(LinkedList *head, int load){
    LinkedList* ll = head;
    while(ll->next != NULL){
        ll = ll->next;
    }
    ll->next = (LinkedList*) malloc(sizeof(LinkedList));
    ll->next->val = load;
    ll->next->next = NULL;
}


void printGlobalState(LinkedList *head, char **thread_map){
/*



        ___           ___           ___         \n
 HEAD  /   \         /   \         /   \        \n -- não precisa de \n's
-----> | 4 | ----->  | 4 | ----->  | 4 | -----> NULL\n
       \___/         \___/         \___/        \n
         |             |             |          \n
         v             v             v          \n
      {s0,s1,s2,    {s0,s1,s2,    {s0,s1,s2,    \n
       i0,i1}        i0,i1}        i0,i1}       \n
*/

char cols[9][6 + head->size*13 + 7];

strcpy(cols[0], "      ");
strcpy(cols[1], " HEAD ");
strcpy(cols[2], "----->");
strcpy(cols[3], "      ");
strcpy(cols[4], "      ");
strcpy(cols[5], "      ");
strcpy(cols[6], "      ");
strcpy(cols[7], "      ");
strcpy(cols[8], "      ");

LinkedList *ptr = head;

while(head != NULL){
    strncat(cols[0], "  ___        ", 14);
    strncat(cols[1], " /   \\       ", 14);

    char temp_int[3];
    char temp[14];
    sprintf(temp_int, "%d", head->val);
    if(strlen(temp_int) == 1)
      sprintf(temp   , " | %d | ----->", head->val);
    else
      sprintf(temp   , " |%d | ----->", head->val);

    strncat(cols[2], temp, 14);

    strncat(cols[3], " \\___/       ", 14);
    strncat(cols[4], "   |         ", 14);
    strncat(cols[5], "   v         ", 14);
    head = head->next;
}
head = ptr;

while(ptr != NULL){
    LinkedList *nodes = ptr->nodes->next;
    int count = 6;
    char str[14] = "{";
    if(nodes == NULL){
        strncat(cols[count], "            ", 15);
        count++;
        ptr = ptr->next;
        continue;
    }
    while(nodes != NULL){
        strncat(str, thread_map[nodes->val], 2000);
        nodes = nodes->next;
        strncat(str, (nodes == NULL)? "}": ",", 2000);
        if (strlen(str) >= 10 || nodes == NULL){
            while(strlen(str) < 13){
                strncat(str, " ", 2);
            }
            strncat(cols[count], str, 15);
            strcpy(str, " ");
            count++;
        }
    }
    ptr = ptr->next;
}


strncat(cols[0], "\n", 2);
strncat(cols[1], "\n", 2);
strncat(cols[2], " NULL\n", 7);
strncat(cols[3], "\n", 2);
strncat(cols[4], "\n", 2);
strncat(cols[5], "\n", 2);
strncat(cols[6], "\n", 2);
strncat(cols[7], "\n", 2);
strncat(cols[8], "\n", 2);

// Printing session
printf("\n\n\n");
for(int i = 0 ; i < 9 ; i++)
    printf("%s",cols[i]);
}

void listFindAnimation(LinkedList *head, int target, int thread_id, char **thread_map){
    LinkedList *ll = head;
    while(ll->next != NULL){
        sem_wait(&(head->node_sem));
        if (ll->next->val == target){
            listDelete(ll->nodes, thread_id);
            sleep(3);
            printGlobalState(head, thread_map);
            sem_post(&(head->node_sem));
            return;
        }
        listDelete(ll->nodes, thread_id);
        listInsert(ll->next->nodes, thread_id);
        sleep(3);
        printGlobalState(head, thread_map);
        ll = ll->next;
        sem_post(&(head->node_sem));
        sleep(0.5);
    }
    return;
}

void listDeleteAnimation(LinkedList *head, int target, int thread_id, char **thread_map){
    sem_wait(&(head->node_sem));
    LinkedList *ll = head;   
    while(ll->next != NULL){
        if(ll->next->val == target){
            LinkedList *ptr = ll->next->next;
            freeList(ll->next->nodes);
            free(ll->next);
            ll->next = ptr;
            head->size--;
            sleep(1.5);
            printGlobalState(head, thread_map);

            listDelete( ll->nodes, thread_id);
            sleep(1.5);
            printGlobalState(head, thread_map);
            sem_post(&(head->node_sem));

            return;
        }
        
        listDelete( ll->nodes, thread_id);
        listInsert( ll->next->nodes, thread_id);
        sleep(1.5);
        printGlobalState(head, thread_map);

        ll = ll->next;
    }
    sem_post(&(head->node_sem));

    return;
}

void listInsertAnimation(LinkedList *head, int load, int thread_id, char **thread_map){
    LinkedList *ll = head;
    while(ll->next != NULL){
        sem_wait(&(head->node_sem));
        listDelete( ll->nodes, thread_id);

        listInsert( ll->next->nodes, thread_id);

        sleep(1.5);
        printGlobalState(head, thread_map);

        sem_post(&(head->node_sem));

        ll = ll->next;
    }
    sem_wait(&(head->node_sem));

    ll->next = newNode(load);
    sleep(1.5);
    printGlobalState(head, thread_map);
    listDelete(ll->nodes, thread_id);
    sleep(1.5);
    printGlobalState(head, thread_map);

    head->size++;
    sem_post(&(head->node_sem));

    return;
}



typedef struct Args{
    LinkedList *ll;
    char **thread_map;
    int val;
    int thread_id;
} Args;


void *f_searcher(void *arg){
    Args *args = (Args*) arg;
    listFindAnimation(args->ll, args->val, args->thread_id, args->thread_map);
}

void *f_inserter(void *arg){
    Args *args = (Args*) arg;
    listInsertAnimation(args->ll, args->val, args->thread_id, args->thread_map);
}

void *f_deleter(void *arg){
    Args *args = (Args*) arg;
    listDeleteAnimation(args->ll, args->val, args->thread_id, args->thread_map);
}

sem_t initial_wait;

int main(){
    LinkedList *ll = readLinkedList();
    int se, in, de;
    scanf("%d %d %d", &se, &in, &de);
    pthread_t threads[se + in + de];
    char **threadMap;
    LinkedList *ptr = ll->nodes;
    for(int i = 0 ; i < se + in + de ; i++){
        ptr->next = newNode(i);
        ptr = ptr->next;
    }
    threadMap = (char**) malloc((se + in + de)* sizeof(char*));
    for(int i = 0 ; i < se + in + de ; i++)
        threadMap[i] = (char*) malloc(3*sizeof(char));
    for(int i = 0 ; i < se + in + de ; i++){
        Args args;
        args.ll = ll; args.thread_id = i; args.thread_map = threadMap; scanf("%d ", &args.val);
        if(i < se){
            sprintf(threadMap[i], "s%d", i);
            pthread_create(&threads[i], NULL, f_searcher, (void*) &args);
        }
        else if (i < se + in){
            continue;
            sleep(5);
            sprintf(threadMap[i], "i%d", i % se);
            pthread_create(&threads[i], NULL, f_inserter, (void*) &args);
        }
        else if (i < se + in + de){
            continue;
            sprintf(threadMap[i], "d%d", i % (se + in));
            pthread_create(&threads[i], NULL, f_deleter, (void*) &args);
        }
    }
    for(int i = 0 ; i < se /*+ in + de*/ ; i++){
        pthread_join(threads[i], NULL);
    }
    freeList(ll);
}