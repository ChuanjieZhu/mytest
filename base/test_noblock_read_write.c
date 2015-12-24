
/*
 *
 *
 *
 */

int noreadn(int fd, char *buf, int size)
{
	int maxfd;
	fd_set rset;
	int iRet = -1;
	
	maxfd = fd + 1;
	while (1)
	{
		FD_ZERO(&rset);

		iRet = select(maxfd, &rset, NULL, NULL, )
		if ()
	}
}
