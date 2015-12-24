

#include <string.h>
#include <stdio.h>

#define W_SPACE " \t\r\n"
#define LEN_1024    1024
#define DEFAULT_CONF_FILE_PATH	"test.conf"


static int parse_line(char *buff)
{
	char *token;

	if ((token = strtok(buff, W_SPACE)) == NULL)
	{
            printf("%s %d\r\n", __FUNCTION__, __LINE__);
		(void) 0;
	}
	else if (token[0] == '#')
	{
		(void) 0;
	}
	else if (strstr(token, "mod_"))
	{
		printf("is mod_ line!\r\n");
	}
	else if (strstr(token, "spec_"))
	{
		printf("is spec_ line!\r\n");
	}
	else
	{
		return 0;
	}
}

static void process_input_line(char *config_input_line, int len, const char *file_name)
{
	char *token;

	if ((token = strchr(config_input_line, '\n')))
	{
		*token = '\0';
	}

	if ((token = strchr(config_input_line, '\r')))
	{
		*token = '\0';
	}

	if (config_input_line[0] == '#')
	{
		goto final;
	}
	else if (config_input_line[0] == '\0')
	{
		goto final;
	}

	if (!parse_line(config_input_line))
	{
		printf("parse_line error. \r\n");
	}

final:
	memset(config_input_line, '\0', LEN_1024);
}


void parse_config_file(const char *file_name)
{
	FILE 	*fp;
	char 	config_input_line[LEN_1024] = {0};

	if (!(fp = fopen(file_name, "r")))
	{
		printf("open %s error. %s %d\r\n", file_name, __FUNCTION__, __LINE__);
		return;
	}

        printf("%s %d\r\n", __FUNCTION__, __LINE__);

	while (fgets(config_input_line, LEN_1024, fp))
	{
		process_input_line(config_input_line, LEN_1024, file_name);
	}

	if (fclose(fp) < 0)
	{
		sprintf(stderr, "close %s error. %s %d\r\n", file_name, __FUNCTION__, __LINE__);
	}
}

int main(int argc, char *argv[])
{
	parse_config_file(DEFAULT_CONF_FILE_PATH);

	return 0;
}


