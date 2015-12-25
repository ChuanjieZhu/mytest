
#ifndef _CL_PARSE_H_
#define _CL_PARSE_H_

#ifndef _PATH_DHCLIENT_DB
#define _PATH_DHCLIENT_DB	"/var/dhcp/dhclient.leases"
#endif

#include <sys/types.h>
#include <net/if.h>
#include <arpa/inet.h>

#define MAX_TIME 0x7fffffff
#define MIN_TIME 0
#define EOL	'\n'
#define MDL __FILE__, __LINE__

enum dhcp_token 
{
	SEMI = ';',
	DOT = '.',
	COLON = ':',
	COMMA = ',',
	SLASH = '/',
	LBRACE = '{',
	RBRACE = '}',
	LPAREN = '(',
	RPAREN = ')',
	EQUAL = '=',
	BANG = '!',
	PERCENT = '%',
 	PLUS = '+',
	MINUS = '-',
	ASTERISK = '*',
	AMPERSAND = '&',
	PIPE = '|',
	CARET = '^',

	HOST = 256,
	FIRST_TOKEN = HOST,
	HARDWARE = 257,
	FILENAME = 258,
	FIXED_ADDR = 259,
	OPTION = 260,
	ETHERNET = 261,
	STRING = 262,
	NUMBER = 263,
	NUMBER_OR_NAME = 264,
	NAME = 265,
	TIMESTAMP = 266,
	STARTS = 267,
	ENDS = 268,
	UID = 269,
	CLASS = 270,
	LEASE = 271,
	RANGE = 272,
	PACKET = 273,
	CIADDR = 274,
	YIADDR = 275,
	SIADDR = 276,
	GIADDR = 277,
	SUBNET = 278,
	NETMASK = 279,
	DEFAULT_LEASE_TIME = 280,
	MAX_LEASE_TIME = 281,
	VENDOR_CLASS = 282,
	USER_CLASS = 283,
	SHARED_NETWORK = 284,
	SERVER_NAME = 285,
	DYNAMIC_BOOTP = 286,
	SERVER_IDENTIFIER = 287,
	DYNAMIC_BOOTP_LEASE_CUTOFF = 288,
	DYNAMIC_BOOTP_LEASE_LENGTH = 289,
	BOOT_UNKNOWN_CLIENTS = 290,
	NEXT_SERVER = 291,
	TOKEN_RING = 292,
	GROUP = 293,
	ONE_LEASE_PER_CLIENT = 294,
	GET_LEASE_HOSTNAMES = 295,
	USE_HOST_DECL_NAMES = 296,
	SEND = 297,
	CLIENT_IDENTIFIER = 298,
	REQUEST = 299,
	REQUIRE = 300,
	TIMEOUT = 301,
	RETRY = 302,
	SELECT_TIMEOUT = 303,
	SCRIPT = 304,
	INTERFACE = 305,
	RENEW = 306,
	REBIND = 307,
	EXPIRE = 308,
	UNKNOWN_CLIENTS = 309,
	ALLOW = 310,
	DENY = 312,
	BOOTING = 313,
	DEFAULT = 314,
	MEDIA = 315,
	MEDIUM = 316,
	ALIAS = 317,
	REBOOT = 318,
	TOKEN_ABANDONED = 319,
	BACKOFF_CUTOFF = 320,
	INITIAL_INTERVAL = 321,
	NAMESERVER = 322,
	DOMAIN = 323,
	SEARCH = 324,
	SUPERSEDE = 325,
	APPEND = 326,
	PREPEND = 327,
	HOSTNAME = 328,
	CLIENT_HOSTNAME = 329,
	REJECT = 330,
	USE_LEASE_ADDR_FOR_DEFAULT_ROUTE = 331,
	MIN_LEASE_TIME = 332,
	MIN_SECS = 333,
	AND = 334,
	OR = 335,
	SUBSTRING = 337,
	SUFFIX = 338,
	CHECK = 339,
	EXTRACT_INT = 340,
	IF = 341,
	TOKEN_ADD = 342,
	BREAK = 343,
	ELSE = 344,
	ELSIF = 345,
	SUBCLASS = 346,
	MATCH = 347,
	SPAWN = 348,
	WITH = 349,
	EXISTS = 350,
	POOL = 351,
	UNKNOWN = 352,
	CLIENTS = 353,
	KNOWN = 354,
	AUTHENTICATED = 355,
	UNAUTHENTICATED = 356,
	ALL = 357,
	DYNAMIC = 358,
	MEMBERS = 359,
	OF = 360,
	PSEUDO = 361,
	LIMIT = 362,
	BILLING = 363,
	PEER = 364,
	FAILOVER = 365,
	MY = 366,
	PARTNER = 367,
	PRIMARY = 368,
	SECONDARY = 369,
	IDENTIFIER = 370,
	PORT = 371,
	MAX_TRANSMIT_IDLE = 372,
	MAX_RESPONSE_DELAY = 373,
	PARTNER_DOWN = 374,
	NORMAL = 375,
	COMMUNICATIONS_INTERRUPTED = 376,
	POTENTIAL_CONFLICT = 377,
	RECOVER = 378,
	FDDI = 379,
	AUTHORITATIVE = 380,
	TOKEN_NOT = 381,
	AUTHENTICATION = 383,
	IGNORE = 384,
	ACCEPT = 385,
	PREFER = 386,
	DONT = 387,
	CODE = 388,
	ARRAY = 389,
	BOOLEAN = 390,
	INTEGER = 391,
	SIGNED = 392,
	UNSIGNED = 393,
	IP_ADDRESS = 394,
	TEXT = 395,
	STRING_TOKEN = 396,
	SPACE = 397,
	CONCAT = 398,
	ENCODE_INT = 399,
	REVERSE = 402,
	LEASED_ADDRESS = 403,
	BINARY_TO_ASCII = 404,
	PICK = 405,
	CONFIG_OPTION = 406,
	HOST_DECL_NAME = 407,
	ON = 408,
	EXPIRY = 409,
	RELEASE = 410,
	COMMIT = 411,
	DNS_UPDATE = 412,
	LEASE_TIME = 413,
	STATIC = 414,
	NEVER = 415,
	INFINITE = 416,
	TOKEN_DELETED = 417,
	UPDATED_DNS_RR = 418,
	DNS_DELETE = 419,
	DUPLICATES = 420,
	DECLINES = 421,
	TSTP = 422,
	TSFP = 423,
	OWNER = 424,
	IS = 425,
	HBA = 426,
	MAX_UNACKED_UPDATES = 427,
	MCLT = 428,
	SPLIT = 429,
	AT = 430,
	NO = 431,
	TOKEN_DELETE = 432,
	NS_UPDATE = 433,
	UPDATE = 434,
	SWITCH = 435,
	CASE = 436,
	NS_FORMERR = 437,
	NS_NOERROR = 438,
	NS_NOTAUTH = 439,
	NS_NOTIMP = 440,
	NS_NOTZONE = 441,
	NS_NXDOMAIN = 442,
	NS_NXRRSET = 443,
	NS_REFUSED = 444,
	NS_SERVFAIL = 445,
	NS_YXDOMAIN = 446,
	NS_YXRRSET = 447,
	TOKEN_NULL = 448,
	TOKEN_SET = 449,
	DEFINED = 450,
	UNSET = 451,
	EVAL = 452,
	LET = 453,
	FUNCTION = 454,
	DEFINE = 455,
	ZONE = 456,
	KEY = 457,
	SECRET = 458,
	ALGORITHM = 459,
	LOAD = 460,
	BALANCE = 461,
	TOKEN_MAX = 462,
	SECONDS = 463,
	ADDRESS = 464,
	RESOLUTION_INTERRUPTED = 465,
	STATE = 466,
	UNKNOWN_STATE = 567,
	CLTT = 568,
	INCLUDE = 569,
	BINDING = 570,
	TOKEN_FREE = 571,
	TOKEN_ACTIVE = 572,
	TOKEN_EXPIRED = 573,
	TOKEN_RELEASED = 574,
	TOKEN_RESET = 575,
	TOKEN_BACKUP = 576,
	TOKEN_RESERVED = 577,
	TOKEN_BOOTP = 578,
	TOKEN_NEXT = 579,
	OMAPI = 580,
	LOG = 581,
	FATAL = 582,
	ERROR = 583,
	TOKEN_DEBUG = 584,
	INFO = 585,
	RETURN = 586,
	PAUSED = 587,
	RECOVER_DONE = 588,
	SHUTDOWN = 589,
	STARTUP = 590,
	ENCAPSULATE = 591,
	VENDOR = 592,
	CLIENT_STATE = 593,
	INIT_REBOOT = 594,
	TOKEN_INIT = 595,
	SELECT = 596,
	BOUND = 597,
	RENEWING = 598,
	REBINDING = 599,
	RECONTACT_INTERVAL = 600,
	CLIENT_UPDATES = 601,
	TOKEN_NEW = 601,
	TRANSMISSION = 602,
	TOKEN_CLOSE = 603,
	TOKEN_CREATE = 604,
	TOKEN_OPEN = 605,
	TOKEN_HELP = 606,
	END_OF_FILE = 607,
	RECOVER_WAIT = 608,
	SERVER = 609,
	CONNECT = 610,
	REMOVE = 611,
	REFRESH = 612,
	DOMAIN_NAME = 613,
	DO_FORWARD_UPDATE = 614,
	KNOWN_CLIENTS = 615
};

