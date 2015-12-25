
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "clparse.h"

time_t cur_time;

const char *path_dhclient_db = _PATH_DHCLIENT_DB;

void parse_string_list (struct parse *cfile, struct string_list **lp, int multiple);

static int get_char(struct parse *cfile)
{
	/* My kingdom for WITH... */
	int c;

    /* 第一次bufix = 0, buflen = 0, 读取全部文件内容 */
	if (cfile->bufix == cfile->buflen) 
    {
		if (cfile->file != -1) 
        {
			cfile->buflen = read(cfile->file, cfile->inbuf, cfile->bufsiz);
			if (cfile->buflen == 0) 
            {
				c = EOF;
				cfile->bufix = 0;
			} 
            else if (cfile->buflen < 0) 
            {
				c = EOF;
				cfile->bufix = cfile->buflen = 0;
			} 
            else
            {
				c = cfile->inbuf[0];        //first char
				cfile->bufix = 1;           //index 
			}
		} 
        else
        {
			c = EOF;
        }
	} 
    else 
    {
        /* 后续每次从buf中取一个字符 */
		c = cfile->inbuf[cfile->bufix];  //buf index 
		cfile->bufix++;
	}

	if (!cfile->ugflag) 
    {
        /* end of line */
		if (c == EOL) 
        {
			if (cfile->cur_line == cfile->line1)
            {   
				cfile->cur_line = cfile->line2;
				cfile->prev_line = cfile->line1;
			}
            else
            {
				cfile->cur_line = cfile->line1;
				cfile->prev_line = cfile->line2;
			}
			cfile->line++;
			cfile->lpos = 1;
			cfile->cur_line[0] = 0;
		} 
        /* end of file */
        else if (c != EOF) 
        {
			if (cfile->lpos <= 80) 
            {
				cfile->cur_line[cfile->lpos - 1] = c;
				cfile->cur_line[cfile->lpos] = 0;
			}
			cfile->lpos++;
		}
	}
    else
    {
		cfile->ugflag = 0;
    }
    
    return c;		
}


static enum dhcp_token read_number(int c, struct parse *cfile)
{
	int i = 0;
	int token = NUMBER;

	cfile->tokbuf[i++] = c;
	for (; i < sizeof(cfile->tokbuf); i++) 
    {
		c = get_char(cfile);

		/* Promote NUMBER -> NUMBER_OR_NAME -> NAME, never demote.
		 * Except in the case of '0x' syntax hex, which gets called
		 * a NAME at '0x', and returned to NUMBER_OR_NAME once it's
		 * verified to be at least 0xf or less.
		 */
		switch(isascii(c) ? token : BREAK) 
        {
		    case NUMBER:
			    if(isdigit(c))
				    break;
			/* FALLTHROUGH */
		    case NUMBER_OR_NAME:
			    if(isxdigit(c)) {
				    token = NUMBER_OR_NAME;
				    break;
			    }
			/* FALLTHROUGH */
		    case NAME:
			    if((i == 2) && isxdigit(c) &&
				(cfile->tokbuf[0] == '0') &&
				((cfile->tokbuf[1] == 'x') || 
				 (cfile->tokbuf[1] == 'X'))) 
		        {
				    token = NUMBER_OR_NAME;
				    break;
			    }
                else if(((c == '-') || (c == '_') || isalnum(c))) 
                {
				    token = NAME;
				    break;
			    }
			/* FALLTHROUGH */
		    case BREAK:
			    /* At this point c is either EOF or part of the next
			    * token.  If not EOF, rewind the file one byte so
			    * the next token is read from there.
			    */
			    if(c != EOF) 
                {
				    cfile->bufix--;
				    cfile->ugflag = 1;
			    }
			    goto end_read;

		    default:
			    printf("read_number():%s:%d: impossible case \r\n", __FUNCTION__, __LINE__);
		}
        
		cfile->tokbuf[i] = c;
	}

	if (i == sizeof(cfile->tokbuf)) 
    {
		printf("numeric token larger than internal buffer \r\n");
		--i;
	}

end_read:
	cfile->tokbuf[i] = 0;
	cfile->tlen = i;
	cfile->tval = cfile->tokbuf;

	return token;
}


static enum dhcp_token read_string(struct parse *cfile)
{
	int i;
	int bs = 0;
	int c;
	int value = 0;
	int hex = 0;

	for (i = 0; i < sizeof(cfile->tokbuf); i++) 
    {
again:
		c = get_char(cfile);
		if (c == EOF) 
        {
			printf("eof in string constant \r\n");
			break;
		}
        
		if (bs == 1)
        {
			switch (c)
            {
			case 't':
				cfile->tokbuf[i] = '\t';
				break;
			case 'r':
				cfile->tokbuf[i] = '\r';
				break;
			case 'n':
				cfile->tokbuf[i] = '\n';
				break;
			case 'b':
				cfile->tokbuf[i] = '\b';
				break;
	        case '0':
	        case '1':
	        case '2':
	        case '3':
				hex = 0;
				value = c - '0';
				++bs;
				goto again;
		    case 'x':
				hex = 1;
				value = 0;
				++bs;
				goto again;
			default:
				cfile->tokbuf[i] = c;
				bs = 0;
				break;
			}
			bs = 0;
		} 
        else if (bs > 1)
        {
			if (hex) 
            {
				if (c >= '0' && c <= '9') {
					value = value * 16 + (c - '0');
				} else if (c >= 'a' && c <= 'f') {
					value = value * 16 + (c - 'a' + 10);
				} else if (c >= 'A' && c <= 'F') {
					value = value * 16 + (c - 'A' + 10);
				} else {
					printf("invalid hex digit: %x \r\n", c);
					bs = 0;
					continue;
				}

                if (++bs == 4) 
                {
					cfile->tokbuf[i] = value;
					bs = 0;
				}
                else
                {
					goto again;
                }
			} 
            else 
            {
				if (c >= '0' && c <= '9') 
                {
					value = value * 8 + (c - '0');
				}
                else
                {
				    if (value != 0) 
                    {
					    printf("invalid octal digit %x \r\n", c);
					    continue;
				    } 
                    else
                    {
					    cfile->tokbuf[i] = 0;
                    }
                    
                    bs = 0;
				}

                if (++bs == 4) 
                {
					cfile->tokbuf[i] = value;
					bs = 0;
				} 
                else
                {
					goto again;
                }
			}
		} 
        else if (c == '\\') 
        {
			bs = 1;
			goto again;
		} 
        else if (c == '"')
        {
			break;
        }
		else
		{
			cfile->tokbuf[i] = c;
		}
	}
    
	/* Normally, I'd feel guilty about this, but we're talking about
	   strings that'll fit in a DHCP packet here... */
	if (i == sizeof(cfile->tokbuf))
    {
		printf("string constant larger than internal buffer \r\n");
        --i;
	}
    
	cfile->tokbuf[i] = 0;
	cfile->tlen = i;
	cfile->tval = cfile->tokbuf;
    
	return STRING;
}

static enum dhcp_token intern(char *atom, enum dhcp_token dfv)
{
	if (!isascii(atom [0]))
		return dfv;

