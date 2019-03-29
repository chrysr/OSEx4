#ifndef LIST_H
#define LIST_H

#include "functions.h"

typedef class treenode treenode;
class listnode{
private:
    treenode* tnode;
    listnode* next;
    listnode* previous;

public:
    listnode();
    ~listnode();

    /* setters */
    void settreenode(treenode* tnode);
    void setnext(listnode* lnode);
    void setprevious(listnode* lnode);

    /* getters */
    treenode* gettreenode();
    listnode* getnext();
    listnode* getprevious();
};

class list{
private:
    listnode* first;
    int counter;

public:
    list();
    ~list();

    /* setters */
    void initlist();
    void setfirst(listnode* lnode);
    void setcounter(int cnt);


    /* getters */
    listnode* getfirst();
    listnode** getall();
    int getcounter();
    int getdircount();


    /* general functions */
    treenode* find(char* n);                       //return treenode if found
    void insert(treenode* tnode);                 //inserts new element at end of list
    void modify(treenode* dest, treenode* newt); //swap a treenode if found with the new one
    void sort();                                //sorts elements of the list by using bubble sort
    void remove(char * name);                  //deletes an element if found
    void remove(treenode* tnode);             //same as above,but uses treenode* as input
    void printall();                         //prints all elements of list
    void cleanup();                         //used by the inodearrays. removes listnodes that contain NULL at their treenode fields
    void removenodel(char* name);          //removes an element if found without deleting it



};
#endif
