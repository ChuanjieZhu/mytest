
#ifndef CLIENT_33310_H
#define CLIENT_33310_H

#ifdef WIN32    
    #pragram pack(1)    
    #define PACK_ALIGN
#else    
    #define PACK_ALIGN __attribute__((packed))
#endif

#define MDL __FUNCTION__, __LINE__

/* 包类型 */
#define     AUTH                0	
#define     GET					1
#define     GET_NEXT			2
#define     PUT					3
#define     PUT_NEXT			4
#define     RESPONSE			5
#define     TRAP                6
#define     HEARTBEAT			7

/* RESPONSE 子类型 */
#define     ACK					0
#define     NAK					1

/* HEARTBEAT 子类型 */
#define     HEART_BEAT          2

/* GET/PUT 子类型 */
#define     SYS_NET             3               /* 网络 */        
#define     SYS_DEV_INFO        4               /* 设备信息 */
#define     SYS_DEV_NAME        5
#define     SYS_VERSION         6               /* 获取终端版本 */
#define     SYS_NET_DNS         7               /* 设置设备DNS */
#define     GET_DEV_LOG         8               /* 获取设备LOG */
#define     SYS_SERVER_IP_PORT  9

#define DATA_MAX_LEN		(33 * 1024)

#define PROTOCOL_HEAD_FLAG 0x6666
#define PROTOCOL_HEAD_INDEX 0x0001


typedef struct PROTOCOL_HEAD {    
    unsigned short flag;    
    unsigned short index;    
    char packType;    
    char subType;    
    unsigned int dataLen;
} PACK_ALIGN PROTOCOL_HEAD, *LPPROTOCOL_HEAD;

typedef struct PROTOCOL_PACK {    
    PROTOCOL_HEAD head;    
    char data[1];
} PACK_ALIGN PROTOCOL_PACK, *LPPROTOCOL_PACK;

typedef struct GET_LOGGER {
    char version[8];
    char sn[16];
    char time[32];
    int loggerLen;
} PACK_ALIGN GET_LOGGER, *LPGET_LOGGER;

typedef struct GET_VERSION {
    char version[16];
} PACK_ALIGN GET_VERSION, *LPGET_VERSION;

typedef struct SET_DNS {
    char dns1[16];
    char dns2[16];
} PACK_ALIGN SET_DNS, *LPSET_DNS;

typedef struct SET_AUTH_SERVER_PORT {
    char ip[16];
    int port;
} PACK_ALIGN SET_AUTH_SERVER_PORT, *LPSET_AUTH_SERVER_PORT;

#endif /* CLIENT_33310_H */