	switch (tolower(atom [0])) 
    {
	    case '-':
		    if (atom [1] == 0)
			    return MINUS;
		    break;

	    case 'a':
		    if (!strncasecmp(atom + 1, "uth", 3)) 
            {
			    if (!strncasecmp(atom + 3, "uthenticat", 10)) 
                {
				    if (!strcasecmp(atom + 13, "ed"))
					    return AUTHENTICATED;
				    if (!strcasecmp(atom + 13, "ion"))
					    return AUTHENTICATION;
				    break;
			    }

                if (!strcasecmp(atom + 1, "uthoritative"))
				    return AUTHORITATIVE;

                break;
		    }

            if (!strcasecmp (atom + 1, "nd"))
			    return AND;
		    if (!strcasecmp (atom + 1, "ppend"))
			    return APPEND;
		    if (!strcasecmp (atom + 1, "llow"))
			    return ALLOW;
		    if (!strcasecmp (atom + 1, "lias"))
			    return ALIAS;
		    if (!strcasecmp (atom + 1, "lgorithm"))
			    return ALGORITHM;
		    if (!strcasecmp (atom + 1, "bandoned"))
			    return TOKEN_ABANDONED;
		    if (!strcasecmp (atom + 1, "dd"))
			    return TOKEN_ADD;
		    if (!strcasecmp (atom + 1, "ll"))
			    return ALL;
		    if (!strcasecmp (atom + 1, "t"))
			    return AT;
		    if (!strcasecmp (atom + 1, "rray"))
			    return ARRAY;
		    if (!strcasecmp (atom + 1, "ddress"))
			    return ADDRESS;
		    if (!strcasecmp (atom + 1, "ctive"))
			    return TOKEN_ACTIVE;
		    break;
	    case 'b':
		    if (!strcasecmp (atom + 1, "ackup"))
			    return TOKEN_BACKUP;
		    if (!strcasecmp (atom + 1, "ootp"))
			    return TOKEN_BOOTP;
		    if (!strcasecmp (atom + 1, "inding"))
			    return BINDING;
		    if (!strcasecmp (atom + 1, "inary-to-ascii"))
			    return BINARY_TO_ASCII;
		    if (!strcasecmp (atom + 1, "ackoff-cutoff"))
			    return BACKOFF_CUTOFF;
		    if (!strcasecmp (atom + 1, "ooting"))
			    return BOOTING;
		    if (!strcasecmp (atom + 1, "oot-unknown-clients"))
			    return BOOT_UNKNOWN_CLIENTS;
		    if (!strcasecmp (atom + 1, "reak"))
			    return BREAK;
		    if (!strcasecmp (atom + 1, "illing"))
			    return BILLING;
		    if (!strcasecmp (atom + 1, "oolean"))
			    return BOOLEAN;
		    if (!strcasecmp (atom + 1, "alance"))
			    return BALANCE;
		    if (!strcasecmp (atom + 1, "ound"))
			    return BOUND;

            break;
	    case 'c':
		    if (!strcasecmp (atom + 1, "ase"))
			    return CASE;
		    if (!strcasecmp (atom + 1, "ommit"))
			    return COMMIT;
		    if (!strcasecmp (atom + 1, "ode"))
			    return CODE;
		    if (!strcasecmp (atom + 1, "onfig-option"))
			    return CONFIG_OPTION;
		    if (!strcasecmp (atom + 1, "heck"))
			    return CHECK;
		    if (!strcasecmp (atom + 1, "lass"))
		    	return CLASS;
		    if (!strcasecmp (atom + 1, "lose"))
			    return TOKEN_CLOSE;
		    if (!strcasecmp (atom + 1, "reate"))
			    return TOKEN_CREATE;
		    if (!strcasecmp (atom + 1, "iaddr"))
			    return CIADDR;
		    if (!strncasecmp (atom + 1, "lient", 5)) 
            {
			    if (!strcasecmp (atom + 6, "-identifier"))
				    return CLIENT_IDENTIFIER;
			    if (!strcasecmp (atom + 6, "-hostname"))
				    return CLIENT_HOSTNAME;
			    if (!strcasecmp (atom + 6, "-state"))
				    return CLIENT_STATE;
			    if (!strcasecmp (atom + 6, "-updates"))
				    return CLIENT_UPDATES;
			    if (!strcasecmp (atom + 6, "s"))
				    return CLIENTS;
		    }
		    if (!strcasecmp (atom + 1, "oncat"))
			    return CONCAT;
		    if (!strcasecmp (atom + 1, "onnect"))
			    return CONNECT;
		    if (!strcasecmp (atom + 1, "ommunications-interrupted"))
			    return COMMUNICATIONS_INTERRUPTED;
		    if (!strcasecmp (atom + 1, "ltt"))
			    return CLTT;
		    break;
	    case 'd':
		    if (!strcasecmp (atom + 1, "ns-update"))
			    return DNS_UPDATE;
		    if (!strcasecmp (atom + 1, "ns-delete"))
			    return DNS_DELETE;
		    if (!strcasecmp (atom + 1, "omain"))
			    return DOMAIN;
		    if (!strcasecmp (atom + 1, "omain-name"))
			    return DOMAIN_NAME;
		    if (!strcasecmp (atom + 1, "o-forward-update"))
			    return DO_FORWARD_UPDATE;
		    if (!strcasecmp (atom + 1, "ebug"))
			    return TOKEN_DEBUG;
		    if (!strcasecmp (atom + 1, "eny"))
			    return DENY;
		    if (!strcasecmp (atom + 1, "eleted"))
			    return TOKEN_DELETED;
		    if (!strcasecmp (atom + 1, "elete"))
			    return TOKEN_DELETE;
		    if (!strncasecmp (atom + 1, "efault", 6))
            {
			    if (!atom [7])
				    return DEFAULT;
			    if (!strcasecmp (atom + 7, "-lease-time"))
				    return DEFAULT_LEASE_TIME;
			    break;
		    }

            if (!strncasecmp (atom + 1, "ynamic", 6)) 
            {
			    if (!atom [7])
				    return DYNAMIC;
			    if (!strncasecmp (atom + 7, "-bootp", 6))
                {
				    if (!atom [13])
					    return DYNAMIC_BOOTP;
				    if (!strcasecmp (atom + 13, "-lease-cutoff"))
					    return DYNAMIC_BOOTP_LEASE_CUTOFF;
				    if (!strcasecmp (atom + 13, "-lease-length"))
					    return DYNAMIC_BOOTP_LEASE_LENGTH;
				    break;
			    }
		    }
		    if (!strcasecmp (atom + 1, "uplicates"))
			    return DUPLICATES;
		    if (!strcasecmp (atom + 1, "eclines"))
			    return DECLINES;
		    if (!strncasecmp (atom + 1, "efine", 5))
            {
			    if (!strcasecmp (atom + 6, "d"))
				    return DEFINED;
			    if (!atom [6])
				    return DEFINE;
		    }
		    break;
	    case 'e':
		if (isascii (atom [1]) && tolower (atom [1]) == 'x') {
			if (!strcasecmp (atom + 2, "tract-int"))
				return EXTRACT_INT;
			if (!strcasecmp (atom + 2, "ists"))
				return EXISTS;
			if (!strcasecmp (atom + 2, "piry"))
				return EXPIRY;
			if (!strcasecmp (atom + 2, "pire"))
				return EXPIRE;
			if (!strcasecmp (atom + 2, "pired"))
				return TOKEN_EXPIRED;
		}
		if (!strcasecmp (atom + 1, "ncode-int"))
			return ENCODE_INT;
		if (!strcasecmp (atom + 1, "thernet"))
			return ETHERNET;
		if (!strcasecmp (atom + 1, "nds"))
			return ENDS;
		if (!strncasecmp (atom + 1, "ls", 2)) {
			if (!strcasecmp (atom + 3, "e"))
				return ELSE;
			if (!strcasecmp (atom + 3, "if"))
				return ELSIF;
			break;
		}
		if (!strcasecmp (atom + 1, "rror"))
			return ERROR;
		if (!strcasecmp (atom + 1, "val"))
			return EVAL;
		if (!strcasecmp (atom + 1, "ncapsulate"))
			return ENCAPSULATE;
		break;
	      case 'f':
		if (!strcasecmp (atom + 1, "atal"))
			return FATAL;
		if (!strcasecmp (atom + 1, "ilename"))
			return FILENAME;
		if (!strcasecmp (atom + 1, "ixed-address"))
			return FIXED_ADDR;
		if (!strcasecmp (atom + 1, "ddi"))
			return FDDI;
		if (!strcasecmp (atom + 1, "ormerr"))
			return NS_FORMERR;
		if (!strcasecmp (atom + 1, "unction"))
			return FUNCTION;
		if (!strcasecmp (atom + 1, "ailover"))
			return FAILOVER;
		if (!strcasecmp (atom + 1, "ree"))
			return TOKEN_FREE;
		break;
	      case 'g':
		if (!strcasecmp (atom + 1, "iaddr"))
			return GIADDR;
		if (!strcasecmp (atom + 1, "roup"))
			return GROUP;
		if (!strcasecmp (atom + 1, "et-lease-hostnames"))
			return GET_LEASE_HOSTNAMES;
		break;
	      case 'h':
		if (!strcasecmp (atom + 1, "ba"))
			return HBA;
		if (!strcasecmp (atom + 1, "ost"))
			return HOST;
		if (!strcasecmp (atom + 1, "ost-decl-name"))
			return HOST_DECL_NAME;
		if (!strcasecmp (atom + 1, "ardware"))
			return HARDWARE;
		if (!strcasecmp (atom + 1, "ostname"))
			return HOSTNAME;
		if (!strcasecmp (atom + 1, "elp"))
			return TOKEN_HELP;
		break;
	      case 'i':
		if (!strcasecmp (atom + 1, "nclude"))
			return INCLUDE;
		if (!strcasecmp (atom + 1, "nteger"))
			return INTEGER;
		if (!strcasecmp (atom + 1, "nfinite"))
			return INFINITE;
		if (!strcasecmp (atom + 1, "nfo"))
			return INFO;
		if (!strcasecmp (atom + 1, "p-address"))
			return IP_ADDRESS;
		if (!strcasecmp (atom + 1, "nitial-interval"))
			return INITIAL_INTERVAL;
		if (!strcasecmp (atom + 1, "nterface"))
			return INTERFACE;
		if (!strcasecmp (atom + 1, "dentifier"))
			return IDENTIFIER;
		if (!strcasecmp (atom + 1, "f"))
			return IF;
		if (!strcasecmp (atom + 1, "s"))
			return IS;
		if (!strcasecmp (atom + 1, "gnore"))
			return IGNORE;
		break;
	      case 'k':
		if (!strncasecmp (atom + 1, "nown", 4)) {
			if (!strcasecmp (atom + 5, "-clients"))
				return KNOWN_CLIENTS;
			if (!atom[5])
				return KNOWN;
			break;
		}
		if (!strcasecmp (atom + 1, "ey"))
			return KEY;
		break;
	      case 'l':
		if (!strcasecmp (atom + 1, "ease"))
			return LEASE;
		if (!strcasecmp (atom + 1, "eased-address"))
			return LEASED_ADDRESS;
		if (!strcasecmp (atom + 1, "ease-time"))
			return LEASE_TIME;
		if (!strcasecmp (atom + 1, "imit"))
			return LIMIT;
		if (!strcasecmp (atom + 1, "et"))
			return LET;
		if (!strcasecmp (atom + 1, "oad"))
			return LOAD;
		if (!strcasecmp (atom + 1, "og"))
			return LOG;
		break;
	      case 'm':
		if (!strncasecmp (atom + 1, "ax", 2)) {
			if (!atom [3])
				return TOKEN_MAX;
			if (!strcasecmp (atom + 3, "-lease-time"))
				return MAX_LEASE_TIME;
			if (!strcasecmp (atom + 3, "-transmit-idle"))
				return MAX_TRANSMIT_IDLE;
			if (!strcasecmp (atom + 3, "-response-delay"))
				return MAX_RESPONSE_DELAY;
			if (!strcasecmp (atom + 3, "-unacked-updates"))
				return MAX_UNACKED_UPDATES;
		}
		if (!strncasecmp (atom + 1, "in-", 3)) {
			if (!strcasecmp (atom + 4, "lease-time"))
				return MIN_LEASE_TIME;
			if (!strcasecmp (atom + 4, "secs"))
				return MIN_SECS;
			break;
		}
		if (!strncasecmp (atom + 1, "edi", 3)) {
			if (!strcasecmp (atom + 4, "a"))
				return MEDIA;
			if (!strcasecmp (atom + 4, "um"))
				return MEDIUM;
			break;
		}
		if (!strcasecmp (atom + 1, "atch"))
			return MATCH;
		if (!strcasecmp (atom + 1, "embers"))
			return MEMBERS;
		if (!strcasecmp (atom + 1, "y"))
			return MY;
		if (!strcasecmp (atom + 1, "clt"))
			return MCLT;
		break;
	      case 'n':
		if (!strcasecmp (atom + 1, "ormal"))
			return NORMAL;
		if (!strcasecmp (atom + 1, "ameserver"))
			return NAMESERVER;
		if (!strcasecmp (atom + 1, "etmask"))
			return NETMASK;
		if (!strcasecmp (atom + 1, "ever"))
			return NEVER;
		if (!strcasecmp (atom + 1, "ext-server"))
			return NEXT_SERVER;
		if (!strcasecmp (atom + 1, "ot"))
			return TOKEN_NOT;
		if (!strcasecmp (atom + 1, "o"))
			return NO;
		if (!strcasecmp (atom + 1, "s-update"))
			return NS_UPDATE;
		if (!strcasecmp (atom + 1, "oerror"))
			return NS_NOERROR;
		if (!strcasecmp (atom + 1, "otauth"))
			return NS_NOTAUTH;
		if (!strcasecmp (atom + 1, "otimp"))
			return NS_NOTIMP;
		if (!strcasecmp (atom + 1, "otzone"))
			return NS_NOTZONE;
		if (!strcasecmp (atom + 1, "xdomain"))
			return NS_NXDOMAIN;
		if (!strcasecmp (atom + 1, "xrrset"))
			return NS_NXRRSET;
		if (!strcasecmp (atom + 1, "ull"))
			return TOKEN_NULL;
		if (!strcasecmp (atom + 1, "ext"))
			return TOKEN_NEXT;
		if (!strcasecmp (atom + 1, "ew"))
			return TOKEN_NEW;
		break;
	      case 'o':
		if (!strcasecmp (atom + 1, "mapi"))
			return OMAPI;
		if (!strcasecmp (atom + 1, "r"))
			return OR;
		if (!strcasecmp (atom + 1, "n"))
			return ON;
		if (!strcasecmp (atom + 1, "pen"))
			return TOKEN_OPEN;
		if (!strcasecmp (atom + 1, "ption"))
			return OPTION;
		if (!strcasecmp (atom + 1, "ne-lease-per-client"))
			return ONE_LEASE_PER_CLIENT;
		if (!strcasecmp (atom + 1, "f"))
			return OF;
		if (!strcasecmp (atom + 1, "wner"))
			return OWNER;
		break;
	      case 'p':
		if (!strcasecmp (atom + 1, "repend"))
			return PREPEND;
		if (!strcasecmp (atom + 1, "acket"))
			return PACKET;
		if (!strcasecmp (atom + 1, "ool"))
			return POOL;
		if (!strcasecmp (atom + 1, "seudo"))
			return PSEUDO;
		if (!strcasecmp (atom + 1, "eer"))
			return PEER;
		if (!strcasecmp (atom + 1, "rimary"))
			return PRIMARY;
		if (!strncasecmp (atom + 1, "artner", 6)) {
			if (!atom [7])
				return PARTNER;
			if (!strcasecmp (atom + 7, "-down"))
				return PARTNER_DOWN;
		}
		if (!strcasecmp (atom + 1, "ort"))
			return PORT;
		if (!strcasecmp (atom + 1, "otential-conflict"))
			return POTENTIAL_CONFLICT;
		if (!strcasecmp (atom + 1, "ick-first-value") ||
		    !strcasecmp (atom + 1, "ick"))
			return PICK;
		if (!strcasecmp (atom + 1, "aused"))
			return PAUSED;
		break;
	      case 'r':
		if (!strcasecmp (atom + 1, "esolution-interrupted"))
			return RESOLUTION_INTERRUPTED;
		if (!strcasecmp (atom + 1, "ange"))
			return RANGE;
		if (!strcasecmp (atom + 1, "ecover"))
			return RECOVER;
		if (!strcasecmp (atom + 1, "ecover-done"))
			return RECOVER_DONE;
		if (!strcasecmp (atom + 1, "ecover-wait"))
			return RECOVER_WAIT;
		if (!strcasecmp (atom + 1, "econtact-interval"))
			return RECONTACT_INTERVAL;
		if (!strcasecmp (atom + 1, "equest"))
			return REQUEST;
		if (!strcasecmp (atom + 1, "equire"))
			return REQUIRE;
		if (!strcasecmp (atom + 1, "equire"))
			return REQUIRE;
		if (!strcasecmp (atom + 1, "etry"))
			return RETRY;
		if (!strcasecmp (atom + 1, "eturn"))
			return RETURN;
		if (!strcasecmp (atom + 1, "enew"))
			return RENEW;
		if (!strcasecmp (atom + 1, "ebind"))
			return REBIND;
		if (!strcasecmp (atom + 1, "eboot"))
			return REBOOT;
		if (!strcasecmp (atom + 1, "eject"))
			return REJECT;
		if (!strcasecmp (atom + 1, "everse"))
			return REVERSE;
		if (!strcasecmp (atom + 1, "elease"))
			return RELEASE;
		if (!strcasecmp (atom + 1, "efused"))
			return NS_REFUSED;
		if (!strcasecmp (atom + 1, "eleased"))
			return TOKEN_RELEASED;
		if (!strcasecmp (atom + 1, "eset"))
			return TOKEN_RESET;
		if (!strcasecmp (atom + 1, "eserved"))
			return TOKEN_RESERVED;
		if (!strcasecmp (atom + 1, "emove"))
			return REMOVE;
		if (!strcasecmp (atom + 1, "efresh"))
			return REFRESH;
		break;
	      case 's':
		if (!strcasecmp (atom + 1, "tate"))
			return STATE;
		if (!strcasecmp (atom + 1, "ecret"))
			return SECRET;
		if (!strcasecmp (atom + 1, "ervfail"))
			return NS_SERVFAIL;
		if (!strcasecmp (atom + 1, "witch"))
			return SWITCH;
		if (!strcasecmp (atom + 1, "igned"))
			return SIGNED;
		if (!strcasecmp (atom + 1, "tring"))
			return STRING_TOKEN;
		if (!strcasecmp (atom + 1, "uffix"))
			return SUFFIX;
		if (!strcasecmp (atom + 1, "earch"))
			return SEARCH;
		if (!strcasecmp (atom + 1, "tarts"))
			return STARTS;
		if (!strcasecmp (atom + 1, "iaddr"))
			return SIADDR;
		if (!strcasecmp (atom + 1, "hared-network"))
			return SHARED_NETWORK;
		if (!strcasecmp (atom + 1, "econdary"))
			return SECONDARY;
		if (!strcasecmp (atom + 1, "erver-name"))
			return SERVER_NAME;
		if (!strcasecmp (atom + 1, "erver-identifier"))
			return SERVER_IDENTIFIER;
		if (!strcasecmp (atom + 1, "erver"))
			return SERVER;
		if (!strcasecmp (atom + 1, "elect-timeout"))
			return SELECT_TIMEOUT;
		if (!strcasecmp (atom + 1, "elect"))
			return SELECT;
		if (!strcasecmp (atom + 1, "end"))
			return SEND;
		if (!strcasecmp (atom + 1, "cript"))
			return SCRIPT;
		if (!strcasecmp (atom + 1, "upersede"))
			return SUPERSEDE;
		if (!strncasecmp (atom + 1, "ub", 2)) {
			if (!strcasecmp (atom + 3, "string"))
				return SUBSTRING;
			if (!strcasecmp (atom + 3, "net"))
				return SUBNET;
			if (!strcasecmp (atom + 3, "class"))
				return SUBCLASS;
			break;
		}
		if (!strcasecmp (atom + 1, "pawn"))
			return SPAWN;
		if (!strcasecmp (atom + 1, "pace"))
			return SPACE;
		if (!strcasecmp (atom + 1, "tatic"))
			return STATIC;
		if (!strcasecmp (atom + 1, "plit"))
			return SPLIT;
		if (!strcasecmp (atom + 1, "et"))
			return TOKEN_SET;
		if (!strcasecmp (atom + 1, "econds"))
			return SECONDS;
		if (!strcasecmp (atom + 1, "hutdown"))
			return SHUTDOWN;
		if (!strcasecmp (atom + 1, "tartup"))
			return STARTUP;
		break;
	      case 't':
		if (!strcasecmp (atom + 1, "imestamp"))
			return TIMESTAMP;
		if (!strcasecmp (atom + 1, "imeout"))
			return TIMEOUT;
		if (!strcasecmp (atom + 1, "oken-ring"))
			return TOKEN_RING;
		if (!strcasecmp (atom + 1, "ext"))
			return TEXT;
		if (!strcasecmp (atom + 1, "stp"))
			return TSTP;
		if (!strcasecmp (atom + 1, "sfp"))
			return TSFP;
		if (!strcasecmp (atom + 1, "ransmission"))
			return TRANSMISSION;
		break;
	      case 'u':
		if (!strcasecmp (atom + 1, "nset"))
			return UNSET;
		if (!strcasecmp (atom + 1, "nsigned"))
			return UNSIGNED;
		if (!strcasecmp (atom + 1, "id"))
			return UID;
		if (!strncasecmp (atom + 1, "se", 2)) {
			if (!strcasecmp (atom + 3, "r-class"))
				return USER_CLASS;
			if (!strcasecmp (atom + 3, "-host-decl-names"))
				return USE_HOST_DECL_NAMES;
			if (!strcasecmp (atom + 3,
					 "-lease-addr-for-default-route"))
				return USE_LEASE_ADDR_FOR_DEFAULT_ROUTE;
			break;
		}
		if (!strncasecmp (atom + 1, "nknown", 6)) {
			if (!strcasecmp (atom + 7, "-clients"))
				return UNKNOWN_CLIENTS;
			if (!strcasecmp (atom + 7, "-state"))
				return UNKNOWN_STATE;
			if (!atom [7])
				return UNKNOWN;
			break;
		}
		if (!strcasecmp (atom + 1, "nauthenticated"))
			return AUTHENTICATED;
		if (!strcasecmp (atom + 1, "pdated-dns-rr"))
			return UPDATED_DNS_RR;
		if (!strcasecmp (atom + 1, "pdate"))
			return UPDATE;
		break;
	      case 'v':
		if (!strcasecmp (atom + 1, "endor-class"))
			return VENDOR_CLASS;
		if (!strcasecmp (atom + 1, "endor"))
			return VENDOR;
		break;
	      case 'w':
		if (!strcasecmp (atom + 1, "ith"))
			return WITH;
		break;
	      case 'y':
		if (!strcasecmp (atom + 1, "iaddr"))
			return YIADDR;
		if (!strcasecmp (atom + 1, "xdomain"))
			return NS_YXDOMAIN;
		if (!strcasecmp (atom + 1, "xrrset"))
			return NS_YXRRSET;
		break;
	      case 'z':
		if (!strcasecmp (atom + 1, "one"))
			return ZONE;
		break;
	}
	return dfv;
}


