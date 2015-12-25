
#ifndef __UBLOX_H__
#define __UBLOX_H__

typedef struct uart_param
{
	int speed;
	int databits;
	int stopbits;
	int parity;
	int opostflag;
} uart_param;

#endif //__UBLOX_H__
