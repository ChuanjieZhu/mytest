#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>

typedef struct USBEvent{
	int event;
	char path[30];
} USBEvent;

int init_hotplug_sock(void)
{
    struct sockaddr_nl snl;
    const int buffersize = 16 * 1024 * 1024;
    int retval;
    memset(&snl, 0x00, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    snl.nl_groups = 1;
    int hotplug_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (hotplug_sock == -1)
    {
        printf("error getting socket: %s", strerror(errno));
        return -1;
    }
    /* set receive buffersize */
    setsockopt(hotplug_sock, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));
    retval = bind(hotplug_sock, (struct sockaddr *) &snl, sizeof(struct sockaddr_nl));

	if (retval < 0) 
	{
        printf("bind failed: %s", strerror(errno));
        close(hotplug_sock);
        hotplug_sock = -1;
        return -1;
    }
    return hotplug_sock;
}

char dstPathTable[8][30];
int recordDstPath(int no, const char *dstPath)
{
	//lenth of dstPath < 30;
	if(strlen(dstPath) >= 30 || no > 8)
	{
		printf("pathname =%s ,no = %d",dstPath,no);
 		return -1;
	}
	
	strcpy(dstPathTable[no-1], dstPath);
	printf("%s has been recorded", dstPathTable[no-1]);
	return 0;
}

const char *LABEL = "label_tmp";
void trim(char *buf)
{
	int i = strlen(buf) - 1;

	while(i=='/n' || i==' ')
	{
		i--;
		printf("true/n");
	}
	
	buf[i+1]=0;
}
int get_label(const char *dev,char *buf,int size)
{
//dev like /dev/sda1;fill buf with usb label;no more than size bytes
static int noLabel = 0;
int ret,fd;
char comm[200]={0};
sprintf(comm,"vol_id -l %s | sed -n '$p' > %s",dev,LABEL);
system(comm);
printf("get_label/n");
fd = open(LABEL,O_RDONLY);
if(fd == -1)
{
perror("open in get_label");
goto error;
}
ret=read(fd,buf,size-1);
if(ret == -1){
perror("read in get_label");
goto error;
}
close(fd);
buf[ret-1]=0;
printf("get_label:%s,%d",buf,strlen(buf));
if(strlen(buf) <= 1)
{
noLabel ++;
sprintf(buf,"usb%d",noLabel);
}
return 0;
error:
noLabel ++;
sprintf(buf,"usb%d",noLabel);
return -1;
}
int make_dir(const char *devPath,const char *dstPath)
{
mode_t dMode = S_IRWXU|S_IRWXG|S_IRWXO;
int ret;
int no = devPath[strlen(devPath)-1]-'0';
ret = mkdir(dstPath,dMode);
if(ret == -1){
perror("make_dir");
return ret;
}
return recordDstPath(no,dstPath);
}
int rm_dir(const char *path)
{
int no = path[strlen(path)-1]-'0';
printf("rmdir:%s/n",dstPathTable[no-1]);
int ret = rmdir(dstPathTable[no-1]);
if(ret == -1)
{
perror("rm_dir");
}
dstPathTable[no-1][0]=0;
return 0;
}
int getDevPath(const char *buf,char *path,int size)
{
int i;
char tmp[20];
for(i = 1;i <= 8;i++)
{
sprintf(tmp,"/sda%d",i);
 if(strstr(buf,tmp))
 {
 strcpy(path,"/dev");
 strcat(path,tmp);
 printf("getDevPath:%s",path);
 return 0;
 }else{continue;}
}
printf("getDevPath:null");
path[0]=0;
return -1;
}
const int ADD_EVENT=0;
const int REMOVE_EVENT=1;
void check()
{
char devPath[20];
char label[21];
char dstPath[30];
char comm[100];
char tmp[30];
int i,ret;
struct stat sb;
struct dirent *file;
USBEvent event;
memset(&event,0,sizeof(event));
for(i=1;i<9;i++){ 
 sprintf(devPath,"/dev/sda%d",i);
 sprintf(comm,"umount %s",devPath);
 system(comm);
}
DIR *dir = opendir("/usb");
while((file=readdir(dir))!=NULL)
{
 sprintf(tmp,"/usb/%s",file->d_name);
 ret = rmdir(tmp);
 if(ret == 0)
 {
 printf("%s has been deleted!",tmp);
 }
}
closedir(dir);
for(i=1;i<9;i++){ 
sprintf(devPath,"/dev/sda%d",i);
ret = open(devPath,O_RDONLY);
 if(ret != -1){
 close(ret);
 printf("%s found/n",devPath);
 get_label(devPath,label,21);
 printf("label:%s/n",label);
 sprintf(dstPath,"/usb/%s",label);
 make_dir(devPath,dstPath);
 sprintf(comm,"mount -t auto -o utf8,rw %s %s",devPath,dstPath);
 ret = system(comm);
 event.event = ADD_EVENT;
 strcpy(event.path,dstPath);
 reportToIplayer(event,"server.socket");
 printf("ret:%s/n",ret);
 }
}
printf("check exit/n");
}