static enum dhcp_token read_num_or_name(int c, struct parse *cfile)
{
	int i = 0;
	enum dhcp_token rv = NUMBER_OR_NAME;
	cfile->tokbuf[i++] = c;
    
	for (; i < sizeof(cfile->tokbuf); i++) 
    {
		c = get_char(cfile);
        
		if (!isascii(c) ||
		    (c != '-' && c != '_' && !isalnum(c)))
		{
			if (c != EOF) 
            {
				cfile->bufix--;
				cfile->ugflag = 1;
			}
			break;
		}
		if (!isxdigit(c))
			rv = NAME;
		cfile->tokbuf[i] = c;
	}

    if (i == sizeof(cfile->tokbuf))
    {
		printf("token larger than internal buffer\r\n");
		--i;
	}
    
	cfile->tokbuf[i] = 0;
	cfile->tlen = i;
	cfile->tval = cfile->tokbuf;
	return intern(cfile->tval, rv);
}


static void skip_to_eol(struct parse *cfile)
{
	int c;
	do {
		c = get_char(cfile);
		if (c == EOF)
			return;
        
#ifdef OLD_LEXER
		if (cfile->comment_index < sizeof(cfile->comments))
			cfile->comments[cfile->comment_index++] = c;
#endif

		if (c == EOL) {
			return;
		}
	} while (1);
}


