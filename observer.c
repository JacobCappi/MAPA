#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
  
#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 1024 /*Assuming length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/

#define NUMTHREAD 1


struct filenames
{
    const char *filename;
    const char *dir;
};

void *EventFireAndForget(void *filesinfo)
{
    struct filenames *fileinfo = filesinfo;
    int fd, wd;

    fd = inotify_init();
    if ( fd < 0 ) 
    {
        perror( "Couldn't initialize inotify");
    }
  
    wd = inotify_add_watch(fd, fileinfo->dir, IN_MODIFY); 

    if (wd == -1) 
    {
        printf("Couldn't add watch for %s\n",fileinfo->filename);
    }
    else 
    {
        printf("Watching: %s\n",fileinfo->filename);
    }

    while(1)
    {
        char buffer[BUF_LEN];
        int length = 0;
    
        length = read( fd, buffer, BUF_LEN );  
        if ( length < 0 ) 
        {
            perror( "Read Error" );
        }  
    
        struct inotify_event *event = ( struct inotify_event * ) &buffer;
        if (event->len) 
        {
            if ((event->mask & IN_MODIFY) && strcmp(event->name, fileinfo->filename) == 0)
            {
                printf( "The file %s was Modified with WD %d\n", event->name, event->wd );
            }
        }
    }
    inotify_rm_watch( fd, wd );
    close( fd );
}
 
int main() 
{
    for (int i = 0; i < NUMTHREAD; i++)
    {
        struct filenames *info = (struct filenames*)malloc(sizeof(struct filenames));
        info->filename = "brightness";
        info->dir = "/sys/class/backlight/amdgpu_bl0/";
        pthread_t brightness_thread;
        pthread_create(&brightness_thread, NULL, EventFireAndForget, info);
    }
    return 0;
}
