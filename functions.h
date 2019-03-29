#ifndef FUNC_H
#define FUNC_H
#include <iostream>
#include <cstdlib>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <string.h>
#include <cstdio>
#include <dirent.h>
#include <cfloat>
#include <limits>
#include "list.h"
#include "tree.h"
#include "inode.h"
#include <ctime>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>

#define PERMS 0777	// access permission for the 3 groups of users

typedef class listnode listnode;
typedef class inode inode;
typedef class treenode treenode;
typedef class list list;
typedef class inodearray inodearray;

#define EVENT_SIZE  ( sizeof (struct inotify_event) ) //size of an event
#define EVENT_BUF_LEN     ( 2048 * ( EVENT_SIZE + 16 ) ) //buffer lenght

//Declaration of extern variables
extern treenode* moveout;
extern unsigned int cookie;
extern treenode* srcroot;
extern inodearray* srcarr;
extern treenode* dstroot;
extern inodearray* dstarr;
extern int fd;
extern int counter;

void fail(const char *message);
const char * target_type(struct inotify_event *event);
const char * target_name(struct inotify_event *event);
const char * event_name(struct inotify_event *event);



void synchronize(treenode* src,treenode* dst,inodearray* sarr,inodearray* darr,int wait=1000);//compares trees and creates/removes dirs and files
int backup(treenode* src,treenode* dst,inodearray* sarr,inodearray* darr,treenode* sroot=NULL,treenode* droot=NULL,int times=1);//creates new files and directoreis at destination
treenode* discovery(char* directory,treenode* root=NULL);//starting from "directory" it goes in a DFS manner and discovers new files and directories
inodearray* fillarray(treenode* root,inodearray* array=NULL,int flag=0);//creates an array and fills it up with each of the nodes of the tree of which the root is provided
void del(treenode* tnode);//deletes the actual directory and files that the treenode and its children reference
void clearup(treenode* src, treenode* dst,inodearray* sarr,inodearray* darr); //deletes excess files and directories at destination
char* strreplace(char* dest,char* find,char* replace);//replaces any instances of find with replace at dest
char* pathify(char* sname,char* dname,char* currentpath);//modifies a path given to change from sname to dname
void pathremove(char* path);//removes excess characters at front of path and removes "/" at end
void addtowatch(int fd,treenode* root,inodearray* rootarr,int times=1);//adds directories to inotify watch
void removesub(int fd,treenode* tnode,inodearray* arr);			//removes subdirectories from inotify watch
void handler(treenode* sroot,inodearray* sarr, treenode* droot,inodearray* darr,struct inotify_event *event,int fd);//handles an inotify_event making the necessary changes
void signalhandler(int signum);	//delete all data structures and close inotify watch when Ctrl-C is pressed




#endif