static enum dhcp_token get_token(struct parse *cfile)
{
	int c;
	enum dhcp_token ttok;
	static char tb[2];
	int l, p, u;

	do {
		l = cfile->line;
		p = cfile->lpos;
		u = cfile->ugflag;

		c = get_char(cfile);

        //printf("c = %d, cfile->eol_token: %d %s %d\r\n", c, cfile->eol_token, __FUNCTION__, __LINE__);
        
		if (!(c == '\n' && cfile->eol_token)
		    && isascii(c) && isspace(c))
		{
			continue;
		}
        
        if (c == '#') 
        {
			skip_to_eol(cfile);
			continue;
		}

        if (c == '"') 
        {
			cfile->lexline = l;
			cfile->lexchar = p;
			ttok = read_string(cfile);
			break;
		}
        
		if ((isascii(c) && isdigit(c)) || c == '-') 
        {
			cfile->lexline = l;
			cfile->lexchar = p;
			ttok = read_number(c, cfile);
			break;
		}
        else if (isascii(c) && isalpha(c)) 
        {
            //printf("....... is here %s %d\r\n", __FUNCTION__, __LINE__);
			cfile->lexline = l;
			cfile->lexchar = p;
			ttok = read_num_or_name(c, cfile);
            //printf(".......ttok: %d is here %s %d\r\n", ttok, __FUNCTION__, __LINE__);
			break;
		} 
        else if (c == EOF)
        {
			ttok = END_OF_FILE;
			cfile->tlen = 0;
			break;
		} 
        else 
        {
			cfile->lexline = l;
			cfile->lexchar = p;
			tb[0] = c;
			tb[1] = 0;
			cfile->tval = tb;
			cfile->tlen = 1;
			ttok = c;
			break;
		}
        
	} while (1);
    
