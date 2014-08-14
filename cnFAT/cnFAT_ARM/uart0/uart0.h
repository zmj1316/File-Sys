#ifndef __UART_H
#define __UART_H

extern void uart0Init (void);
extern void uart0ByteSend (unsigned char ucData);
extern void uart0StrSend (char *str);

#endif

