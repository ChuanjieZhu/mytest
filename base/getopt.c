/***************************************************************************\
*
*	A simple program test the getopt function
*	time: 2013-10-29 21:30:00
*
****************************************************************************/

#include <stdio.h>

/* for getopt(), opterr, optind, optopt, optarg */
#include <unistd.h>

#include <stdlib.h>

/* for isprint function */
#include <ctype.h>

int main(int argc, char *argv[])
{
	int aflag = 0;
	int bflag = 0;
	char *cvalue = NULL;
	int index;
	int c;

	/* 
	 * If the value of this variable is nonzero, then getopt prints an error message to the standard error 
	 * stream if it encounters an unknown option character or an option with a missing required argument. 
	 * This is the default behavior. If you set this variable to zero, getopt does not print any messages, 
	 * but it still returns the character ? to indicate an error. 
	 */
	opterr = 0;		

	while ((c = getopt(argc, argv, "abc:")) != -1)
	{
		switch (c)
		{
			case 'a':
				aflag = 1;
				break;
				
			case 'b':
				bflag = 1;
				break;

			case 'c':
				/*
				 * This variable is set by getopt to point at the value of the option argument, 
				 * for those options that accept arguments. 
				 */
				cvalue = optarg;		
				break;

			case '?':
				/*
				 * When getopt encounters an unknown option character or an option with a missing required 
				 * argument, it stores that option character in this variable. You can use this for providing 
				 * your own diagnostic messages. 
				 */
				if (optopt == 'c')
				{
					fprintf(stderr, "Option -%c requireds an argument.\n", optopt);
				}
				else if (isprint(optopt))
				{
					fprintf (stderr, "Unknown option '-%c'.\n", optopt);
				}
				else
				{
					fprintf (stderr, "Unknown option character '\\x%x'.\n", optopt);
				}
				return 1;

			default:
				abort();
		}
	}

	printf ("aflag = %d, bflag = %d, cvalue = %s\n", aflag, bflag, cvalue);
	
	/*
	 * optind:
	 * This variable is set by getopt to the index of the next element of the argv array to be processed. 
	 * Once getopt has found all of the option arguments, you can use this variable to determine where the 
	 * remaining non-option arguments begin. The initial value of this variable is 1. 
	 */
	for (index = optind; index < argc; index++)
	{
         printf ("Non-option argument %s\n", argv[index]);
	}

	return 0;
}


