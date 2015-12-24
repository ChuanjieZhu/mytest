
#include <stdio.h>
#include <stdlib.h>

#define DEBUG_PARSER        1
#define	CFG_MAXARGS			16

int parse_line (char *line, char *argv[])
{
	int nargs = 0;

#ifdef DEBUG_PARSER
	printf ("parse_line: \"%s\"\n", line);
#endif

	while (nargs < CFG_MAXARGS) 
    {
		/* skip any white space */
		while ((*line == ' ') || (*line == '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
#ifdef DEBUG_PARSER
		printf ("parse_line: nargs=%d\n", nargs);
#endif
			return (nargs);
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		while (*line && (*line != ' ') && (*line != '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
#ifdef DEBUG_PARSER
		printf ("parse_line: nargs=%d\n", nargs);
#endif
			return (nargs);
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf ("** Too many args (max. %d) **\n", CFG_MAXARGS);

#ifdef DEBUG_PARSER
	printf ("parse_line: nargs=%d\n", nargs);
#endif
	return (nargs);
}

int main()
{
    const char *cmd = "1111 2222 3333 4444 5555";
    char line[256] = {0};
    char *argv[CFG_MAXARGS + 1];	/* NULL terminated	*/
    
    snprintf(line, sizeof(line), "%s", cmd);

    int nargs = parse_line(line, argv);

    printf("nargs = %d \n", nargs);

    int i;
    for (i = 0; i < nargs; i++) {
        printf("argv[%d]: %s \n", i, argv[i]);
    }
        
    return 0;
}


