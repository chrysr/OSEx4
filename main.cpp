#include "functions.h"


using namespace std;

treenode* moveout=NULL;
unsigned int cookie=0;
treenode* srcroot=NULL;
inodearray* srcarr=NULL;
treenode* dstroot=NULL;
inodearray* dstarr=NULL;
int fd;
int counter=0;

int main(int argc,char** argv)
{
    signal(SIGINT, signalhandler);
    if(argc!=3)
    {
        cout<<"**************************************************************"<<endl;
		cout<<"This program syncs 2 directories and their files in real time "<<endl;
		cout<<"**************************************************************"<<endl<<endl;

        cout<<"Usage: "<<argv[0]<<" [SOURCE PATH] [DESTINATION PATH]"<<endl<<endl;
		return -5;
    }
    cout<<"*****************************PATHS*****************************"<<endl;
    cout<<"Source:      "<<argv[1]<<endl;
    cout<<"Destination: "<<argv[2]<<endl;
    cout<<"***************************************************************"<<endl<<endl<<endl;
    sleep(1);

    char path[PATH_MAX+1];
    getcwd(path,sizeof(path));
    char par1[PATH_MAX+1];
    char par2[PATH_MAX+1];
    strcpy(par1,argv[1]);
    strcpy(par2,argv[2]);
    if(strstr(par1,path)!=NULL)//handle full paths
    {
        char c[3]=".";
        strcpy(par1,strreplace(par1,path,c));
    }
    if(strstr(par2,path)!=NULL) //handle full paths
    {
        char c[3]=".";
        strcpy(par2,strreplace(par2,path,c));
    }

    srcroot=discovery(par1);     //creates a tree based on the source directory
    srcarr=fillarray(srcroot);      //fills an array based on the upon tree

    dstroot=discovery(par2);     //same as above but for destination directory
    dstarr=fillarray(dstroot);


    synchronize(srcroot,dstroot,srcarr,dstarr); //syncs the two trees and arrays


    //code below was given by Mr. A. Delis (simpleinotify.c)
    //we only added functions addtowatch(),handler()

    int length,i;
    char buffer[EVENT_BUF_LEN];	//the buffer to use for reading the events
    int read_ptr, read_offset; //management of variable length events
    int wd;					// descriptors returned from inotify subsystem



    fd = inotify_init();  	//creating the INOTIFY instance
    if (fd < 0)
        fail("inotify_init");

    addtowatch(fd,srcroot,srcarr);      //add the root and subdirs to inotify watch
    if(srcarr->getcountwd()==0)
        fail("Nothing to watch!");

    read_offset = 0; //remaining number of bytes from previous read
    while (1) {
        // read next series of events
        length = read(fd, buffer + read_offset, sizeof(buffer) - read_offset);
        if (length < 0)
            fail("read");
        length += read_offset; // if there was an offset, add it to the number of bytes to process
        read_ptr = 0;

        // process each event
        // make sure at least the fixed part of the event in included in the buffer
        while (read_ptr + EVENT_SIZE <= length ) {
            //point event to beginning of fixed part of next inotify_event structure
            struct inotify_event *event = (struct inotify_event *) &buffer[ read_ptr ];

            // if however the dynamic part exceeds the buffer,
            // that means that we cannot fully read all event data and we need to
            // deffer processing until next read completes
            if( read_ptr + EVENT_SIZE + event->len > length )
                break;
            //event is fully received, process

            handler(srcroot,srcarr,dstroot,dstarr,event,fd);    //handles the inotify_event
            //advance read_ptr to the beginning of the next event
            read_ptr += EVENT_SIZE + event->len;
        }
        //check to see if a partial event remains at the end
        if( read_ptr < length ) {
            //copy the remaining bytes from the end of the buffer to the beginning of it
            memcpy(buffer, buffer + read_ptr, length - read_ptr);
            //and signal the next read to begin immediatelly after them
            read_offset = length - read_ptr;
        } else
            read_offset = 0;
    }
    return 0;

}
