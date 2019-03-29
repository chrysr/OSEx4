#include "tree.h"

using namespace std;

treenode::treenode(char* n,char* p)//Treenode Constructor
{
    this->name=new char[NAME_MAX + 1];
    this->path=new char[PATH_MAX+1];
    char srcpath[PATH_MAX+1];
                                            //Allocate space for fields
    if(p==NULL)
    {
        getcwd(srcpath,sizeof(srcpath));  // Determines the path name of the working directory and stores it in srcpath
        strcpy(this->path,srcpath);
    }
    else
        strcpy(this->path,p);

    strcpy(this->name,n);
    this->lst=new list;                     //creates new list
    this->ptr_stat=NULL;                    //set struct stat to null
}
treenode::~treenode()                       //treenode destructor
{                                           //free up all dynamically allocated memory
    if(this->name!=NULL)
        delete[] this->name;
    if(this->lst!=NULL)
        delete this->lst;
    if(this->ptr_stat!=NULL)
        delete this->ptr_stat;
    if(this->path!=NULL)
        delete[] this->path;
}

/****************************** setters ***************************************/

void treenode::setinode(struct stat* p_stat) // set inode pointer of treenode
{
    if(this->ptr_stat!=NULL)
        delete this->ptr_stat;

    if(p_stat!=NULL)
    {
        this->ptr_stat=new struct stat;
        memcpy(this->ptr_stat,p_stat,sizeof(struct stat));
    }
    else
        this->ptr_stat=NULL;
}

void treenode::setpath(char* p){    strcpy(this->path,p);}     //sets path

void treenode::updatesubpaths()     //updates paths of elements inside this treenode's list (recursive)
{
    if(this->getlist()->getcounter()==0)
        return;

    listnode** ll=this->getlist()->getall();
    char tmp[PATH_MAX+1];
    char path[PATH_MAX+1];
    strcpy(path,this->getpath());
    strcpy(tmp,this->getname());
    pathremove(tmp);
    strcat(path,tmp);
    for(int i=0;i<this->getlist()->getcounter();i++)
    {
        ll[i]->gettreenode()->setpath(path);
        if(ll[i]->gettreenode()->isdir())
            ll[i]->gettreenode()->updatesubpaths();
    }
    delete[] ll;
}


/****************************** getters ***************************************/

char* treenode::getname(){ return this->name; }

char* treenode::getpath(){ return this->path; }

struct stat* treenode::getinode(){ return this->ptr_stat; }

list* treenode::getlist(){ return this->lst; }

int treenode::getsubtreecounter()       //recursive
{
    listnode** l=this->getlist()->getall();
    int counter=this->getlist()->getcounter();
    for(int i=0;i<this->getlist()->getcounter();i++)
    {
        if(l[i]->gettreenode()->isdir()==0)
            continue;
        counter+=l[i]->gettreenode()->getsubtreecounter();
    }
    delete[] l;
    return counter;
}


/************************ general functions ***********************************/


int treenode::isdir() // Return 1 if this treenode is a directory
{
    if(S_ISDIR(this->getinode()->st_mode))
        return 1;
    return 0;
}