	return ttok;
}


enum dhcp_token next_token(const char **rval, unsigned *rlen, struct parse *cfile)
{
	int rv;
    
	if (cfile->token) // 0
    {
		if (cfile->lexline != cfile->tline)
		{
			cfile->token_line = cfile->cur_line;
		}
        
		cfile->lexchar = cfile->tlpos;
		cfile->lexline = cfile->tline;
		rv = cfile->token;
		cfile->token = 0;
	} 
    else 
    {
		rv = get_token(cfile);
		cfile->token_line = cfile->cur_line;
	}
    
	if (rval)
		*rval = cfile->tval;
	if (rlen)
		*rlen = cfile->tlen;

	return rv;
}

enum dhcp_token peek_token(const char **rval, unsigned int *rlen, struct parse *cfile)
{
	int x;

	if (!cfile->token) 
    {
		cfile->tlpos = cfile->lexchar;
		cfile->tline = cfile->lexline;
		cfile->token = get_token(cfile);
		if (cfile->lexline != cfile->tline)
			cfile->token_line = cfile->prev_line;

		x = cfile->lexchar;
		cfile->lexchar = cfile->tlpos;
		cfile->tlpos = x;

		x = cfile->lexline;
		cfile->lexline = cfile->tline;
		cfile->tline = x;
	}
    
	if (rval)
		*rval = cfile->tval;
	if (rlen)
		*rlen = cfile->tlen;
    
	return cfile->token;
}


int new_parse(struct parse **cfile, 
                int file, 
                char *inbuf, 
                unsigned buflen, 
                const char *name, 
                int eolp)
{
    struct parse *tmp;
    tmp = (struct parse *)malloc(sizeof(struct parse));
    if (!tmp)
    {
        return (-1);
    }

    memset(tmp, 0, sizeof(*tmp));
    tmp->token = 0;
    tmp->tlname = name;
    tmp->lpos = tmp->line = 1;
    tmp->cur_line = tmp->line1;
    tmp->prev_line = tmp->line2;
    tmp->token_line = tmp->cur_line;
    tmp->cur_line[0] = tmp->prev_line[0] = 0;
    tmp->warnings_occurred = 0;
    tmp->file = file;
    tmp->eol_token = eolp;

    tmp->bufix = 0;
    tmp->buflen = buflen;
    if (inbuf) 
    {
        tmp->bufsiz = 0;
        tmp->inbuf = inbuf;
    }
    else
    {
        tmp->inbuf = (char *)malloc(8192);
        if (!tmp->inbuf) 
        {
            free(tmp);
            return (-1);
        }

        tmp->bufsiz = 8192;
    }

    *cfile = tmp;

    return (0);
}

int end_parse(struct parse **cfile)
{
	if ((*cfile)->bufsiz) {
		free((*cfile)->inbuf);
	}
    
    free(*cfile);
    
	*cfile = (struct parse *)0;

    return (0);
}


void skip_to_rbrace(struct parse *cfile, int brace_count)
{
	enum dhcp_token token;
	const char *val;

	do 
    {
		token = peek_token(&val, (unsigned *)0, cfile);
		if (token == RBRACE) 
        {
			token = next_token(&val, (unsigned *)0, cfile);
			if (brace_count) 
            {
				if (!--brace_count)
					return;
			}
            else
			{
				return;
            }
		}
        else if (token == LBRACE) 
	    {
			brace_count++;
		} 
        else if (token == SEMI && !brace_count)
        {
			token = next_token(&val, (unsigned *)0, cfile);
			return;
		}
        else if (token == EOL) 
        {
			/* EOL only happens when parsing /etc/resolv.conf,
			   and we treat it like a semicolon because the
			   resolv.conf file is line-oriented. */
			token = next_token(&val, (unsigned *)0, cfile);
			return;
		}

        token = next_token(&val, (unsigned *)0, cfile);
        
	} while (token != END_OF_FILE);
}

void skip_to_semi(struct parse *cfile)
{
	skip_to_rbrace(cfile, 0);
}

int parse_semi(struct parse *cfile)
{
	enum dhcp_token token;
	const char *val;

	token = next_token(&val, (unsigned *)0, cfile);
	if (token != SEMI) 
    {
		printf("semicolon expected. \r\n");
		skip_to_semi (cfile);
		return 0;
	}
    
	return 1;
}


void putShort(unsigned char *obuf, int32_t val)
{
	int16_t tmp = htons(val);
	memcpy(obuf, &tmp, sizeof(tmp));
}


void putUShort(unsigned char *obuf, u_int32_t val)
{
	u_int16_t tmp = htons(val);
	memcpy(obuf, &tmp, sizeof(tmp));
}

void putLong(unsigned char *obuf, int32_t val)
{
	int32_t tmp = htonl(val);
	memcpy(obuf, &tmp, sizeof(tmp));
}    

void putULong (unsigned char *obuf, u_int32_t val)
{
	u_int32_t tmp = htonl(val);
	memcpy(obuf, &tmp, sizeof(tmp));
}

