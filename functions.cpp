#include "functions.h"

using namespace std;


//Code provided by Mr. A. Delis
void fail(const char *message) {
	perror(message);
	exit(1);
}

const char * target_type(struct inotify_event *event) {
	if( event->len == 0 )
		return "";
	else
		return event->mask & IN_ISDIR ? "directory" : "file";
}

const char * target_name(struct inotify_event *event) {
	return event->len > 0 ? event->name : NULL;
}

const char * event_name(struct inotify_event *event) {
	if (event->mask & IN_ACCESS)
		return "access";
	else if (event->mask & IN_ATTRIB)
		return "attrib";
	else if (event->mask & IN_CLOSE_WRITE)
		return "close write";
	else if (event->mask & IN_CLOSE_NOWRITE)
		return "close nowrite";
	else if (event->mask & IN_CREATE)
		return "create";
	else if (event->mask & IN_DELETE)
		return "delete";
	else if (event->mask & IN_DELETE_SELF)
		return "watch target deleted";
	else if (event->mask & IN_MODIFY)
		return "modify";
	else if (event->mask & IN_MOVE_SELF)
		return "watch target moved";
	else if (event->mask & IN_MOVED_FROM)
		return "moved out";
	else if (event->mask & IN_MOVED_TO)
		return "moved into";
	else if (event->mask & IN_OPEN)
		return "open";
	else
		return "unknown event";
}

//End

