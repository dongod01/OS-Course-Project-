#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

using namespace std;

#define SIZECONV 1024*1024
#define POW_2_24 16777216
#define POW_2_23 8388608

extern char dtype_name[4][6];
extern long curr_table_entries,curr_mem_entries,overall_tb_entries,curr_st_entries,curr_fseg_entries;
extern int MAX_TABLE_SIZE;
extern int MAX_MEM_SIZE;
extern pthread_t thr_id;

//void* tb1;

extern "C" int modulo(int,int);

typedef struct tablenode{
    char name[20];
    int d_type; //0->int,1->medium int,2->char,3->bool
    int arrind = -2;
    long index = -1;
    struct tablenode* prev = NULL;
    struct tablenode* next = NULL;
    tablenode(){
        arrind = -2;
        index = -1 ;
    }
} tnode;

typedef struct stacknode{
    char name[20];
    int func_ctr;
    int arrind = -2;
    long offset;
    struct stacknode* next = NULL,*prev = NULL;
    stacknode(){
        arrind = -2;
        offset = -1;
    }
} stnode;

extern int *mseg;
extern tnode *table;
extern stnode *gstseg,*ustseg,*hole_st_seg;
extern stnode *free_space_seg;

struct Llist2{
    stnode *head = NULL,*current=NULL,*tail;
    
    Llist2(){
        head = current = tail = NULL;
    }
    
    void insertnode(char vname[20],int f_c,int arridx ,long offs);
    //delete first item
    stnode* deletenode(char vname[20]);
    
    stnode* find(char vname[20]);
};

struct stack{
    stnode *head = NULL,*topn;

    void push(char vname[20],int func_ctr,int arrind ,long offset);
    void pop();
    stnode* top();
};

struct Llist{
    tnode *head = NULL,*current=NULL,*tail;
    /*tnode *end;
    bool isempty ;
    Llist(){
        head = tail = current = end = NULL;
        isempty = false;
    }*/
    Llist(){
        head = current = tail = NULL;
    }
    void printList();
    /*void update_endptr(){
        if(overall_tb_entries>=1) {
            end -> next = table+overall_tb_entries;
            (table + overall_tb_entries)->prev = end;
        }
        else if(overall_tb_entries==0){
            end->next = table + 1;
            (table+1)->prev = end;
        }
    }*/
    void insertnode(char vname[20],int d_t,int arridx ,long ind);
    //delete first item
    tnode* deletenode(char vname[20]);
    long length();
    //find a entry with given name
    tnode* find(char vname[20]);
};

/*struct QNode {
    int data;
    QNode* next;
    QNode(int d)
    {
        data = d;
        next = NULL;
    }
};
 
struct Queue {
    QNode *front, *rear;
    Queue()
    {
        front = rear = NULL;
    }
 
    void enQueue(int x)
    {
 
        // Create a new LL node
        QNode* temp = new QNode(x);
 
        if (rear == NULL) {
            front = rear = temp;
            return;
        }

        rear->next = temp;
        rear = temp;
    }

    void deQueue()
    {
        if (front == NULL)
            return;
 
        QNode* temp = front;
        front = front->next;
 
        if (front == NULL)
            rear = NULL;
 
        delete (temp);
    }
};*/

extern Llist Symbol_Table;

extern "C"{
    void compaction();
    void createMem(long data_mb,long max_data);
    void createVar(int d_type,char name[30],int f_ctr);
    void assignVar(int d_type,char name[30],int* intval,char* chval);
    void createArr(int d_type,char name[30],int arrsz,int f_ctr);
    void assignArr(int d_type,char name[30],int locind,int* intval,char* chval);
    void freeElem(char vname[20],int f_ctr);
    void gc_initialize();
    void* gc_run(void*);
}
