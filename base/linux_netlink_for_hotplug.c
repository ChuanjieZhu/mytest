

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <sys/select.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define UEVENT_BUFFER_SIZE	2048

static int InitHotplugSock()
{
	const int bufferSize = 1024;
	int ret;

	struct sockaddr_nl snl;
	bzero(&snl, sizeof(struct sockaddr_nl));
	snl.nl_family = AF_NETLINK;
	snl.nl_pid = getpid();
	snl.nl_groups = 1;

	int sockfd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (-1 == sockfd)
	{
		perror("socket");
		return -1;
	}

	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize));

	/* sockfd non_block */
	//int flags = fcntl(sockfd, F_GETFL,0);
    //fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
	
	ret = bind(sockfd, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
	if (ret < 0)
	{
		perror("bind");
		close(sockfd);
		return -1;
	}

	return sockfd;
}

/* ÈÈ²å°Î¼ì²âÏß³Ì */
void *HotplugThread(void *pArg)
{
	struct timeval tv;
	int sockFd = 0;
	int recvLen = 0;
	int ret = -1;
	fd_set rfds;

	char buf[UEVENT_BUFFER_SIZE * 2] = {0};
	
	sockFd = InitHotplugSock();

	while (sockFd > 0)
	{
		memset(buf, 0, sizeof(buf));
		FD_ZERO(&rfds);  
		FD_SET(sockFd, &rfds);  
		tv.tv_sec = 0;  
		tv.tv_usec = 100 * 1000;

		ret = select(sockFd + 1, &rfds, NULL, NULL, &tv);
		if (ret < 0)
		{
			perror("select");
			break;
		}

		if(ret > 0 && FD_ISSET(sockFd, &rfds))
		{
            recvLen = recv(sockFd, &buf, sizeof(buf), 0);
			if (recvLen > 0)
			{
				printf("%s\n", buf); 
			} 
		}
		
		usleep(100 * 1000);
	}

	if(sockFd >= 0)
	{
		close(sockFd);
	}
	
	printf("Exit %s %d\r\n", __FUNCTION__, __LINE__);
	 
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	//int hotplug_sock = init_hotplug_sock();

	pthread_t hotplugThreadId = -1;

	pthread_create(&hotplugThreadId, NULL, HotplugThread, NULL);
	
	while (1)
	{
		usleep(100 * 1000);
	}

	return 0;
}

