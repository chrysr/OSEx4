#include "inode.h"


using namespace std;

/********************************* inode **************************************/
inode::inode(treenode* t,inode* b)
{
    fileStat=new struct stat;
    memcpy(fileStat,t->getinode(),sizeof(struct stat));
    if(b!=NULL)
        backup=b;
    else backup=NULL;
    this->namelist= new list;
    this->namelist->insert(t);
    this->wd=-1;
    this->modified=0;
}

inode::~inode(){
    listnode** lst=this->namelist->getall();
    int cnt=namelist->getcounter();
    for(int i=0;i<cnt;i++)
    {
        this->namelist->modify(lst[i]->gettreenode(),NULL);
        this->namelist->cleanup();

    }
    delete this->namelist;
    delete this->fileStat;
    delete[] lst;
}

/***************************(inode) setters ***********************************/
void inode::setbackup(inode* b){ backup=b; }

void inode::setwd(int d){wd=d;}

void inode::setstat(struct stat* fs){       //sets stat if it does not exist, otherwise overwrites
    memcpy(fileStat,fs,sizeof(struct stat));
    listnode** ls=this->namelist->getall();
    for(int i=0;i<this->namelist->getcounter();i++)
        ls[i]->gettreenode()->setinode(fs);

    delete[] ls;
}

void inode::setmodified(bool m) {this->modified=m;}     //sets modified bit



/***************************(inode) getters ***********************************/
struct stat* inode::getstat(){ return fileStat; }           //gets struct stat

int inode::getwd(){return wd;}                              //gets watch descriptor

bool inode::getmodified(){return this->modified;}        //gets modified bit

treenode* inode::gettreenode(char* n)                       //returns treenode if it exists in our list
{
    listnode** lst=this->namelist->getall();
    for(int i=0;i<this->namelist->getcounter();i++)
    {
        if(strcmp(lst[i]->gettreenode()->getname(),n)==0)
        {
            return lst[i]->gettreenode();
        }
    }
    delete[] lst;
}

list* inode::getlist(){ return this->namelist; }

inode* inode::getbackup(){ return backup; }

treenode* inode::gettreenodeifdir()     //returns treenode if directory
{
    if(this->namelist->getcounter()==1)
    {
        treenode* t=this->namelist->getfirst()->gettreenode();
        if(t->isdir())
            return t;
    }
    return NULL;
}

/************************(inode) general functions ****************************/
void inode::insert(treenode* tnode)
{this->namelist->insert(tnode);} //insert treenode to list

int inode::find(char* n)            //if found return 1
{
    if(this->namelist->find(n))
        return 1;
    return 0;
}
int inode::find(treenode* tnode)    //if found return 1
{
    listnode** l=this->namelist->getall();
    int flag=0;
    for(int i=0;i<this->namelist->getcounter();i++)
    {
        if(l[i]->gettreenode()==tnode)
            flag=1;
    }
    delete[] l;
    if(flag)
        return 1;
    return 0;
}

void inode::del(char* name)//removes without actually deleting
{
    this->namelist->modify(this->namelist->find(name),NULL);
    this->namelist->cleanup();
}
void inode::del(treenode* tnode)        //same as above
{
    this->namelist->modify(tnode,NULL);
    this->namelist->cleanup();
}

