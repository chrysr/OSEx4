#include "functions.h"

#ifndef INODE_H
#define INODE_H

class inode{

private:
    struct stat* fileStat;
    list* namelist;   //list of names that show here (and number of them)
    inode* backup;
    int wd;
    bool modified;

public:
    inode(treenode* t,inode* b=NULL);
    ~inode();

    /* setters */
    void setbackup(inode* b);
    void setstat(struct stat* fs); //sets stat if it does not exist, otherwise overwrites
    void setwd(int d);
    void setmodified(bool m);    //sets modified bit

    /* getters */
    struct stat* getstat();           //gets struct stat
    treenode* gettreenode(char* n);  //returns treenode if it exists in our list
    list* getlist();
    inode* getbackup();
    int getwd();                  //gets watch descriptor
    treenode* gettreenodeifdir();//returns treenode if directory
    bool getmodified();         //gets modified bit

    /* general functions */
    void insert(treenode* tnode);    //insert treenode to list
    int find(char* n);              //if found return 1
    int find(treenode* tnode);     //if found return 1
    void del(char* name);         //removes without actually deleting
    void del(treenode* tnode);   //same as above
    void printstat();           //prints struct stat
    void printstatmin();       //prints only minimal information
    list* iflink();           //returns a list if it is a link (multiple entries,same struct stat)

};
#endif

#ifndef INODEARRAY_H
#define INODEARRAY_H
class inodearray{
private:
    inode** array;
    int size;
    int maxcapacity;

public:
    inodearray(int initsize=20);
    ~inodearray();

    /* getters */
    struct stat* getinode(char* name);   //return struct stat
    struct stat* getinode(treenode* t); //return struct stat


    /* general functions */
    void insert(treenode* tnode);               //inserts treenode to array
    void del(treenode* tnode);                 //deletes treenode from array. if no other treenode exists for same struct stat, we delete inode as well
                                              //also runs recursively for treenodes that are sub-elements of this treenode
    void printall();                         //printall every entry in the array
    void printallmin();                     //prints minimal info about struct stat for every entry
    void printallminbackup();                                       //prints also backup if !=NULL
    inode* find(treenode* tnode);                                  //find inode of treenode
    void setbackup(inode* src,inode* dst);                        //sets backup inode. used only in source
    void setwd(treenode* tnode,int d);                           //sets watch descriptor
    int getwd(treenode* tnode);                                 //gets watch descriptor
    int getcountwd();                                          //get how many items are watched by inotify
    treenode* gettreenode(int d);                             //return treenode based on watch descriptor
    void updatestat(struct stat* old,struct stat* current);  //updates struct stat
    bool getmodified(treenode* tnode);                      //return 1 if item is modified
    void setmodified(treenode* tnode,bool m);              //sets modified bit


};


#endif