void convert_num(struct parse *cfile, 
                  unsigned char *buf, 
                  const char *str, 
                  int base, 
                  unsigned size)
{
	const char *ptr = str;
	int negative = 0;
	u_int32_t val = 0;
	int tval;
	int max;

	if (*ptr == '-') 
    {
		negative = 1;
		++ptr;
	}

	/* If base wasn't specified, figure it out from the data. */
	if (!base) 
    {
		if (ptr[0] == '0') 
        {
			if (ptr[1] == 'x') 
            {
				base = 16;
				ptr += 2;
			}
            else if (isascii(ptr[1]) && isdigit(ptr[1])) 
            {
				base = 8;
				ptr += 1;
			} 
            else
            {
				base = 10;
			}
		}
        else
		{
			base = 10;
		}
	}

	do {
		tval = *ptr++;
		/* XXX assumes ASCII... */
		if (tval >= 'a')
			tval = tval - 'a' + 10;
		else if (tval >= 'A')
			tval = tval - 'A' + 10;
		else if (tval >= '0')
			tval -= '0';
		else {
			printf("Bogus number: %s. \r\n", str);
			break;
		}

        if (tval >= base) 
        {
			printf("Bogus number %s: digit %d not in base %d. \r\n",
				    str, tval, base);
			break;
		}
		val = val * base + tval;
        
	} while (*ptr);

	if (negative)
		max = (1 << (size - 1));
	else
		max = (1 << (size - 1)) + ((1 << (size - 1)) - 1);

    if (val > max) 
    {
		switch (base) 
        {
		case 8:
			printf("%s%lo exceeds max (%d) for precision.",
				    negative ? "-" : "",
				    (unsigned long)val, max);
			break;
		case 16:
			printf("%s%lx exceeds max (%d) for precision.",
				   negative ? "-" : "",
				   (unsigned long)val, max);
			break;
		default:
			printf("%s%lu exceeds max (%d) for precision.",
				    negative ? "-" : "",
				    (unsigned long)val, max);
			break;
		}
	}

	if (negative) 
    {
		switch (size) 
        {
		case 8:
			*buf = -(unsigned long)val;
			break;
		case 16:
			putShort(buf, -(long)val);
			break;
		case 32:
			putLong (buf, -(long)val);
			break;
		default:
			printf("Unexpected integer size: %d\n", size);
			break;
		}
	} 
    else 
    {
		switch (size) 
        {
		case 8:
			*buf = (u_int8_t)val;
			break;
		case 16:
			putUShort (buf, (u_int16_t)val);
			break;
		case 32:
			putULong (buf, val);
			break;
		default:
			printf("Unexpected integer size: %d\n", size);
			break;
		}
	}
}

pair cons(caddr_t car, pair cdr)
{
	pair foo = (pair)malloc (sizeof *foo);
	if (!foo)
	{
		printf("no memory for cons. \r\n");
        return NULL;
    }
    
	foo -> car = car;
	foo -> cdr = cdr;

    return foo;
}


unsigned char *parse_numeric_aggregate(struct parse *cfile, 
                                       unsigned char *buf,  //buf[16]
					                   unsigned *max,       // 4
					                   int seperator, 
					                   int base,            // 10
					                   unsigned size)       // 8
{
	const char *val;
	enum dhcp_token token;
	unsigned char *bufp = buf, *s, *t;
	unsigned count = 0;
	pair c = (pair)0;

	if (!bufp && *max) 
    {
		bufp = (unsigned char *)malloc (*max * size / 8);
		if (!bufp) {
			printf("no space for numeric aggregate. \r\n");
		}
		s = 0;
	} 
    else
    {
		s = bufp;
    }

	do 
    {
		if (count) 
        {
			token = peek_token(&val, (unsigned *)0, cfile);
			if (token != seperator) 
            {
				if (!*max)
					break;
				if (token != RBRACE && token != LBRACE)
					token = next_token (&val,
							    (unsigned *)0,
							    cfile);
				printf("too few numbers. \r\n");
				if (token != SEMI)
					skip_to_semi (cfile);
				return (unsigned char *)0;
			}
			token = next_token (&val, (unsigned *)0, cfile);
		}

        token = next_token(&val, (unsigned *)0, cfile);
		if (token == END_OF_FILE) 
        {
			printf("unexpected end of file. \r\n");
			break;
		}

		/* Allow NUMBER_OR_NAME if base is 16. */
		if (token != NUMBER &&
		    (base != 16 || token != NUMBER_OR_NAME)) 
		{
			printf("expecting numeric value. \r\n");
			skip_to_semi(cfile);
			return (unsigned char *)0;
		}
        
		/* If we can, convert the number now; otherwise, build
		   a linked list of all the numbers. */
		if (s) 
        {
			convert_num (cfile, s, val, base, size);
			s += size / 8;
		}
        else 
        {
			t = (unsigned char *)malloc(strlen (val) + 1);
			if (!t)
			{
				printf("no temp space for number. \r\n");
                return 0;
			}
            
			strcpy((char *)t, val);
			c = cons ((caddr_t)t, c);
		}
	} while (++count != *max);

	/* If we had to cons up a list, convert it now. */
	if (c) {
		bufp = (unsigned char *)malloc(count * size / 8);
		if (!bufp)
		{
			printf("no space for numeric aggregate. \r\n");
            return 0;
		}
        
		s = bufp + count - size / 8;
		*max = count;
	}
	while (c) {
		pair cdr = c -> cdr;
		convert_num (cfile, s, (char *)(c->car), base, size);
		s -= size / 8;
		/* Free up temp space. */
		free (c->car);
		free (c);
		c = cdr;
	}
	return bufp;
}

/*
 * ip-address :== NUMBER DOT NUMBER DOT NUMBER DOT NUMBER
 */

int parse_ip_addr(struct parse *cfile, struct iaddr *addr)
{
	const char *val;
	enum dhcp_token token;

	addr->len = 4;
    
	if (parse_numeric_aggregate(cfile, addr->iabuf, &addr->len, DOT, 10, 8))
		return 1;
    
	return 0;
}