void synchronize(treenode* src,treenode* dst,inodearray* sarr,inodearray* darr,int wait)//compares trees and creates/removes dirs and files
{
	if(src->getsubtreecounter()!=counter)
	{
		src->sort();
		dst->sort();
		usleep(wait-1000);				//sleep to make sure that copies have finished
	    clearup(src,dst,sarr,darr);		//deletes excess files and directories at destination
	    backup(src,dst,sarr,darr);		//creates new files and directoreis at destination
		src->sort();
		dst->sort();
		src->printall();
		dst->printall();
		cout<<"---------------------------------------"<<endl;
		counter=src->getsubtreecounter();
	}
}
int backup(treenode* src,treenode* dst,inodearray* sarr,inodearray* darr,treenode* sroot,treenode* droot,int times)//creates new files and directoreis at destination
{
    if(times==1)	//default arguments is times=1. If true, sets the source and destination roots to give as parameters in future calls
    {
        sroot=src;
        droot=dst;
    }
    listnode** ll=NULL;
    ll=src->getlist()->getall();
    int cnt=src->getlist()->getcounter();
    char* p;
	char path[PATH_MAX+1];
	treenode* destination=NULL;
    for(int i=0;i<cnt;i++)		//loop for every treenode that exists inside the src directory
    {
		destination=NULL;
        p=pathify(sroot->getname(),droot->getname(),ll[i]->gettreenode()->getpath());
		strcpy(path,p);
		delete[] p;
        destination=dst->find(path,ll[i]->gettreenode()->getname());		//check if treenode exists in destination
        if(destination==NULL)		//if it does not exist
        {
            if(ll[i]->gettreenode()->isdir())		//if directory
            {
                char tmppath[PATH_MAX+1];
                strcpy(tmppath,path);
                strcat(tmppath,"/");
                strcat(tmppath,ll[i]->gettreenode()->getname());		//create directory
                mkdir(tmppath,PERMS);
            }
            else
            {
                list* sl=NULL;
                sl=sarr->find(ll[i]->gettreenode())->iflink();		//else if not directory check if it is a link
                if(sl==NULL)			//if not a link
                {
                    int ch;
                    ch=fork();
                    if(ch==-1)
                        return -1;

                    else if(ch==0)	//fork
                    {
						char paths[PATH_MAX+1];
						char pathd[PATH_MAX+1];
						char tmp[PATH_MAX+1];

						strcpy(paths,ll[i]->gettreenode()->getpath());		//source path
						strcpy(tmp,ll[i]->gettreenode()->getname());		//source name
						pathremove(tmp);									//pathremove() removes excess characters at front of a path so that we can have a standard path layout
						strcat(paths,tmp);									//create final path of source
						strcpy(pathd,path);
						strcpy(tmp,ll[i]->gettreenode()->getname());		//copy current path (destination path)
						pathremove(tmp);
						strcat(pathd,tmp);									//cat new file name
                        execlp("cp","cp","-p",paths,pathd,NULL);		//child copies from source to destination with -p which keeps attributes
                    }
					wait(NULL);											//parent waits for child to return
                }
                else //if link
                {
                    treenode* tempnode=NULL;
                    listnode** ss=sl->getall();
                    for(int j=0;j<sl->getcounter();j++)	//find if the link exists at destination
                    {
                        char* tmppath=pathify(sroot->getname(),droot->getname(),ss[j]->gettreenode()->getpath());//pathify replaces a part of one path with another part
                        tempnode=droot->find(tmppath,ss[j]->gettreenode()->getname());
                        if(tempnode!=NULL)
                        {
                            delete[] tmppath;
                            break;
                        }
                        delete[] tmppath;
                    }
                    delete[] ss;
                    if(tempnode!=NULL)			//if found to be a link
                    {
                        char paths[PATH_MAX+1];
                        char pathd[PATH_MAX+1];
                        strcpy(paths,tempnode->getpath());
                        strcat(paths,"/");
                        strcat(paths,tempnode->getname());
                        strcpy(pathd,path);
                        strcat(pathd,"/");
                        strcat(pathd,ll[i]->gettreenode()->getname());
                        link(paths,pathd);				//link it to the already existing file
                    }
                    else		//else it is not yet a link at destination, so copy it from scratch
                    {
                        int ch;
                        ch=fork();
                        if(ch==-1)
                            return -1;

                        else if(ch==0)//this code is same as before (in copying the file)
                        {
							char paths[PATH_MAX+1];
							char pathd[PATH_MAX+1];
							char tmp[PATH_MAX+1];

							strcpy(paths,ll[i]->gettreenode()->getpath());
							strcpy(tmp,ll[i]->gettreenode()->getname());
							pathremove(tmp);
							strcat(paths,tmp);
							strcpy(pathd,path);
							strcpy(tmp,ll[i]->gettreenode()->getname());
							pathremove(tmp);
							strcat(pathd,tmp);
                            execlp("cp","cp","-p",paths,pathd,NULL);
                        }
						wait(NULL);

                    }
                }
            }		//at this point the new treenode exists at the appropriate destination directory
            destination=new treenode(ll[i]->gettreenode()->getname(),path); //so create a new treenode
            dst->getlist()->insert(destination);	//insert it to the destination Tree
            struct stat* fileStat;
            fileStat=new struct stat;
            char curpath[PATH_MAX+1];
            getcwd(curpath,sizeof(curpath));
            chdir(path);
            if((stat(ll[i]->gettreenode()->getname(),fileStat))==-1)//stat it
            {
                delete fileStat;
                chdir(curpath);
                cout<<"probl"<<endl;
                return 0;
            }
            chdir(curpath);
            destination->setinode(fileStat);		//set struct stat for current treenode
            delete fileStat;

            darr->insert(destination);				//also insert it to the array at destination
        }
		else	//if exists at destination
		{
			if(destination->isdir()==0)	//if not dir
			{			//if attributes are the same, continue to next treeenode
				if((destination->getinode()->st_mtime==ll[i]->gettreenode()->getinode()->st_mtime)&&\
				  (destination->getinode()->st_size==ll[i]->gettreenode()->getinode()->st_size)&&\
				  ((destination->getinode()->st_mode&S_IFMT)==(ll[i]->gettreenode()->getinode()->st_mode&S_IFMT)))
					continue;
				else//if attributes not the same
				{
					list* sl=NULL;
	                sl=sarr->find(ll[i]->gettreenode())->iflink();//check if link
	                if(sl==NULL)//if not a link, just copy it over
	                {
	                    int ch;
	                    ch=fork();
	                    if(ch==-1)
	                        return -1;

	                    else if(ch==0)
	                    {
							char paths[PATH_MAX+1];
							char pathd[PATH_MAX+1];
							char tmp[PATH_MAX+1];

							strcpy(paths,ll[i]->gettreenode()->getpath());
							strcpy(tmp,ll[i]->gettreenode()->getname());
							pathremove(tmp);
							strcat(paths,tmp);
							strcpy(pathd,path);
							strcpy(tmp,ll[i]->gettreenode()->getname());
							pathremove(tmp);
							strcat(pathd,tmp);
	                        execlp("cp","cp","-p",paths,pathd,NULL);
	                    }
						wait(NULL);

	                }
	                else //if link
	                {
	                    treenode* tempnode=NULL;
	                    listnode** ss=sl->getall();
	                    for(int j=0;j<sl->getcounter();j++)//check if it exists at destination as link
	                    {
	                        char* tmppath=pathify(sroot->getname(),droot->getname(),ss[j]->gettreenode()->getpath());
	                        tempnode=droot->find(tmppath,ss[j]->gettreenode()->getname());
	                        if(tempnode!=NULL)
	                        {
	                            delete[] tmppath;
	                            break;
	                        }
	                        delete[] tmppath;
	                    }
	                    delete[] ss;
	                    if(tempnode!=NULL)//if it exists, link it to an existing same file
	                    {
	                        char paths[PATH_MAX+1];
	                        char pathd[PATH_MAX+1];
	                        strcpy(paths,tempnode->getpath());
	                        strcat(paths,"/");
	                        strcat(paths,tempnode->getname());
	                        strcpy(pathd,path);
	                        strcat(pathd,"/");
	                        strcat(pathd,ll[i]->gettreenode()->getname());
	                        link(paths,pathd);
	                    }
	                    else//otherwise just copy it over
	                    {
	                        int ch;
	                        ch=fork();
	                        if(ch==-1)
	                            return -1;

	                        else if(ch==0)
	                        {
	                            char paths[PATH_MAX+1];
	                            char pathd[PATH_MAX+1];
								char tmp[PATH_MAX+1];

	                            strcpy(paths,ll[i]->gettreenode()->getpath());
								strcpy(tmp,ll[i]->gettreenode()->getname());
								pathremove(tmp);
								strcat(paths,tmp);
	                            strcpy(pathd,path);
								strcpy(tmp,ll[i]->gettreenode()->getname());
								pathremove(tmp);
								strcat(pathd,tmp);
	                            execlp("cp","cp","-p",paths,pathd,NULL);
	                        }
							wait(NULL);
	                    }
	                }
				}
			}
		}
        if(ll[i]->gettreenode()->isdir())//if treenode is a directory run backup recursively so that we can copy sub-directories to destination
            backup(ll[i]->gettreenode(),destination,sarr,darr,sroot,droot,times+1);
		sarr->setbackup(sarr->find(ll[i]->gettreenode()),darr->find(destination));//create link to the inode of destination from source
    }
    if(ll!=NULL)
        delete[] ll;
}
treenode* discovery(char* directory,treenode* root)//starting from "directory" it goes in a DFS manner and discovers new files and directories
{
    if(directory==NULL)//if directory not provided, return null
        return NULL;

    char path[PATH_MAX+1];
    int frst=0;
    DIR *dir;
    struct dirent* direntry;
    struct stat* fileStat;

    fileStat=new struct stat;

    if((lstat(directory,fileStat))==-1)	//stat current treenode
    {
        cout<<"problem with lstat"<<endl;
        delete fileStat;
        return NULL;
    }
    if(root==NULL)//this helps so that we do not have to allocate memory outside of this function
    {
        root=new treenode(directory);
        getcwd(path,sizeof(path));
        frst=1;
    }

    root->setinode(fileStat);//set its struct stat
    list* lst=root->getlist();
    if(root->isdir()==0)  // If it's a file return without recursively looking into it
    {
        delete fileStat;
        return root;
    }
    if((dir=opendir(directory))==NULL)	//open directory for discovery
    {
        delete fileStat;
        return NULL;
    }
    chdir(directory);	//change current directory
    while((direntry=readdir(dir))!=NULL){  //next directory entry in the directory stream pointed to by dir
        if((lstat(direntry->d_name,fileStat))==-1)//if stats incorrectly return
        {
            delete fileStat;
            return NULL;
        }
        if(strcmp(direntry->d_name,".")!=0&&strcmp(direntry->d_name,"..")!=0){ 	//exclude "." and ".." directories
            treenode* tnode;
            tnode=new treenode(direntry->d_name);//create new treenode and insert it to the Tree
            lst->insert(tnode);
        }
    }
    listnode** l=NULL;
    l=lst->getall();
    for(int i=0;i<lst->getcounter();i++)//for each of the newly created treenodes, run a recursive discovery to find all of their files and directories
        discovery(l[i]->gettreenode()->getname(),l[i]->gettreenode());

    if(l!=NULL)
        delete[] l;
    chdir("..");//go back one directory
    closedir(dir);
    delete fileStat;
    if(frst)
        chdir(path);//if we're back at the top level, chdir to the original directory
    return root;	//return the root treenode of the tree
}
inodearray* fillarray(treenode* root,inodearray* array,int flag)//creates an array and fills it up with each of the nodes of the tree of which the root is provided
{
    if(root==NULL)		//if null return
        return NULL;
    if(array==NULL)		//if null create a new one
    {
        array=new inodearray;
        array->insert(root);
    }
	if(flag==1)			//flag helps us use this function later on without changing much
	{
		array->insert(root);
	}
	if(root->getlist()->getcounter()==0)//if nothing in this directory, return null;
		return array;
    listnode** lst=NULL;
    lst=root->getlist()->getall();
    for(int i=0;i<root->getlist()->getcounter();i++)//for each treenode, run fillaray recursively
    {
        fillarray(lst[i]->gettreenode(),array);
        array->insert(lst[i]->gettreenode());//insert each treenode in array
    }
    delete[] lst;
    return array;			//return the array that was created
}
void del(treenode* tnode)//deletes the actual directory and files that the treenode and its children reference
{
    char path[PATH_MAX+1];
    getcwd(path,sizeof(path));	//keep starting path
    chdir(tnode->getpath());	//change directory to treenode's path and then change into the actual tnode director
    chdir(tnode->getname());
    listnode** l=tnode->getlist()->getall();
    int cnt=tnode->getlist()->getcounter();
    for(int i=0;i<cnt;i++)		//for each treenode, if it is a dir, run del recursively, otherwise just unlink it
    {
        if(l[i]->gettreenode()->isdir())
            del(l[i]->gettreenode());
        else unlink(l[i]->gettreenode()->getname());
    }

    delete[] l;
    chdir("..");			//chdir one directory back
    rmdir(tnode->getname());//remove directory
    chdir(path);			//return to the initial path
}
void clearup(treenode* src, treenode* dst,inodearray* sarr,inodearray* darr) //deletes excess files and directories at destination
{
    listnode** d=dst->getlist()->getall();
    int flag=0;
    int cnt=dst->getlist()->getcounter();
    for(int i=0;i<cnt;i++)	//for every treenode in destination tree
    {
        flag=0;
        treenode* t=NULL;
        t=src->getlist()->find(d[i]->gettreenode()->getname());
        if(t==NULL)					//if not found on source tree
        {
            char path[PATH_MAX+1];
            getcwd(path,sizeof(path));

            if(d[i]->gettreenode()->isdir())//delete actual files and folders on disk
                del(d[i]->gettreenode());

            else							//if just a file, then unlink it
            {
                chdir(d[i]->gettreenode()->getpath());
                unlink(d[i]->gettreenode()->getname());
            }

            chdir(path);
            darr->del(d[i]->gettreenode());								//delete it also from destination array
            dst->findandelete(d[i]->gettreenode()->getpath(),d[i]->gettreenode()->getname());		//and destination tree
        }
        else						//if found on source tree
        {
            if(d[i]->gettreenode()->isdir())//if dir run recursively
            	clearup(t,d[i]->gettreenode(),sarr,darr);

			else//if not  a dir check for attributes
			{
				if((t->getinode()->st_mtime==d[i]->gettreenode()->getinode()->st_mtime)&&\
				  (t->getinode()->st_size==d[i]->gettreenode()->getinode()->st_size)&&\
				  ((t->getinode()->st_mode&S_IFMT)==(d[i]->gettreenode()->getinode()->st_mode&S_IFMT)))
					continue;		//if attributes same, continue

				else	//if attributes not the same
				{
					char path[PATH_MAX+1];
		            getcwd(path,sizeof(path));
					chdir(d[i]->gettreenode()->getpath());
	                unlink(d[i]->gettreenode()->getname());//unlink it from destination
					chdir(path);
		            darr->del(d[i]->gettreenode());		//delete from array
		            dst->findandelete(d[i]->gettreenode()->getpath(),d[i]->gettreenode()->getname());	//delete from tree
				}
			}
        }
    }
    delete[] d;
}
char* strreplace(char* dest,char* find,char* replace)//replaces any instances of find with replace at dest
{
    static char buffer[PATH_MAX+1];
    char* p;

    if(!(p=strstr(dest,find)))//if not more matches return
        return dest;
    strncpy(buffer,dest,p-dest);//else copy from where the next match is
    buffer[p-dest]='\0';
    sprintf(buffer+(p-dest),"%s%s",replace,p+strlen(find));
    return buffer;
}
char* pathify(char* sname,char* dname,char* currentpath)//modifies a path given to change from sname to dname
{
    char* tmp=new char[PATH_MAX+1];
    char* tmp2=new char[PATH_MAX+1];
    char path[PATH_MAX+1];
    strcpy(tmp,sname);
    strcpy(tmp2,dname);
    int flag=0;
    int flag2=0;
    if(tmp[0]=='.'||tmp[0]=='/')//remove excess characters
    {
        flag++;
        tmp++;
    }
    if(tmp[0]=='/')
    {
        flag++;
        tmp++;
    }
    if(tmp2[0]=='.'||tmp2[0]=='/')
    {
        flag2++;
        tmp2++;
    }
    if(tmp2[0]=='/')
    {
        flag2++;
        tmp2++;
    }
    int i=0;
    char* newcurrentpath=new char[PATH_MAX];
    strcpy(newcurrentpath,currentpath);

    while(newcurrentpath[i]!='\0')
        i++;

    newcurrentpath[i]='/';		//add "/" at end
    newcurrentpath[i+1]='\0';
    strcpy(path,strreplace(newcurrentpath,tmp,tmp2));//replace one string with the new one

    i=0;
    while(path[i]!='\0')
        i++;

    path[i-1]='\0';
    tmp-=flag;
    tmp2-=flag2;


    delete[] tmp2;
    delete[] tmp;
    delete[] newcurrentpath;

    char* finalpath=new char[PATH_MAX+1];
    strcpy(finalpath,path);
    return finalpath;

}
void pathremove(char* path)//removes excess characters at front of path and removes "/" at end
{
    int i=0;
    char* finalpath;
    finalpath=new char[PATH_MAX+1];

    while(path[i]=='.'||path[i]=='/')//remove excess at front
    {
        i++;
    }
    if(i>0)
    {
        i--;
        if(path[i]=='/')
        ;
        else path[i]='/';
        path+=i;
        strcpy(finalpath,path);
        strcat(finalpath,"\0");
        path-=i;
    }
    else
    {
        strcpy(finalpath,"/");
        strcat(finalpath,path);
        strcat(finalpath,"\0");

    }
    i=0;
    while(finalpath[i]!='\0')
        i++;
    if(finalpath[i-1]=='/')//remove "/" at end
        finalpath[i-1]='\0';

    strcpy(path,finalpath);
    delete[] finalpath;
}
void addtowatch(int fd,treenode* root,inodearray* rootarr,int times)//adds directories to inotify watch
{
    char path[PATH_MAX+1];
    char tmp[PATH_MAX+1];

    int wd;
    if(times==1)//add the root as well
    {
        strcpy(path,root->getpath());
        strcpy(tmp,root->getname());
        pathremove(tmp);
        strcat(path,tmp);
		if(rootarr->getwd(root)==-1)
		{
			wd=inotify_add_watch(fd,path,IN_CREATE|IN_ATTRIB|IN_MODIFY|IN_CLOSE_WRITE|IN_DELETE|IN_DELETE_SELF|IN_MOVED_FROM|IN_MOVED_TO);
	        cout<<"Added to watch "<<path<<endl;
	        rootarr->setwd(root,wd);//set the watch descriptor given by inotify to the array
		}


    }
    listnode** ls=root->getlist()->getall();
    for(int i=0;i<root->getlist()->getcounter();i++)
    {
        if(ls[i]->gettreenode()->isdir())//for every treenode if it is a dir
        {
            strcpy(path,ls[i]->gettreenode()->getpath());
            strcpy(tmp,ls[i]->gettreenode()->getname());
            pathremove(tmp);
            strcat(path,tmp);										//add treenode to inotify watch
			if(rootarr->getwd(ls[i]->gettreenode())==-1)
			{
				wd=inotify_add_watch(fd,path,IN_CREATE|IN_ATTRIB|IN_MODIFY|IN_CLOSE_WRITE|IN_DELETE|IN_DELETE_SELF|IN_MOVED_FROM|IN_MOVED_TO);
	            cout<<"Added to watch "<<path<<endl;
	            rootarr->setwd(ls[i]->gettreenode(),wd);				//update array
			}
            addtowatch(fd,ls[i]->gettreenode(),rootarr,times+1);	//run recursively
        }
    }
    delete[] ls;
}
void removesub(int fd,treenode* tnode,inodearray* arr)			//removes subdirectories from inotify watch
{
    if(tnode->getlist()->getcounter()==0)//if no subdirs return
        return;
    listnode** ls=tnode->getlist()->getall();
    for(int i=0;i<tnode->getlist()->getcounter();i++)//for every treenode
    {
        int wd=arr->getwd(ls[i]->gettreenode());	//look for its watch descriptor
        if(wd>-1)
        {
            inotify_rm_watch(fd,wd);				//remove from watch
            arr->setwd(ls[i]->gettreenode(),-1);	//update watch descriptor at array to -1 (default)
        }
        if(ls[i]->gettreenode()->isdir())			//if it is a dir run recursively
            removesub(fd,ls[i]->gettreenode(),arr);
    }
    delete[] ls;
}
void handler(treenode* sroot,inodearray* sarr, treenode* droot,inodearray* darr,struct inotify_event *event,int fd)//handles an inotify_event making the necessary changes
{
    int syncflag=1000;
    if(strcmp(event_name(event),"unknown event")==0||target_name(event)==NULL)	//if it is an unknow event or target name is null just synchronize the two trees and return
    {
        synchronize(sroot,droot,sarr,darr);
        return;
    }
    treenode* tnode=sarr->gettreenode(event->wd);			//find which directory the event is refering to (from its watch descriptor)
    int wd;
    treenode* target=NULL;
    char path[PATH_MAX+1];
    if(tnode==NULL)	//if watch descriptor was not found return
        return;

    else
    {
        char tmpname[NAME_MAX+1];
        strcpy(tmpname,target_name(event));
        strcat(tmpname,"\0");
								//Handle hidden and temporary files (common formats only)
        if(tmpname[0]=='.')
        	return;

        int i=0;
        while(tmpname[i]!='\0')
            i++;

        if(tmpname[i-1]=='~')
        	return;				//end handling

        strcpy(path,tnode->getpath());
        strcpy(tmpname,tnode->getname());
        pathremove(tmpname);
        strcat(path,tmpname);
        strcpy(tmpname,target_name(event));
        target=sroot->find(path,tmpname);		//check if target_name exists in source tree
												//if it exists, target wil be !=NULL otherwise ==NULL
    }
    if(moveout!=NULL&&strcmp(event_name(event),"moved into")!=0)//if a move out was called and a move into is pending, but was not received
    {
		sarr->del(moveout);			//delete treenode from source array
		delete moveout;				//delete treenode
        moveout=NULL;
        cookie=0;
        syncflag=1000;					//ask for synchronization
    }
    if(strcmp(event_name(event),"create")==0)		//if something was created within the watched folders
    {

		char curpath[PATH_MAX+1];
		char tmp[PATH_MAX+1];
		strcpy(tmp,tnode->getname());
		pathremove(tmp);
		strcpy(path,tnode->getpath());
		strcat(path,tmp);
		strcpy(tmp,target_name(event));
		if(target!=NULL)
			return;
						//if we are notified about something that already exists, return
						//(this handles events where items are copied in bulk and inotify doesn't show creation events because new directories
						//have not yet been added to watch. In our implementation we chose to check new directories manually for containing items.)
        if(strcmp(target_type(event),"directory")==0)		//if it was a directory
        {
            treenode* t=NULL;
            getcwd(curpath,sizeof(curpath));
            strcpy(tmp,tnode->getname());
            pathremove(tmp);
            strcpy(path,tnode->getpath());
            strcat(path,tmp);
            strcpy(tmp,target_name(event));
            chdir(path);
            t=discovery(tmp);								//run a discovery to discover new folders and files within it
            fillarray(t,sarr,1);							//update the array. 1 parameter means include the root without creating a new array
            tnode->getlist()->insert(t);					//add newly created subtree to the existing tree
            addtowatch(fd,t,sarr);							//recursively add to watch newly created directories
            chdir(curpath);
        }
        else					//if it is a file
        {
            char tmpname[NAME_MAX+1];
            strcpy(path,tnode->getpath());
            strcpy(tmpname,tnode->getname());
            pathremove(tmpname);
            strcat(path,tmpname);
            strcpy(tmpname,target_name(event));
            treenode* t=new treenode(tmpname,path);		//just create a new treenode without discovering any further
            tnode->getlist()->insert(t);				//append it to existing tree
            getcwd(path,sizeof(path));
            chdir(t->getpath());
            struct stat* fileStat;
            fileStat=new struct stat;
            if((lstat(t->getname(),fileStat))==-1)		//stat it
            {
                cout<<"problem with lstat"<<endl;
                delete fileStat;
                return;
            }
            t->setinode(fileStat);
            sarr->insert(t);							//insert to array
            delete fileStat;
            chdir(path);
        }
        syncflag=2000;
    }
    else if(strcmp(event_name(event),"attrib")==0)//if an attribute was changed (modification time or size)
    {
        if(target==NULL)			//if it does not exist in tree,return. there is an error
        	return;

        getcwd(path,sizeof(path));
        chdir(target->getpath());
        struct stat* fileStat;
        fileStat=new struct stat;
        if((lstat(target->getname(),fileStat))==-1)		//update the item's struct stat
        {
            cout<<"problem with lstat"<<endl;
            delete fileStat;
            return;
        }
        sarr->updatestat(target->getinode(),fileStat);	//update the item's stat at the array as well
        delete fileStat;
        chdir(path);
        syncflag=1000;					//ask for sync
    }
    else if(strcmp(event_name(event),"modify")==0)		//if a file was opened and modified, mark it as modified
    {
        if(target==NULL)			//if target does not exist return
        	return;
        if(target->isdir()==0)			//if dir
            sarr->setmodified(target,1);	//mark it as modified in array
    }
    else if(strcmp(event_name(event),"close write")==0)		//if the file was closed after writing
    {
        if(target==NULL)   //if target does not exist return
     		return;
        bool mod=0;
        if(target->isdir()==0)		//if it is a dir that was modified, we do not care, so check if it is a file
            mod=sarr->getmodified(target);			//look if it was modified or not
        if(mod)				//if modified
        {
            getcwd(path,sizeof(path));
            chdir(target->getpath());
            struct stat* fileStat;
            fileStat=new struct stat;
            if((lstat(target->getname(),fileStat))==-1)	//re- stat it
            {
                cout<<"problem with lstat"<<endl;
                delete fileStat;
                return;
            }
            sarr->updatestat(target->getinode(),fileStat);	//update array
            delete fileStat;
            chdir(path);
            syncflag=1000;								//ask to sync
            sarr->setmodified(target,0);			//mark as non modified
        }
    }
    else if(strcmp(event_name(event),"delete")==0)		//if a containing item was deleted
    {
        if(target==NULL)    //if target does not exist return
            return;


        if(tnode->isdir())	//if it was a dir (in our implementation all dirs are watched, so dirs should technically only be getting "watch target deleted")
        {
            sarr->del(target);			//delete item from array
            sroot->findandelete(target->getpath(),target->getname());		//delete from tree
        }
        else
        {
            sarr->del(target);						//delete from array
            sroot->findandelete(target->getpath(),target->getname());		//delete from tree
		}
        syncflag=1000;						//ask to sync
    }
    else if(strcmp(event_name(event),"watch target deleted")==0)		//an item that is monitored was deleted
    {
        if(target==NULL)			//if target does not exist return
            return;

        if(tnode->isdir())		//if dir
        {
            removesub(fd,tnode,sarr);				//remove subdirs from watch
            inotify_rm_watch(fd,sarr->getwd(tnode));		//remove self from watch
            sarr->del(tnode);							//delete self from array
            sroot->findandelete(tnode->getpath(),tnode->getname());	//delete treenode from tree
        }
        else		//technically we do not monitor files, so it should never end up here
        {
            inotify_rm_watch(fd,sarr->getwd(tnode));		//remove from watch
            sarr->del(tnode);						//delete from array
            sroot->findandelete(tnode->getpath(),tnode->getname());		//remove from tree
        }
        syncflag=1000;				//ask to sync
    }
    else if(strcmp(event_name(event),"moved out")==0)	//if something was moved out
    {
        if(target==NULL)		//if target does not exist return
            return;

        moveout=target;				//keep it for future reference
        cookie=event->cookie;
        sroot->findandremovenodel(target->getpath(),target->getname());	//remove it from tree without deleting it
		if(target->isdir())
        	removesub(fd,target,sarr);				//remove its subdirs from watch if it is a dir

        inotify_rm_watch(fd,sarr->getwd(target));	//remove self
    }
    else if(strcmp(event_name(event),"moved into")==0)//rewatch this
    {
        if(moveout==NULL)		//if moveout was null, it means that the item was moved in from an unmonitored location, so we treat it as creation
        {
			if(strcmp(target_type(event),"directory")==0)	//if directory
			{
				getcwd(path,sizeof(path));
	            char tmp[PATH_MAX+1];
	            strcpy(path,tnode->getpath());
	            strcpy(tmp,tnode->getname());
	            pathremove(tmp);
	            strcat(path,tmp);
	            chdir(path);
	            strcpy(tmp,target_name(event));
	            treenode* n=discovery(tmp);					//run discovery to find new sub-elements
	            tnode->getlist()->insert(n);				//append to tree
	            fillarray(n,sarr,1);							//fill array with new information
	            chdir(path);
				addtowatch(fd,n,sarr);						//watch new item and subdirectories

			}
			else		//if it is a file
			{
				char tmpname[NAME_MAX+1];
	            strcpy(path,tnode->getpath());
	            strcpy(tmpname,tnode->getname());
	            pathremove(tmpname);
	            strcat(path,tmpname);
	            strcpy(tmpname,target_name(event));
	            treenode* t=new treenode(tmpname,path);		//just create a new treenode without discovering any further
	            tnode->getlist()->insert(t);				//append it to existing tree
	            getcwd(path,sizeof(path));
	            chdir(t->getpath());
	            struct stat* fileStat;
	            fileStat=new struct stat;
	            if((lstat(t->getname(),fileStat))==-1)		//stat it
	            {
	                cout<<"problem with lstat"<<endl;
	                delete fileStat;
	                return;
	            }
	            t->setinode(fileStat);
	            sarr->insert(t);							//insert to array
	            delete fileStat;
	            chdir(path);
			}
        }
        else				//if it is not null, it means there is something pending
        {
            if(strcmp(target_name(event),moveout->getname())!=0||cookie!=event->cookie)		//if it is different than what we have, something went wrong, return
                return;

            tnode->getlist()->insert(moveout);		//insert treenode to new location in tree
            char tmp[PATH_MAX+1];
            strcpy(tmp,tnode->getname());
            pathremove(tmp);
            strcpy(path,tnode->getpath());
            strcat(path,tmp);
            moveout->setpath(path);					//set newpath
            if(moveout->isdir())					//if it is a directory update its sub-elements paths
                moveout->updatesubpaths();
            moveout=NULL;							//reset moveout and cookie
            cookie=0;
        }
		syncflag=2000;
    }
    synchronize(sroot,droot,sarr,darr,syncflag);
}
void signalhandler(int signum)	//delete all data structures and close inotify watch when Ctrl-C is pressed
{
	synchronize(srcroot,dstroot,srcarr,dstarr);
	srcroot->sort();
	dstroot->sort();
    srcroot->printall();	//sort source and destination trees and print them for a final time
    dstroot->printall();
	if(moveout!=NULL)
	{
		srcarr->del(moveout);
		delete moveout;
	}
    if(dstarr!=NULL)
        delete dstarr;
    if(srcarr!=NULL)
        delete srcarr;
    if(srcroot!=NULL)
        delete srcroot;
    if(dstroot!=NULL)
        delete dstroot;
    close(fd);
	cout<<"#######################"<<endl;
	cout<<"#    Program Ended    #"<<endl;
	cout<<"#######################"<<endl<<endl;
    exit(21);
}