int reportToIplayer(USBEvent event,const char *servername)
{
    const char *CLIENT_PATH = "/tmp/cli";
    struct sockaddr_un un;
    int ret,fd,size,i;
    unlink(CLIENT_PATH);
    memset(&un,0,sizeof(un));
    un.sun_family = AF_UNIX;
    sprintf(un.sun_path,"%s%5d",CLIENT_PATH,getpid());
    size = offsetof(struct sockaddr_un,sun_path)+strlen(un.sun_path);
    fd = socket(AF_UNIX,SOCK_STREAM,0);
    if(fd == -1){
        perror("socket error");
        return -1;
    }
    unlink(un.sun_path);
    ret = bind(fd,(struct sockaddr *)&un,size);
    if(ret == -1){
        perror("bind error");
        return -1;
    }
    memset(&un,0,sizeof(un));
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path,servername);
    ret = connect(fd,(struct sockaddr *)&un,size);
    if(ret == -1){
        perror("connect error");
        return -1;
    }
    ret = write(fd,&event,sizeof(event));
    if(ret == -1){
        perror("write error");
        return -1;
    }
    ret = close(fd);
    if(ret == -1){
        perror("close error");
        return -1;
    }
}
int reportToIplayer(USBEvent event,const char *servername);
void check();
int getDevPath(const char *buf,char *path,int size);
int rm_dir(const char *path);
int make_dir(const char *devPath,const char *dstPath);
int get_label(const char *dev,char *buf,int size);
int recordDstPath(int no,const char *dstPath);
int init_hotplug_sock(void);
int main(int argc,char *argv[])
{
char buf[1024]={0};
char comm[200]={0};
char dstPath[30]={0};
char devPath[30]={0};
char vol[21]={0};
int hotplug_sock;
int sum,size;
char *str;
USBEvent event;
memset(&event,0,sizeof(event));
check();
hotplug_sock = init_hotplug_sock();
while(1){
sum = 0;
size=recv(hotplug_sock,buf,sizeof(buf),0);
while(sum<size)
{
    str=buf+sum;
    sum+=strlen(str);
    buf[sum]='/n';
}
buf[sum]=0;
if(strstr(buf,"ACTION=add"))
{
printf("ACTION=add/n");
if(getDevPath(buf,devPath,30)==0)
       {
 get_label(devPath,vol,21);
 sprintf(dstPath,"/usb/%s",vol);
 make_dir(devPath,dstPath);
 sprintf(comm,"mount -t auto -o utf8,rw %s %s",devPath,dstPath);
 system(comm);
 event.event = ADD_EVENT;
 strcpy(event.path,dstPath);
 reportToIplayer(event,"server.socket");
 }
}
if(strstr(buf,"ACTION=remove"))
{
printf("ACTION=remove/n");
 if(getDevPath(buf,devPath,30)==0)
 {
 sprintf(comm,"umount %s",devPath);
 system(comm);
 rm_dir(devPath);
 event.event = REMOVE_EVENT;
 reportToIplayer(event,"server.socket");
 }
}
}//while
}//main
