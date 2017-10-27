#ifndef IR_DECODE_H
#define IR_DECODE_H

#define IRKEY_MENU 0x46
#define IRKEY_EXIT 0x4A
#define IRKEY_1 0x02
#define IRKEY_LEFT 0x47
#define IRKEY_RIGHT 0x4B
#define IRKEY_UP 0x56
#define IRKEY_DOWN 0x50
#define IRKEY_POWER 0x01
#define IRKEY_OK 0x57

#define IRKEY_UPDATE 0xFF

extern int irflag;
extern char record[4];

void ir_config(void);

#endif
