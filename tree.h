#ifndef TREE_H
#define TREE_H
#include "functions.h"
typedef class listnode listnode;
typedef class inode inode;
typedef class treenode treenode;
typedef class list list;
class treenode{
private:
    char* name;            /* Name of treenode */
    char* path;            /* Full path */
    struct stat* ptr_stat; /* inode information */
    list* lst;             /* List of listnodes */

public:
    treenode(char* n,char* path=NULL);
    ~treenode();

    /* setters */
    void setinode(struct stat* p_stat); // set inode pointer of treenode
    void setpath(char* p);             //set path

    /* getters */
    char* getname();
    char* getpath();
    struct stat* getinode();
    list* getlist();
    int getsubtreecounter();                               //get count of sub-tree elements


    /* general functions */
    int isdir();                                          // Return 1 if this treenode is a directory
    treenode* find(char* path,char* name,int times=1);   //returns the treenode if found
    void sort();                                        //sorts the elements on every sub-directory
    void printall(int d=1,char* ch=NULL);              // Prints every dir with its subdirs
    void printdirs(int d=1,char* ch=NULL);            //Prints every dir with its subdirs
    void printallpaths(int d=1,char* ch=NULL);       //prints everything along with its paths
    void printinode();                              //prints struct stat
    int findandelete(char* path,char* name);       //if found, it deletes the element
    int findandremovenodel(char* path,char* name);//find and remove from tree without deleting the treenode
    void updatesubpaths();                       //updates the paths of sub-elements

};

#endif