#define is_identifier(x)	((x) >= FIRST_TOKEN &&	\
				 (x) != STRING &&	\
				 (x) != NUMBER &&	\
				 (x) != END_OF_FILE)

/* A pair of pointers, suitable for making a linked list. */
typedef struct _pair 
{
	caddr_t car;
	struct _pair *cdr;
} *pair;


struct parse 
{
	int lexline;
	int lexchar;
	char *token_line;
	char *prev_line;
	char *cur_line;
	const char *tlname;
	int eol_token;

	char line1[81];
	char line2[81];
    char unused[2];
	int lpos;
	int line;
	int tlpos;
	int tline;
	enum dhcp_token token;
	int ugflag;
	char *tval;
	int tlen;
	char tokbuf[1500];

#ifdef OLD_LEXER
	char comments [4096];
	int comment_index;
#endif
	int warnings_occurred;
	int file;
	char *inbuf;
	unsigned bufix;
    unsigned buflen;
	unsigned bufsiz;
};

struct iaddr {
	unsigned len;
	unsigned char iabuf[16];
};

struct string_list {
	struct string_list *next;
	char string[1];
    char unused[3];
};


struct option_state 
{
	int refcnt;
	int universe_count;
	int site_universe;
	int site_code_min;
	void *universes[1];
};