time_t parse_date(struct parse *cfile)
{
	//struct tm tm;
	int guess;
	int tzoff, wday, year, mon, mday, hour, min, sec;
	const char *val;
	enum dhcp_token token;
	static int months[11] = { 31, 59, 90, 120, 151, 181,
					  212, 243, 273, 304, 334 };

	/* Day of week, or "never"... */
	token = next_token(&val, (unsigned *)0, cfile);
	if (token == NEVER) 
    {
		if (!parse_semi(cfile))
			return 0;
        
		return MAX_TIME;
	}

	if (token != NUMBER) 
    {
		printf("numeric day of week expected. \r\n");
		if (token != SEMI)
			skip_to_semi(cfile);
		return (time_t)0;
	}
    
	wday = atoi(val);

	/* Year... */
	token = next_token (&val, (unsigned *)0, cfile);
	if (token != NUMBER) 
    {
		printf("numeric year expected. \r\n");
		if (token != SEMI)
			skip_to_semi(cfile);
		return (time_t)0;
	}

	/* Note: the following is not a Y2K bug - it's a Y1.9K bug.   Until
	   somebody invents a time machine, I think we can safely disregard
	   it.   This actually works around a stupid Y2K bug that was present
	   in a very early beta release of dhcpd. */
	year = atoi (val);
	if (year > 1900)
		year -= 1900;

	/* Slash seperating year from month... */
	token = next_token(&val, (unsigned *)0, cfile);
	if (token != SLASH) 
    {
		printf("expected slash seperating year from month. \r\n");
		if (token != SEMI)
			skip_to_semi (cfile);
		return (time_t)0;
	}

	/* Month... */
	token = next_token(&val, (unsigned *)0, cfile);
	if (token != NUMBER) 
    {
		printf("numeric month expected. \r\n");
		if (token != SEMI)
			skip_to_semi(cfile);
		return (time_t)0;
	}
    
	mon = atoi(val) - 1;

	/* Slash seperating month from day... */
	token = next_token (&val, (unsigned *)0, cfile);
	if (token != SLASH) 
    {
		printf("expected slash seperating month from day.\r\n");
		if (token != SEMI)
			skip_to_semi (cfile);
		return (time_t)0;
	}

	/* Day of month... */
	token = next_token(&val, (unsigned *)0, cfile);
	if (token != NUMBER) 
    {
		printf("numeric day of month expected. \r\n");
		if (token != SEMI)
			skip_to_semi(cfile);
		return (time_t)0;
	}
    
	mday = atoi(val);

	/* Hour... */
	token = next_token (&val, (unsigned *)0, cfile);
	if (token != NUMBER) {
		printf("numeric hour expected. \r\n");
		if (token != SEMI)
			skip_to_semi (cfile);
		return 0;
	}
    
	hour = atoi(val);

	/* Colon seperating hour from minute... */
	token = next_token(&val, (unsigned *)0, cfile);
	if (token != COLON) {
		printf("expected colon seperating hour from minute.\r\n");
		if (token != SEMI)
			skip_to_semi (cfile);
		return 0;
	}

	/* Minute... */
	token = next_token (&val, (unsigned *)0, cfile);
	if (token != NUMBER) {
		printf("numeric minute expected. \r\n");
		if (token != SEMI)
			skip_to_semi (cfile);
		return (time_t)0;
	}
	min = atoi (val);

	/* Colon seperating minute from second... */
	token = next_token(&val, (unsigned *)0, cfile);
	if (token != COLON) 
    {
		printf("expected colon seperating hour from minute. \r\n");
		if (token != SEMI)
			skip_to_semi(cfile);
		return (time_t)0;
	}

	/* Minute... */
	token = next_token (&val, (unsigned *)0, cfile);
	if (token != NUMBER) {
		printf("numeric minute expected. \r\n");
		if (token != SEMI)
			skip_to_semi (cfile);
		return 0;
	}
	sec = atoi (val);

	token = peek_token (&val, (unsigned *)0, cfile);
	if (token == NUMBER) {
		token = next_token (&val, (unsigned *)0, cfile);
		tzoff = atoi (val);
	} else
		tzoff = 0;

	/* Make sure the date ends in a semicolon... */
	if (!parse_semi (cfile))
		return 0;

	/* Guess the time value... */
	guess = ((((((365 * (year - 70) +	/* Days in years since '70 */
		      (year - 69) / 4 +		/* Leap days since '70 */
		      (mon			/* Days in months this year */
		       ? months [mon - 1]
		       : 0) +
		      (mon > 1 &&		/* Leap day this year */
		       !((year - 72) & 3)) +
		      mday - 1) * 24) +		/* Day of month */
		    hour) * 60) +
		  min) * 60) + sec + tzoff;

	/* This guess could be wrong because of leap seconds or other
	   weirdness we don't know about that the system does.   For
	   now, we're just going to accept the guess, but at some point
	   it might be nice to do a successive approximation here to
	   get an exact value.   Even if the error is small, if the
	   server is restarted frequently (and thus the lease database
	   is reread), the error could accumulate into something
	   significant. */

	return guess;
}

void free_client_lease(struct client_lease *lease, const char *file, int line)
{   
    if (lease)
    {
	    free(lease);
    }
}

int option_state_dereference(struct option_state **ptr, const char *file, int line)
{
	int i;
	struct option_state *options;

	if (!ptr || !*ptr) 
    {
		printf("%s(%d): null pointer", file, line);
		return 0;
	}

	options = *ptr;
	*ptr = (struct option_state *)0;
	--options->refcnt;
	
	if (options->refcnt > 0)
		return 1;

	if (options->refcnt < 0) {
		printf("%s(%d): negative refcnt!", file, line);
		return 0;
	}

#if 0
	/* Loop through the per-universe state. */
	for (i = 0; i < options->universe_count; i++)
		if (options->universes[i] &&
		    universes[i]-> option_state_dereference)
			((*(universes [i] -> option_state_dereference))
			 (universes [i], options, file, line));
	dfree (options, file, line);
#endif
    
	return 1;
}


void destroy_client_lease (struct client_lease *lease)
{
	int i;

	if (lease->server_name)
		free(lease->server_name);
	if (lease->filename)
		free(lease->filename);
    
	option_state_dereference (&lease->options, MDL);
	free_client_lease(lease, MDL);
}


/* string-parameter :== STRING SEMI */
int parse_string(struct parse *cfile, char **sptr, unsigned *lptr)
{
	const char *val;
	enum dhcp_token token;
	char *s;
	unsigned len;

	token = next_token(&val, &len, cfile);
	if (token != STRING) 
    {
		printf("expecting a string \r\n");
		skip_to_semi (cfile);
		return 0;
	}
    
	s = (char *)malloc(len + 1);
	if (!s)
	{
		printf("no memory for string %s. \r\n", val);
        return 0;
	}
    
    memcpy(s, val, len + 1);

	if (!parse_semi(cfile))
    {
		free(s);
		return 0;
	}
    
	if (sptr)
		*sptr = s;
	else
		free(s);
    
	if (lptr)
		*lptr = len;

    return 1;
}


void make_client_state(struct client_state **state)
{
	*state = (struct client_state *)malloc(sizeof(**state));
	if (!*state)
	{
		printf("no memory for client state\n");
        return;
	}
    
	memset (*state, 0, sizeof(**state));
}

#if 1
void parse_client_lease_declaration(struct parse *cfile, 
                                     struct client_lease *lease, 
                                     struct interface_info **ipp, 
                                     struct client_state **clientp)
{
	const char *val;
	char *t, *n;
    enum dhcp_token token;
	struct interface_info *ip;
	//struct option_cache *oc;
	struct client_state *client = (struct client_state *)0;
	//struct data_string key_id;

    token = next_token(&val, (unsigned *)0, cfile);

    printf("token: %d %s %d\r\n", token, __FUNCTION__, __LINE__);
  
	switch (token) 
    {
	case KEY:

		token = next_token(&val, (unsigned *)0, cfile);
		if (token != STRING && !is_identifier(token)) 
        {
			printf("expecting key name. \r\n");
			skip_to_semi (cfile);
			break;
		}
        
	    parse_semi(cfile);

        break;
	case TOKEN_BOOTP:
		lease->is_bootp = 1;
		break;

	case INTERFACE:
		token = next_token(&val, (unsigned *)0, cfile);

        printf("token: %d %s %d\r\n", token, __FUNCTION__, __LINE__);
          
		if (token != STRING) 
        {
			printf("expecting interface name (in quotes). \r\n");
			skip_to_semi (cfile);
			break;
		}

        //interface_or_dummy(ipp, val);
        printf("val: %s \r\n", val);

        break;

	case NAME:
		token = next_token(&val, (unsigned *)0, cfile);
		ip = *ipp;
		if (!ip) 
        {
			printf("state name precedes interface. \r\n");
			break;
		}
		for (client = ip->client; client; client = client->next)
			if (client->name && !strcmp(client->name, val))
				break;
		if (!client)
			printf("lease specified for unknown pseudo. \r\n");
		*clientp = client;
		break;

	case FIXED_ADDR:
		if (!parse_ip_addr(cfile, &lease->address))
			return;
		break;

	case MEDIUM:
		parse_string_list(cfile, &lease->medium, 0);
		return;

	case FILENAME:
		parse_string(cfile, &lease->filename, (unsigned *)0);
		return;

	case SERVER_NAME:
		parse_string (cfile, &lease->server_name, (unsigned *)0);
		return;

	case RENEW:
		lease->renewal = parse_date(cfile);
		return;

	case REBIND:
		lease->rebind = parse_date(cfile);
		return;

	case EXPIRE:
		lease->expiry = parse_date(cfile);
		return;

	case OPTION:
#if 0
		oc = (struct option_cache *)0;
		if (parse_option_decl (&oc, cfile)) {
			save_option (oc -> option -> universe,
				     lease -> options, oc);
			option_cache_dereference (&oc, MDL);
		}
#endif
		return;

	default:
	    printf("expecting lease declaration. \r\n");
		skip_to_semi (cfile);
		break;
	}
    
	token = next_token (&val, (unsigned *)0, cfile);
	if (token != SEMI) {
		printf("expecting semicolon. \r\n");
		skip_to_semi (cfile);
	}
}

