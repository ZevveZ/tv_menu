#ifndef M_CURSES_H 
#define M_CURSES_H

#define MENU_MAX_TITLE 64
#define MENU_MAX_STATUS 64

#include<string.h>

struct menu_item{
	unsigned long device_id;	//设备id，根据设备id绑定对应的信号处理函数和刷新函数
	char title[MENU_MAX_TITLE];
	char status[MENU_MAX_STATUS];
	struct menu *sub_menu;
	void (*phandler)(const char signal, struct menu_item *menu_item);
	void (*pshow)(struct menu_item *menu_item);

};

typedef void handler(const char signal, struct menu_item *menu_item);
typedef void show(struct menu_item *menu_item);

struct menu{
	char title[64];
	int device_id;
	struct menu_item **menu_item;
	int item_cnt;
	int pre_menu_id;
	int current_menu_item_id;
};



#endif