void inode::printstat()     //prints struct stat
{
    cout << "***************************" << endl;
	cout<<"st_mode: "<< fileStat->st_mode << endl;
	cout<<"st_ino: "<< fileStat->st_ino << endl;
	cout<<"st_dev: "<< fileStat->st_dev << endl;
	cout<<"st_rdev: "<< fileStat->st_rdev << endl;
	cout<<"st_nlink: "<< fileStat->st_nlink << endl;
	cout<<"st_uid: "<< fileStat->st_uid << endl;
	cout<<"st_gid: "<< fileStat->st_gid << endl;
	cout<<"st_size: "<< fileStat->st_size << endl;
    long int t = fileStat->st_atime;
    cout<<"st_atime: "<< ctime(&t);
    t = fileStat->st_mtime;
    cout<<"st_mtime: "<< ctime(&t);
    t = fileStat->st_ctime;
    cout<<"st_ctime: "<< ctime(&t);
	cout<<"st_blksize: "<< fileStat->st_blksize << endl;
	cout<<"st_blocks: "<< fileStat->st_blocks << endl;
	if((fileStat->st_mode&S_IFMT)==S_IFDIR)
		cout<<"directory"<<endl;
	if((fileStat->st_mode&S_IFMT)==S_IFREG)
		cout<<"regular file"<<endl;
	if((fileStat->st_mode&S_IFMT)==S_IFLNK)
		cout<<"symbolic link"<<endl;
	if((fileStat->st_mode&S_IFMT)==S_IFBLK)
		cout<<"block device"<<endl;
	if((fileStat->st_mode&S_IFMT)==S_IFCHR)
		cout<<"character device"<<endl;
	if((fileStat->st_mode&S_IFMT)==S_IFIFO)
		cout<<"FIFO"<<endl;
    cout << "---------------------------" << endl;
}
void inode::printstatmin()      //prints only minimal information
{
    cout << "***************************" << endl;
	cout<<"st_mode: "<< fileStat->st_mode << endl;
	cout<<"st_ino: "<< fileStat->st_ino << endl;
    if((fileStat->st_mode&S_IFMT)==S_IFDIR)
		cout<<"directory"<<endl;
	if((fileStat->st_mode&S_IFMT)==S_IFREG)
		cout<<"regular file"<<endl;
	if((fileStat->st_mode&S_IFMT)==S_IFLNK)
		cout<<"symbolic link"<<endl;
	if((fileStat->st_mode&S_IFMT)==S_IFBLK)
		cout<<"block device"<<endl;
	if((fileStat->st_mode&S_IFMT)==S_IFCHR)
		cout<<"character device"<<endl;
	if((fileStat->st_mode&S_IFMT)==S_IFIFO)
		cout<<"FIFO"<<endl;
    cout << "---------------------------" << endl;
}
list* inode::iflink()           //returns a list if it is a link (multiple entries,same struct stat)
{
    if(this->namelist->getcounter()>1)
    {
        return this->namelist;
    }
    return NULL;
}

/****************************** inodearray ************************************/
inodearray::inodearray(int initsize)
{
    this->size=0;
    this->maxcapacity=initsize;
    this->array=new inode*[maxcapacity];
}

inodearray::~inodearray()
{
    for(int i=0;i<size;i++)
    {
        delete array[i];
    }
    delete[] this->array;
}


/*************************(inodearray) getters ********************************/
struct stat* inodearray::getinode(char* name)       //return struct stat
{
    for(int i=0;i<size;i++)
    {
        if(this->array[i]->find(name))
            return this->array[i]->getstat();
    }
    return NULL;
}

struct stat* inodearray::getinode(treenode* t)      //return struct stat
{
    for(int i=0;i<size;i++)
    {
        if(this->array[i]->find(t->getname()))
            return this->array[i]->getstat();
    }
    return NULL;
}


/*********************(inodearray) general functions **************************/

