#include "memlab.hpp"

char dtype_name[4][6]={"int","m-int","char","bool"};
long curr_table_entries = 0;
long curr_mem_entries=0;
long overall_tb_entries = 0;
long curr_st_entries = 0;
long curr_fseg_entries=0;
pthread_t thr_id;
int MAX_STACK_SIZE = 281481;
int MAX_TABLE_SIZE = 261376;
int MAX_MEM_SIZE = 65536000;

int *mseg;
tnode *table;
stnode *gstseg;
//stnode *ustseg;
stnode *free_space_seg;
Llist Symbol_Table;
stack Global_Stack;
//stack Unused_Stack;
Llist2 Unused_List;


void Llist2::insertnode(char vname[20],int f_c,int arridx ,long offs) {
    //lock table_entries
    /*if(isempty) {
        end = table;
        isempty = true;
    }*/
    printf("Number of unused list elements : %d\n",curr_fseg_entries);
    if(curr_fseg_entries==0){
        head = current = tail = free_space_seg;
    }
    if(curr_table_entries>=(long)(MAX_STACK_SIZE)){
        cout<<"Insertion unsuccessful,bookkeeping error\n";
        return;
    }
    //update_endptr();
    //create a link node, that will be inserted
    stnode *link = (free_space_seg+curr_fseg_entries);
    strcpy(link->name,vname);
    link->func_ctr = f_c;
    link->arrind = arridx;
    link->offset = offs;
    link->next = NULL;

    /*tail++;
    tail->prev = link;
    tail->next = NULL;
    link->next = tail;*/

    
    
    if(curr_fseg_entries==0) {
        head=link;
        tail=link;
    } 
    else if(curr_fseg_entries>0){
        tail->next = link;
        link->prev = tail;
        tail = link;
    }
    printf("UNUSED LIST: Head : %u Tail : %u \n",head,tail);

    //if(curr_table_entries==overall_tb_entries) overall_tb_entries++;
    curr_fseg_entries++;
    
}

//delete first item
stnode* Llist2::deletenode(char vname[20]) {
    if(curr_table_entries==0){
        printf("Symbol Table is empty, no element can be deleted\n");
    }
    stnode *tempLink = find(vname);
    if(tempLink==NULL){
        printf("Node to be deleted not found\n");
        return NULL;
    }
    if(tempLink!=head && tempLink!=tail){
        stnode* previous = tempLink->prev,*nextel = tempLink->next;
        previous->next = nextel;
        nextel->prev= previous;
    }
    else if(tempLink==head){
        head = tempLink->next;
        head->prev = NULL;
    }
    else{
        tail = tempLink->prev;
        tail->next = NULL;
    }
    curr_fseg_entries--;
    return tempLink;
}

//find a entry with given name
stnode* Llist2::find(char vname[20]) {

    //start from the first link
    stnode* current = head;

    //if list is empty
    if(head == NULL) {
        return NULL;
    }

    //navigate through list
    while(strcmp(current->name,vname)!=0) {
        
        //if it is last node
        if(current->next == NULL) {
            return NULL;
        } else {
            //go to next link
            current = current->next;
        }
    }      
        
    return current;
}



void stack::push(char vname[20],int func_ct,int arrind ,long offs){
    printf("Entered push stack. Number of stack elements : %d\n",curr_table_entries);
    if(curr_st_entries==0){
        head = gstseg;
        topn = gstseg;
    }
    if(curr_table_entries>=(long)(MAX_STACK_SIZE)){
        cout<<"Insertion unsuccessful,bookkeeping error\n";
        return;
    }
    //update_endptr();
    //create a link node, that will be inserted
    stnode *link = (gstseg+curr_st_entries);
    strcpy(link->name,vname);
    link->func_ctr = func_ct;
    link->arrind = arrind;
    link->offset = offs;
    link->next = NULL;

    /*tail++;
    tail->prev = link;
    tail->next = NULL;
    link->next = tail;*/

    
    
    if(curr_st_entries==0) {
        head=link;
        topn=link;
    } 
    else if(curr_st_entries>0){
        topn->next = link;
        link->prev = topn;
        topn = link;
    }
    printf("STACK: Head : %u Top element : %u \n",head,topn);

    //if(curr_table_entries==overall_tb_entries) overall_tb_entries++;
    curr_st_entries++;
    
}

void stack::pop(){
    if(curr_st_entries==0){
        printf("Stack is empty, no element can be deleted\n");
    }

    topn = topn->prev;
    topn->next = NULL;
    
    curr_st_entries--;
    return;
}

stnode* stack::top(){
    return topn;
}

void Llist::printList() {
    tnode *ptr = head;
    printf("Complete list ->\n");
        
    //start from the beginning
    while(ptr != NULL) {
        printf("Name : %s,Value: ",ptr->name);
        if(ptr->d_type==0 || ptr->d_type==1){
            printf("%d,",*(mseg+ptr->index));
        }
        else if(ptr->d_type==2){
            printf("%c,",(char)(*(mseg+ptr->index)));
        }
        else{
            *(mseg+ptr->index)? printf("true,") : printf("false,");
        }
        printf(" Type: %s,Index: %d\n",dtype_name[ptr->d_type],ptr->index);
        ptr = ptr->next;
    }


}