void parse_string_list (struct parse *cfile, struct string_list **lp, int multiple)
{
	enum dhcp_token token;
	const char *val;
	struct string_list *cur, *tmp;

	/* Find the last medium in the media list. */
	if (*lp) {
		for (cur = *lp; cur->next; cur = cur->next)
			;
	} else {
		cur = (struct string_list *)0;
	}

	do {
		token = next_token (&val, (unsigned *)0, cfile);
		if (token != STRING) {
			printf("Expecting media options. \r\n");
			skip_to_semi (cfile);
			return;
		}

		tmp = (struct string_list *)malloc(strlen (val) + sizeof(struct string_list));
		if (!tmp)
		{
			printf("no memory for string list entry. \r\n");
            return;
		}
        
		strcpy(tmp->string, val);
		tmp->next = (struct string_list *)0;

		/* Store this medium at the end of the media list. */
		if (cur)
			cur->next = tmp;
		else
			*lp = tmp;
        
		cur = tmp;

		token = next_token (&val, (unsigned *)0, cfile);
	} while (multiple && token == COMMA);

	if (token != SEMI) {
		printf("expecting semicolon. \r\n");
		skip_to_semi (cfile);
	}
}

#if 0
void parse_reject_statement (cfile, config)
	struct parse *cfile;
	struct client_config *config;
{
	int token;
	const char *val;
	struct iaddr addr;
	struct iaddrlist *list;

	do {
		if (!parse_ip_addr (cfile, &addr)) {
			parse_warn (cfile, "expecting IP address.");
			skip_to_semi (cfile);
			return;
		}

		list = (struct iaddrlist *)dmalloc (sizeof (struct iaddrlist),
						    MDL);
		if (!list)
			log_fatal ("no memory for reject list!");

		list -> addr = addr;
		list -> next = config -> reject_list;
		config -> reject_list = list;

		token = next_token (&val, (unsigned *)0, cfile);
	} while (token == COMMA);

	if (token != SEMI) {
		parse_warn (cfile, "expecting semicolon.");
		skip_to_semi (cfile);
	}
}	


/* allow-deny-keyword :== BOOTP
   			| BOOTING
			| DYNAMIC_BOOTP
			| UNKNOWN_CLIENTS */

int parse_allow_deny (oc, cfile, flag)
	struct option_cache **oc;
	struct parse *cfile;
	int flag;
{
	enum dhcp_token token;
	const char *val;
	unsigned char rf = flag;
	struct expression *data = (struct expression *)0;
	int status;

	parse_warn (cfile, "allow/deny/ignore not permitted here.");
	skip_to_semi (cfile);
	return 0;
}
#endif

const int universe_count = 10; 

int option_state_allocate(struct option_state **ptr, const char *file, int line)
{
	unsigned size;

	if (!ptr) 
    {
		printf("%s(%d): null pointer", file, line);
		return 0;
	}
    
	if (*ptr) 
    {
		printf("%s(%d): non-null pointer", file, line);
		*ptr = (struct option_state *)0;
	}

	size = sizeof(**ptr) + (universe_count - 1) * sizeof(void *);
	*ptr = (struct option_state *)malloc(size);
	if (*ptr) 
    {
		memset(*ptr, 0, size);
		(*ptr)->universe_count = universe_count;
		(*ptr)->refcnt = 1;

        return 1;
	}
    
	return 0;
}


void parse_client_lease_statement(struct parse *cfile, int is_static)
{
	struct client_lease *lease, *lp, *pl, *next;
	struct interface_info *ip = (struct interface_info *)0;
	enum dhcp_token token;
	const char *val;
	struct client_state *client = (struct client_state *)0;

	token = next_token(&val, (unsigned *)0, cfile);

    printf("token: %d %s %d\r\n", token, __FUNCTION__, __LINE__);
    
	if (token != LBRACE) 
    {
		printf("expecting left brace. \r\n");
		skip_to_semi(cfile);
		return;
	}

	lease = (struct client_lease *)malloc(sizeof(struct client_lease));
	if (!lease) {
		printf("no memory for lease. %s %d\r\n", __FUNCTION__, __LINE__);
        return;
	}
	
	memset(lease, 0, sizeof(*lease));
	lease->is_static = is_static;
    
	if (!option_state_allocate(&lease->options, MDL))
		printf("no memory for lease options.\n");

	do 
    {
		token = peek_token(&val, (unsigned *)0, cfile);
        
        printf("peek_token token: %d %s %d\r\n", token, __FUNCTION__, __LINE__);
        
        if (token == END_OF_FILE) 
        {
			printf("unterminated lease declaration. \r\n");
			return;
		}

        if (token == RBRACE)
        {
			break;
        }

        parse_client_lease_declaration(cfile, lease, &ip, &client);

    } while (1);
    
	token = next_token(&val, (unsigned *)0, cfile);

	/* If the lease declaration didn't include an interface
	   declaration that we recognized, it's of no use to us. */
	if (!ip) {
		destroy_client_lease(lease);
		return;
	}

	/* Make sure there's a client state structure... */
	if (!ip -> client) {
		make_client_state (&ip -> client);
		ip -> client -> interface = ip;
	}
	if (!client)
		client = ip -> client;

	/* If this is an alias lease, it doesn't need to be sorted in. */
	if (is_static == 2) {
		ip -> client->alias = lease;
		return;
	}

	/* The new lease may supersede a lease that's not the
	   active lease but is still on the lease list, so scan the
	   lease list looking for a lease with the same address, and
	   if we find it, toss it. */
	pl = (struct client_lease *)0;
	for (lp = client->leases; lp; lp = next) {
		next = lp->next;
		if (lp->address.len == lease->address.len &&
		    !memcmp (lp->address.iabuf, lease->address.iabuf, lease->address.len)) 
	    {
			if (pl)
				pl -> next = next;
			else
				client->leases = next;
			destroy_client_lease (lp);
			break;
		} else
			pl = lp;
	}

	/* If this is a preloaded lease, just put it on the list of recorded
	   leases - don't make it the active lease. */
	if (is_static) {
		lease->next = client->leases;
		client->leases = lease;
		return;
	}
		
	/* The last lease in the lease file on a particular interface is
	   the active lease for that interface.    Of course, we don't know
	   what the last lease in the file is until we've parsed the whole
	   file, so at this point, we assume that the lease we just parsed
	   is the active lease for its interface.   If there's already
	   an active lease for the interface, and this lease is for the same
	   ip address, then we just toss the old active lease and replace
	   it with this one.   If this lease is for a different address,
	   then if the old active lease has expired, we dump it; if not,
	   we put it on the list of leases for this interface which are
	   still valid but no longer active. */
	if (client->active) 
    {
		if (client->active->expiry < cur_time)
			destroy_client_lease(client->active);
		else if (client->active->address.len == lease->address.len &&
			 !memcmp (client->active->address.iabuf,
				  lease->address.iabuf,
				  lease->address.len))
			destroy_client_lease(client->active);
		else {
			client->active->next = client->leases;
			client->leases = client->active;
		}
	}
    
	client->active = lease;

	/* phew. */
}
#endif



void read_client_lease()
{
    int file;
    struct parse *cfile;
    const char *val;
    int token;

    cur_time = time(NULL);
    
    if ((file = open(path_dhclient_db, O_RDONLY)) < 0)
    {
		return;
    }
    
    cfile = (struct parse *)0;
    new_parse(&cfile, file, (char *)0, 0, path_dhclient_db, 0);

    do 
    {
        token = next_token(&val, (unsigned *)0, cfile);

        printf("token: %d %s %d\r\n", token, __FUNCTION__, __LINE__);
        
        if (token == END_OF_FILE) 
        {
            break;
        }

        if (token != LEASE) 
        {
            printf("Corrupt lease file - possible data loss! \r\n");
            skip_to_semi(cfile);
            break;
        }
        else 
        {
            parse_client_lease_statement(cfile, 0);
        }

    } while (1);

    close(file);

    end_parse(&cfile);
}