struct hardware 
{
	u_int8_t hlen;
	u_int8_t hbuf[17];
    u_int8_t unused[2];
};

struct interface_info 
{
	struct interface_info *next;	/* Next interface in list... */
	struct hardware hw_address;	    /* Its physical address. */
	struct in_addr primary_address;	/* Primary interface address. */

	u_int8_t *circuit_id;		/* Circuit ID associated with this
					   interface. */
	unsigned circuit_id_len;	/* Length of Circuit ID, if there
					   is one. */
	u_int8_t *remote_id;		/* Remote ID associated with this
					   interface (if any). */
	unsigned remote_id_len;		/* Length of Remote ID. */

	char name[IFNAMSIZ];		/* Its name... */
	int index;			/* Its index. */
	int rfdesc;			/* Its read file descriptor. */
	int wfdesc;			/* Its write file descriptor, if
					   different. */
	unsigned char *rbuf;		/* Read buffer, if required. */
	unsigned int rbuf_max;		/* Size of read buffer. */
	size_t rbuf_offset;		/* Current offset into buffer. */
	size_t rbuf_len;		/* Length of data in buffer. */

	struct ifreq *ifp;		/* Pointer to ifreq struct. */
	u_int32_t flags;		/* Control flags... */
#define INTERFACE_REQUESTED 1
#define INTERFACE_AUTOMATIC 2
#define INTERFACE_RUNNING 4

	/* Only used by DHCP client code. */
	struct client_state *client;
};

/* Per-interface state used in the dhcp client... */
struct client_state 
{
	struct client_state *next;
	struct interface_info *interface;
	char *name;

	struct client_lease *active;		  /* Currently active lease. */
	struct client_lease *new;			       /* New lease. */
	struct client_lease *offered_leases;	    /* Leases offered to us. */
	struct client_lease *leases;		/* Leases we currently hold. */
	struct client_lease *alias;			     /* Alias lease. */

	//enum dhcp_state state;		/* Current state for this interface. */
	struct iaddr destination;		    /* Where to send packet. */
	u_int32_t xid;					  /* Transaction ID. */
	u_int16_t secs;			    /* secs value from DHCPDISCOVER. */
	time_t first_sending;			/* When was first copy sent? */
	time_t interval;		      /* What's the current resend interval? */
	int dns_update_timeout;		 /* Last timeout set for DNS update. */
	struct string_list *medium;		   /* Last media type tried. */
	//struct dhcp_packet packet;		    /* Outgoing DHCP packet. */
	unsigned packet_length;	       /* Actual length of generated packet. */

	struct iaddr requested_address;	    /* Address we would like to get. */
	struct string_list *env;	       /* Client script environment. */
	int envc;			/* Number of entries in environment. */

	struct option_state *sent_options;	/* Options we sent. */
};

/* DHCP client lease structure... */
struct client_lease 
{
	struct client_lease *next;		      /* Next lease in list. */
	time_t expiry, renewal, rebind;			  /* Lease timeouts. */
	struct iaddr address;			    /* Address being leased. */
	char *server_name;			     /* Name of boot server. */
	char *filename;		     /* Name of file we're supposed to boot. */
	struct string_list *medium;			  /* Network medium. */
	//struct auth_key *key;      /* Key used in basic DHCP authentication. */

	unsigned int is_static : 1;    /* If set, lease is from config file. */
	unsigned int is_bootp: 1;  /* If set, lease was acquired with BOOTP. */

	struct option_state *options;	     /* Options supplied with lease. */
};



void read_client_lease();

#endif /* _CL_PARSE_H_ */

