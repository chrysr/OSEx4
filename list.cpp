#include "list.h"

using namespace std;


/******************************* listnode *************************************/

listnode::listnode()//set everything to null
{
    this->tnode=NULL;
    this->next=NULL;
    this->previous=NULL;
}

listnode::~listnode()//free up space if it has been allocated
{
    if(this->gettreenode()!=NULL) /* If listnode has a tnode */
        delete this->tnode;
}


/**************************(listnode) setters *********************************/
void listnode::settreenode(treenode* tnode){ this->tnode=tnode; }

void listnode::setnext(listnode* lnode){ this->next=lnode; }

void listnode::setprevious(listnode* lnode){ this->previous=lnode; }


/**************************(listnode) getters *********************************/
treenode* listnode::gettreenode(){ return this->tnode; }

listnode* listnode::getnext(){ return this->next; }

listnode* listnode::getprevious(){ return this->previous; }



/********************************* list ***************************************/
list::list()//initialize list
{
    this->first=NULL;
    this->counter=0;
}

list::~list(){          //destroy list node by node
    if(counter!=0)
    {
        listnode* lnode=this->first;
        listnode* lnode2;
        while(lnode!=NULL)
        {
            lnode2=lnode;
            lnode=lnode->getnext();
            delete lnode2;
        }
    }
}


/****************************(list) setters ***********************************/
void list::initlist(){ this->first=new listnode;}

void list::setfirst(listnode* lnode){ this->first=lnode; }

void list::setcounter(int cnt){ this->counter=cnt; }


/****************************(list) getters ***********************************/
listnode* list::getfirst(){ return first; }

listnode** list::getall()//it returns all elements in an array like format (very useful)
{
    if(this->getcounter()==0)
        return NULL;
    listnode** l=new listnode*[this->counter];
    listnode* node=this->first;
    int i=0;
    while(node!=NULL)
    {
        l[i]=node;
        i++;
        node=node->getnext();
    }
    return l;
}

int list::getcounter(){ return this->counter; }

int list::getdircount()//return how many treenodes are directories
{
    int cnt=0;
    listnode *node;
    node=this->first;
    while(node!=NULL){
        if(node->gettreenode()!=NULL&&node->gettreenode()->isdir())
            cnt++;
        node=node->getnext();
    }
    return cnt;
}


/************************(list) general functions *****************************/
treenode* list::find(char* n)           //return treenode if found
{
    if(this->counter==0)
        return NULL;
    listnode *node=NULL;
    node=this->first;
    while(node!=NULL){
        if(strcmp(node->gettreenode()->getname(),n)==0)
            return node->gettreenode();

        node=node->getnext();
    }
    return NULL;
}

void list::insert(treenode* tnode){ //inserts new element at end of list
    if(this->counter==0)
    {
        this->first=new listnode;
        this->first->settreenode(tnode);
        this->counter++;
        return;
    }
    listnode *lnode=new listnode;
    lnode->settreenode(tnode);
    listnode *node;
    node=this->first;
    while(node->getnext()!=NULL)
    {
        node=node->getnext();
    }
    node->setnext(lnode);
    lnode->setprevious(node);
    this->counter++;
    return;
}

void list::modify(treenode* dest, treenode* newt)       //swap a treenode if found with the new one
{
    listnode *node;
    node=this->first;
    while(node!=NULL){
        if(dest==node->gettreenode())
        {
            node->settreenode(newt);
            return;
        }
        node=node->getnext();
    }
}

void list::sort()       //sorts elements of the list by using bubble sort
{
    listnode** array=NULL;
    array=this->getall();
    listnode* lnode;
    int i,j,flag;
    for(i=0,flag=1;i<this->getcounter()&&flag==1;i++)//bubble sort
    {
        for(j=0,flag=0;j<this->getcounter()-i-1;j++)
        {
            if(strcmp(array[j]->gettreenode()->getname(),array[j+1]->gettreenode()->getname())>0)
            {
                lnode=array[j];
                array[j]=array[j+1];
                array[j+1]=lnode;
                flag=1;
            }
        }
    }
    for(i=0;i<this->getcounter();i++)//re-connect pointers
    {
        if(i>0)
            array[i]->setprevious(array[i-1]);
        else array[i]->setprevious(NULL);

        if(i+1==this->getcounter())
        {
            array[i]->setnext(NULL);
            break;
        }
        array[i]->setnext(array[i+1]);
    }
    if(this->getcounter()!=0)
        this->first=array[0];
    if(array!=NULL)
        delete[] array;
}

void list::remove(char * name){     //deletes an element if found
    listnode* node;
    node=this->first;
    while(node!=NULL){
        if(strcmp(node->gettreenode()->getname(),name)==0){
            if(node->getprevious()!=NULL)
                node->getprevious()->setnext(node->getnext());
            else this->setfirst(node->getnext());
            if(node->getnext()!=NULL)
                node->getnext()->setprevious(node->getprevious());
            delete node;
            counter--;
            break;
        }
        node=node->getnext();
    }
    if(counter==0)
        this->first=NULL;

}
void list::removenodel(char* name)      //removes an element if found without deleting it
{
    listnode* node;
    node=this->first;
    while(node!=NULL){
        if(strcmp(node->gettreenode()->getname(),name)==0){
            if(node->getprevious()!=NULL)
                node->getprevious()->setnext(node->getnext());
            else this->setfirst(node->getnext());
            if(node->getnext()!=NULL)
                node->getnext()->setprevious(node->getprevious());
            node->settreenode(NULL);
            delete node;
            counter--;
            break;
        }
        node=node->getnext();
    }
    if(counter==0)
        this->first=NULL;
}

void list::remove(treenode* tnode)      //same as above,but uses treenode* as input
{
    listnode* node;
    node=this->first;
    while(node!=NULL){
        if(node->gettreenode()==tnode){
            if(node->getprevious()!=NULL)
                node->getprevious()->setnext(node->getnext());
            else this->setfirst(node->getnext());
            if(node->getnext()!=NULL)
                node->getnext()->setprevious(node->getprevious());
            delete node;
            counter--;
            break;
        }
        node=node->getnext();
    }
    if(counter==0)
        this->first=NULL;
}

void list::printall(){          //prints all elements of list
    listnode *node;
    node=this->first;
    while(node!=NULL){
        if(node->gettreenode()!=NULL)
        {
            if(node->gettreenode()!=NULL)
                cout << node->gettreenode()->getname() << endl;
            node=node->getnext();
        }
        else break;
    }
}

void list::cleanup()                //used by the inodearrays. removes listnodes that contain NULL at their treenode fields
{
    listnode *node=NULL;
    listnode* previous=NULL;
    node=this->first;
    while(node!=NULL){
        if(node->gettreenode()==NULL)
        {
            if(node->getprevious()!=NULL)
                node->getprevious()->setnext(node->getnext());
            else this->first=node->getnext();
            if(node->getnext()!=NULL)
                node->getnext()->setprevious(node->getprevious());
            delete node;
            this->counter--;
            break;
        }
        previous=node;
        node=node->getnext();
    }
    if(this->counter==0)
        this->first=NULL;
}
