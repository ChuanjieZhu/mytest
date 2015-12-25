#include <stdio.h>

#include "pqhttp.h"

int http_event(unsigned int total_bytes, unsigned int already_bytes, unsigned int per_bytes)
{
	printf("Downloading... (total:%u, already:%u, per:%u)\n", total_bytes, already_bytes, per_bytes);
	return 1;
}

int main(int argc, char *argv[])
{
	int i = 1;
	int ret;
	PQ_Http *http = pqhttp_create();

	if ( !http ) {
		printf("create http failed!\n");
		return 1;
	}
	
	while ( argc > i ) {
		if ( (ret = pqhttp_load_url(http, argv[i])) != PQH_OK ) {
			printf("load url failed! errno=%#x\n", ret);
			return 1;	
		}

		ret = pqhttp_download(http, NULL, NULL, 0, 1206850, http_event);
		if ( ret == PQH_OK ) {	
			printf("download finish! ok.\n");	
		} else {
			printf("download error! errno=%#x\n", ret);
		}
		++i;
	}

	pqhttp_free(http);

	return 0;
}