void inodearray::insert(treenode* tnode)        //inserts treenode to array
{
    int flag=0;
    for(int i=0;i<size;i++)
    {
        if(tnode->getinode()->st_ino==array[i]->getstat()->st_ino)      //if it exists, it is part of a link, add it to list
        {
            array[i]->insert(tnode);
            flag=1;
        }
    }
    if(flag)
        return;
    if(size+1==maxcapacity)     //check for capacity
    {
        inode** tmp=new inode*[maxcapacity+10];
        for(int i=0;i<maxcapacity;i++)
        {
            tmp[i]=this->array[i];
        }
        delete[]this->array;
        maxcapacity+=10;
        this->array=tmp;
    }
    this->array[size]=new inode(tnode);//create new inode in this existing space
    size++;
}
void inodearray::setwd(treenode* tnode,int d)       //sets watch descriptor
{
    for(int i=0;i<size;i++)
    {
        if(tnode->getinode()->st_ino==array[i]->getstat()->st_ino)
        {
            if(tnode->isdir())
            {
                array[i]->setwd(d);
                break;
            }
        }
    }
}
int inodearray::getwd(treenode* tnode)      //gets watch descriptor
{
    for(int i=0;i<size;i++)
    {
        if(tnode->getinode()->st_ino==array[i]->getstat()->st_ino)
        {
            if(tnode->isdir())
                return array[i]->getwd();
        }
    }
    return -1;
}
int inodearray::getcountwd()            //get how many items are watched by inotify
{
    int counter=0;
    for(int i=0;i<size;i++) 
    {
        if(array[i]->getwd()!=-1)
            counter++;
    }
    return counter;

}
bool inodearray::getmodified(treenode* tnode)       //return 1 if item is modified
{
    for(int i=0;i<size;i++)
    {
        if(tnode->getinode()->st_ino==array[i]->getstat()->st_ino)
            return array[i]->getmodified();
    }
    return 0;
}
void inodearray::setmodified(treenode* tnode,bool m)        //sets modified bit
{
    for(int i=0;i<size;i++)
    {
        if(tnode->getinode()->st_ino==array[i]->getstat()->st_ino)
        {
            array[i]->setmodified(m);
            break;
        }
    }
}
treenode* inodearray::gettreenode(int d) //return treenode based on watch descriptor
{
    for(int i=0;i<size;i++)
    {
        if(array[i]->getwd()==d)
        {
            return array[i]->gettreenodeifdir();
        }
    }
    return NULL;
}
void inodearray::updatestat(struct stat* old,struct stat* current)      //updates struct stat
{
    if(old==NULL||current==NULL)
        return;
    for(int i=0;i<size;i++)
    {
        if(array[i]->getstat()->st_ino==old->st_ino)
        {
            array[i]->setstat(current);
            break;
        }
    }
}
void inodearray::del(treenode* tnode)   //deletes treenode from array. if no other treenode exists for same struct stat, we delete inode as well
{                                         //also runs recursively for treenodes that are sub-elements of this treenode
    if(tnode==NULL)
        return;
    listnode** ll;
    int lsize=0;
    for(int i=0;i<size;i++)
    {
        if(this->array[i]->find(tnode))
        {
            ll=tnode->getlist()->getall();
            lsize=tnode->getlist()->getcounter();
            this->array[i]->del(tnode);
            if(this->array[i]->getlist()->getcounter()==0)
            {
                delete this->array[i];
                this->array[i]=this->array[size-1];
                size--;
            }
            break;
        }
    }
    for(int i=0;i<lsize;i++)
    {
        this->del(ll[i]->gettreenode());
    }
    delete[] ll;

}
void inodearray::printall()     //printall every entry in the array
{
    for(int i=0;i<this->size;i++)
    {
        cout<<"===================="<<endl<<"Names: "<<endl;
        this->array[i]->getlist()->printall();
        this->array[i]->printstat();
    }
}
void inodearray::printallmin()      //prints minimal info about struct stat for every entry
{
    for(int i=0;i<this->size;i++)
    {
        cout<<"===================="<<endl<<"Names: "<<endl;
        this->array[i]->getlist()->printall();
        this->array[i]->printstatmin();
    }
}
void inodearray::printallminbackup()        //prints also backup if !=NULL
{
    for(int i=0;i<this->size;i++)
    {
        cout<<"######################################"<<endl<<"Names: "<<endl;
        this->array[i]->getlist()->printall();
        this->array[i]->printstatmin();
        if(array[i]->getbackup()!=NULL)
        {
            cout<<"--------------------------------------"<<endl<<"Names: "<<endl;
            this->array[i]->getbackup()->getlist()->printall();
            this->array[i]->getbackup()->printstatmin();
            cout<<"######################################"<<endl<<endl;
        }
        else cout<<"NULL"<<endl;
    }
}

inode* inodearray::find(treenode* tnode)    //find inode of treenode
{
    for(int i=0;i<size;i++)
    {
        if(array[i]->getstat()->st_ino==tnode->getinode()->st_ino)
            return array[i];
    }
    return NULL;

}

void inodearray::setbackup(inode* src,inode* dst)   //sets backup inode. used only in source
{
    if(src==NULL||dst==NULL)
        return;
    for(int i=0;i<size;i++)
    {
        if(array[i]->getstat()->st_ino==src->getstat()->st_ino)
        {
            if(array[i]->getbackup()==NULL)
                array[i]->setbackup(dst);
            break;
        }
    }
    return;
}