/*void Llist::update_endptr(){
    if(overall_tb_entries>=1) {
        end -> next = table+overall_tb_entries;
        (table + overall_tb_entries)->prev = end;
    }
    else if(overall_tb_entries==0){
        end->next = table + 1;
        (table+1)->prev = end;
    }
}*/

void Llist::insertnode(char vname[20],int d_t,int arridx ,long ind) {
    //lock table_entries
    /*if(isempty) {
        end = table;
        isempty = true;
    }*/
    printf("Number of table elements : %d\n",curr_table_entries);
    if(curr_table_entries==0){
        head = current = tail = table;
    }
    if(curr_table_entries>=(long)(MAX_TABLE_SIZE)){
        cout<<"Insertion unsuccessful,bookkeeping error\n";
        return;
    }
    //update_endptr();
    //create a link node, that will be inserted
    tnode *link = (table+curr_table_entries);
    strcpy(link->name,vname);
    link->d_type = d_t;
    link->arrind = arridx;
    link->index = ind;
    link->next = NULL;

    /*tail++;
    tail->prev = link;
    tail->next = NULL;
    link->next = tail;*/

    
    
    if(curr_table_entries==0) {
        head=link;
        tail=link;
    } 
    else if(curr_table_entries>0){
        tail->next = link;
        link->prev = tail;
        tail = link;
    }
    printf("Head : %u Tail : %u \n",head,tail);

    //if(curr_table_entries==overall_tb_entries) overall_tb_entries++;
    curr_table_entries++;
    if(arridx!=-1) curr_mem_entries+=arridx;
    else curr_mem_entries+=1;
}

//delete first item
tnode* Llist::deletenode(char vname[20]) {
    if(curr_table_entries==0){
        printf("Symbol Table is empty, no element can be deleted\n");
    }
    tnode *tempLink = find(vname);
    if(tempLink==NULL){
        printf("Node to be deleted not found\n");
        return NULL;
    }
    if(tempLink!=head && tempLink!=tail){
        tnode* previous = tempLink->prev,*nextel = tempLink->next;
        previous->next = nextel;
        nextel->prev= previous;
    }
    else if(tempLink==head){
        head = tempLink->next;
        head->prev = NULL;
    }
    else{
        tail = tempLink->prev;
        tail->next = NULL;
    }
    curr_table_entries--;
    return tempLink;
}

long Llist::length() {    
    return curr_table_entries;
}

//find a entry with given name
tnode* Llist::find(char vname[20]) {

    //start from the first link
    tnode* current = head;

    //if list is empty
    if(head == NULL) {
        return NULL;
    }

    //navigate through list
    while(strcmp(current->name,vname)!=0) {
        
        //if it is last node
        if(current->next == NULL) {
            return NULL;
        } else {
            //go to next link
            current = current->next;
        }
    }      
        
    return current;
}

extern "C" void compaction(){

}

extern "C" void createMem(long data_mb,long max_data){
    /*
    -mallocing the correct amount of data
    •separating out the book keeping space 
    •Design considerations for efficiency (in terms of memory footprint / speed)
    •Printing message
    */

    //data_mb stores amount of data required in mb, initialized to 500
    if(data_mb>1000) {
        cout<<"Requested memory is too large\n";
        return;
    }

    long data_by = data_mb*SIZECONV;    //Data in bytes 
    max_data *=SIZECONV;                //Connverting to bytes
    mseg = (int*)malloc(data_by);      //Allocating memory segment
    table = (tnode*)malloc(max_data);    //Allocating bookkeeping space for page table
    gstseg = (stnode*)malloc(max_data);    //Allocating bookkeeping space for removed memory stack
    //ustseg = (stnode*)malloc(max_data);
    free_space_seg= (stnode*)malloc(max_data);  //Allocating bookkeeping space for removed memory stack
    cout<<"Memory allocation successful\n";

    gc_initialize();    //Creates thread for garbage
    
}

extern "C" void createVar(int d_type,char name[30],int f_ctr){
    //lock
    //Error detection
    if(d_type<0 || d_type>3){
        printf("d_type is invalid\n");
        return;
    }
    // Check if memory is available
    if(curr_mem_entries==(long)MAX_MEM_SIZE){
        cout<<"Variable creation unsuccessful, out of memory\n";
        return;
    }

    if(curr_mem_entries>=MAX_MEM_SIZE/2){
        compaction();
    }
    //Identify address
    int *temp = (mseg + curr_mem_entries);

    //Add to symbol table
    Symbol_Table.insertnode(name,d_type,-1,curr_mem_entries);
    Global_Stack.push(name,f_ctr,-1,curr_mem_entries);
    printf("Variable %s created successfully\n",name);

    //unlock
    return;
}