treenode* treenode::find(char* path,char* name,int times){// Returns listnode if (char*) exists in tree, otherwise null     (recursive)
    if(path==NULL||name==NULL)
        return NULL;

    if(times==1)
    {
        if(strcmp(this->getname(),name)==0&&strcmp(this->getpath(),path)==0)
            return this;
    }
    if(this->getlist()->getcounter()==0)
        return NULL;
    listnode** l=this->getlist()->getall();
    for(int i=0;i<this->getlist()->getcounter();i++)    //recursively check sub-elements and compare paths and name
    {
        if(strcmp(l[i]->gettreenode()->getname(),name)==0&&strcmp(l[i]->gettreenode()->getpath(),path)==0)
        {
            treenode* tt=l[i]->gettreenode();
            delete[] l;
            return tt;
        }
        if(l[i]->gettreenode()->isdir()==0)
            continue;

        treenode* tnode=l[i]->gettreenode()->find(path,name,times+1);  //if a dir, run recursively
        if(tnode!=NULL)
        {
            delete[] l;
            return tnode;
        }
    }
    delete[] l;
    return NULL;
}
int treenode::findandremovenodel(char* path,char* name) //finds the node if it exists and remove it without actually deleting it. Used for "moved out" case
{
    if(path==NULL||name==NULL)                               //same as find (above) but if found, it removes the element from the list that contains it
        return 0;
    if(this->getlist()->getcounter()==0)
        return 0;

    listnode** l=this->getlist()->getall();
    for(int i=0;i<this->getlist()->getcounter();i++)
    {
        if(strcmp(l[i]->gettreenode()->getname(),name)==0&&strcmp(l[i]->gettreenode()->getpath(),path)==0)
        {
            treenode* tt=l[i]->gettreenode();
            delete[] l;
            this->getlist()->removenodel(name);             //removes the element without deleting it
            return 1;
        }
        if(l[i]->gettreenode()->isdir()==0)
            continue;
        int t=l[i]->gettreenode()->findandremovenodel(path,name);       //recrusively
        if(t)
        {
            delete[] l;
            return 1;
        }
    }
    delete[] l;
    return 0;
}
int treenode::findandelete(char* path,char* name)//finds element and deletes it from the list (same as above)
{
    if(path==NULL||name==NULL)
        return 0;
    if(this->getlist()->getcounter()==0)
        return 0;

    listnode** l=this->getlist()->getall();
    for(int i=0;i<this->getlist()->getcounter();i++)
    {
        if(strcmp(l[i]->gettreenode()->getname(),name)==0&&strcmp(l[i]->gettreenode()->getpath(),path)==0)
        {
            treenode* tt=l[i]->gettreenode();
            delete[] l;
            this->getlist()->remove(name);
            return 1;
        }
        if(l[i]->gettreenode()->isdir()==0)
        {    continue;}
        int t=l[i]->gettreenode()->findandelete(path,name);
        if(t)
        {
            delete[] l;
            return 1;
        }
    }
    delete[] l;
    return 0;
}
void treenode::sort()
{
    listnode** l=this->getlist()->getall();
    this->getlist()->sort();
    for(int i=0;i<this->getlist()->getcounter();i++)
    {
        if(l[i]->gettreenode()->isdir()==0)
            continue;
        l[i]->gettreenode()->sort();
    }
    delete[] l;
    return;
}
void treenode::printall(int d,char* ch){ // Prints every dir with its subdirs (with formating)
    if(d==1)
        cout<<endl;
    int flag=0;
    if(ch==NULL)
    {
        ch=new char[PATH_MAX+1];
        strcpy(ch,"\0");
        strcpy(ch,"  ");
        flag=1;
    }
    char nn[PATH_MAX+1]="\0";
    listnode** l=NULL;
    l=this->getlist()->getall();
    if(d==1)
        cout<<this->getname()<<endl;
    for(int i=0;i<this->getlist()->getcounter();i++)
    {
        cout<<ch;
        cout<<"|_"<<l[i]->gettreenode()->getname()<<endl;
        if(i+1==this->getlist()->getcounter())
        {
            strcpy(nn,ch);
            strcat(nn,"  ");
        }
        else
        {
            strcpy(nn,ch);
            strcat(nn,"|  ");
        }
        l[i]->gettreenode()->printall(d+1,nn);
    }
    if(l!=NULL)
        delete[] l;
    if(flag)
        delete[] ch;
    if(d==1)
        cout<<endl;
    return;
}
void treenode::printdirs(int d,char* ch){  // Prints only dirs, otherwise same as above
    int flag=0;
    if(ch==NULL)
    {
        ch=new char[PATH_MAX+1];
        strcpy(ch,"\0");
        strcpy(ch,"  ");
        flag=1;
    }
    char nn[PATH_MAX+1]="\0";
    listnode** l=this->getlist()->getall();
    if(d==1)
        cout<<this->getname()<<endl;
    int dirs=this->getlist()->getdircount();
    for(int i=0;i<this->getlist()->getcounter();i++)
    {
        if(l[i]->gettreenode()->isdir()==0)
            continue;
        cout<<ch;
        cout<<"|_"<<l[i]->gettreenode()->getname()<<endl;
        if(dirs==1)
        {
            strcpy(nn,ch);
            strcat(nn,"  ");
        }
        else
        {
            strcpy(nn,ch);
            strcat(nn,"|  ");
        }
        l[i]->gettreenode()->printdirs(d+1,nn);
        dirs--;
    }
    delete[] l;
    if(flag)
        delete[] ch;
    return;
}
void treenode::printallpaths(int d,char* ch){  //prints everything as above along with their paths
    int flag=0;
    if(ch==NULL)
    {
        ch=new char[PATH_MAX+1];
        strcpy(ch,"\0");
        strcpy(ch,"  ");
        flag=1;
    }
    char nn[PATH_MAX+1]="\0";
    listnode** l=this->getlist()->getall();
    if(d==1)
        cout<<this->getname()<<endl;
    for(int i=0;i<this->getlist()->getcounter();i++)
    {
        cout<<ch;
        cout<<"|_"<<l[i]->gettreenode()->getname()<<"  "<<l[i]->gettreenode()->getpath()<<endl;
        if(i+1==this->getlist()->getcounter())
        {
            strcpy(nn,ch);
            strcat(nn,"  ");
        }
        else
        {
            strcpy(nn,ch);
            strcat(nn,"|  ");
        }
        l[i]->gettreenode()->printallpaths(d+1,nn);
    }
    delete[] l;
    if(flag)
        delete[] ch;
    return;
}
void treenode::printinode()         //prints the struct stat of a treenode
{
    struct stat fileStat;
    fileStat=*ptr_stat;
    cout << "---------------------------" << endl;
    cout << "Information for " << this->getname() << endl << "***************************" << endl;
	cout<<"st_mode: "<< fileStat.st_mode << endl << "st_ino: " << fileStat.st_ino << endl;
	cout<<"st_dev: " << fileStat.st_dev  << endl << "st_rdev: "<< fileStat.st_rdev<< endl;
	cout<<"st_nlink: "<< fileStat.st_nlink << endl <<"st_uid: "<< fileStat.st_uid << endl;
	cout<<"st_gid: "<< fileStat.st_gid << endl<<"st_size: "<< fileStat.st_size << endl;
    long int t = fileStat.st_atime;
    cout<<"st_atime: "<< ctime(&t);
    t = fileStat.st_mtime;
    cout<<"st_mtime: "<< ctime(&t);
    t = fileStat.st_ctime;
    cout<<"st_ctime: "<< ctime(&t);
	cout<<"st_blksize: "<< fileStat.st_blksize << endl <<"st_blocks: "<< fileStat.st_blocks << endl;
	if((fileStat.st_mode&S_IFMT)==S_IFDIR) cout<<"directory"<<endl;
	if((fileStat.st_mode&S_IFMT)==S_IFREG) cout<<"regular file"<<endl;
	if((fileStat.st_mode&S_IFMT)==S_IFLNK) cout<<"symbolic link"<<endl;
	if((fileStat.st_mode&S_IFMT)==S_IFBLK) cout<<"block device"<<endl;
	if((fileStat.st_mode&S_IFMT)==S_IFCHR) cout<<"character device"<<endl;
	if((fileStat.st_mode&S_IFMT)==S_IFIFO) cout<<"FIFO"<<endl;
    cout << "---------------------------" << endl;
}