extern "C" void assignVar(int d_type,char name[30],int* intval,char* chval){
    //lock
    
    //Find table entry:
    printf("Starting find in assignvar\n");
    tnode* tb_entry = Symbol_Table.find(name);
    printf("Found something in assignvar\n");
    if(tb_entry==NULL){
        printf("Variable not found in memory\n");
        return;
    }

    //Type checking
    if(tb_entry->d_type!=d_type){
        printf("Value doesnt match datatype of existing variable\n");
    }
    //Find physical address
    int *temp = (mseg + tb_entry->index);

    //Change value at physical address
    int intv;
    if(d_type==0){
        intv = *intval;
    }
    else if(d_type==1){
        intv = modulo(*intval,POW_2_24)-POW_2_23;
    }
    else if(d_type==2){
        intv = (int)(*chval);
    }
    else{
        if(*intval>1)
        {
            printf("Assigning integer to boolean\n");
            exit(1);
        }

        intv = modulo(*intval,2);
    }

    *((int*)temp) = intv;
    
    printf("Variable %s assigned new value %d successfully\n",name,*temp);
    
    //unlock
    return;
}

extern "C" void createArr(int d_type,char name[30],int arrsz,int f_ctr){
    //lock
    //Error detection
    if(arrsz<=0 || d_type<0 || d_type>3){
        printf("Parameter error\n");
    }
    // Check if memory is available
    if(curr_mem_entries>=(long)(MAX_MEM_SIZE - arrsz)){
        cout<<"Array creation unsuccessful, out of memory\n";
        return;
    }
    //Identify address
    int *temp = (mseg + curr_mem_entries);
    
    //Add to symbol table
    Symbol_Table.insertnode(name,d_type,arrsz,curr_mem_entries);
    Global_Stack.push(name,f_ctr,arrsz,curr_mem_entries);
    printf("Array %s space allocated successfully\n",name);

    //unlock mem_entries
    return;
}

extern "C" void assignArr(int d_type,char name[30],int locind,int* intval,char* chval){
    //lock
    
    //Find table entry:
    tnode* tb_entry = Symbol_Table.find(name);
    if(tb_entry==NULL){
        printf("Array not found in memory\n");
        exit(1);
        return;
    }

    //Type checking
    if(tb_entry->d_type!=d_type){
        printf("Array insertion datatype doesnt match datatype of existing array\n");
        exit(1);
    }
    //Find physical address
    int *temp = (mseg + tb_entry->index + locind);

    //Change value at physical address
    int intv;
    if(d_type==0){
        intv = *intval;
    }
    else if(d_type==1){
        intv = modulo(*intval,POW_2_24)-POW_2_23;
    }
    else if(d_type==2){
        intv = (int)(*chval);
    }
    else{
        if(*intval>1||*intval<0)
        {
            printf("Assigning integer to boolean\n");
            exit(1);
        }
        intv = modulo(*intval,2);
    }

    *((int*)temp) = intv;
    
    printf("Array %s assigned new value %d at index %d successfully\n",name,*temp,locind);
    
    //unlock
    return;
}

extern "C" void freeElem(char vname[20],int f_ctr){
    //Find table entry:
    tnode* tb_entry = Symbol_Table.find(vname);
    if(tb_entry==NULL){
        printf("Array/variable not found in memory\n");
        return;
    }
    //Clearing Structure Table and Pushing the unused variables 
    while(Global_Stack.top()->func_ctr==f_ctr){
        stnode* topnode = Global_Stack.top();
        //Clearing symbol table
        tnode* remv = Symbol_Table.deletenode(topnode->name);
        printf("Successfully deleted %s from symbol table\n",remv->name);
        Unused_List.insertnode(topnode->name,topnode->func_ctr,topnode->arrind,topnode->offset);
        Global_Stack.pop();
    }
    
    //Clearing Physical Memory
    

    

    
}

extern "C" int modulo(int x,int y){
    return (x%y +y)%(y);
}



extern "C" void gc_initialize(){
    pthread_attr_t cur_attr;
    pthread_attr_init(&cur_attr);

    int thread_no =1;
    int create_status = pthread_create(&thr_id, &cur_attr, gc_run, (void*)&(thread_no));
    if(create_status != 0)
    {
            printf("Error in creating producer thread!");
            exit(EXIT_FAILURE);
    }

}

extern "C" void* gc_run(void* th_ind){
    while(1){
        //Clear unused list till empty
        sleep(1);   //Sleep 1 sec
        while(curr_fseg_entries!=0){
            stnode* elem = Unused_List.tail;

            int *temp;
            if(elem->arrind==-1){
                //Variable
                temp = (mseg + elem->offset);
                *temp=NULL;
                printf("Successfully freed variable %s\n",elem->name);
            }
            else if(elem->arrind>=1){
                int counter = elem->arrind;
                temp = (mseg + elem->offset);
                while(counter--){
                    *temp = NULL;
                    temp++;
                }
                printf("Successfully freed array %s\n",elem->name);
            }

            Unused_List.deletenode(elem->name);
        }
        
        
    }

}
/*
stack1, stack2;
void compaction(){

}

init-> pthread(gcrun)

void gc_run(){
    while(1){
        sleep();    //Period
        while stack2 not empty:
            pop
        first and last
        first->next = last
        compaction: pushes
    }
}
*/


