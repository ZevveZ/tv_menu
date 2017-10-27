#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"
#include "ir_decode.h"
#include "m_curses.h"
#include "led.h"
#include "servo.h"

struct menu **menu_arr;
int current_menu_id;
static int menu_cnt;

static char *deconf="{\"menu\":[{\"title\":\"FACTORY MENU\",\"menu_item\":[{\"device_id\":1,\"title\":\"LED-R\",\"status\":\"FALSE\"},{\"device_id\":1,\"title\":\"LED-G\",\"status\":\"FALSE\"},{\"device_id\":1,\"title\":\"LED-B\",\"status\":\"FALSE\"},{\"device_id\":2,\"title\":\"Steering level\",\"status\":\"0\"},{\"device_id\":0,\"title\":\"SUB MENU\",\"status\":\"1\"}]},{\"title\":\"SUB MENU\",\"menu_item\":[{\"device_id\":3,\"title\":\"sub-item1\",\"status\":\"FALSE\"},{\"device_id\":3,\"title\":\"sub-item2\",\"status\":\"TRUE\"}]}]}";

void curses_config(void){
	initscr();	//保存当前终端的配置，并建立curses终端
	noecho();
	intrflush(stdscr,FALSE);
	refresh();	//clear screen
	curs_set(0);
/*
	if(has_colors()&&start_color()==0){
		init_pair(1,COLOR_WHITE, COLOR_BLUE);
		attron(COLOR_PAIR(1));
	}
*/
}


static void show_menu(){
	int i;
	int x,y;
	if(current_menu_id<0){
		clear();
		refresh();
		return;
	}

	struct menu *menu=menu_arr[current_menu_id];

	clear();
/*	move(0,0);

	for(i=0;i<134;++i){
		addch(' ');
	}
*/
	mvaddstr(0,60,menu->title);

	move(1,0);
	printw("<exit> return/exit\t<menu> show menu\t");
	getyx(stdscr,y,x);

	move(2,0);
	for(i=0;i<134;++i){
		addch('=');
	}
/*
	++y;
	while(--x>=0){
		mvaddch(y,x,'=');
	}
*/
	y=2;
	for(i=0;i<menu->item_cnt;++i){
		if(menu->menu_item[i]->pshow){
			menu->menu_item[i]->pshow(menu->menu_item[i]);
		}
		mvprintw(++y,0, "%c %2d %64s %64s", i==menu->current_menu_item_id?'*':' ', i+1, menu->menu_item[i]->title,menu->menu_item[i]->device_id==0?">>":menu->menu_item[i]->status);
	}
	refresh();

}


static void handler_submenu(const char signal, struct menu_item *menu_item){

	int next_menu_id=atoi(menu_item->status);
	switch(signal){
		case IRKEY_RIGHT:
			if(menu_arr[next_menu_id]->pre_menu_id<0)
				menu_arr[next_menu_id]->pre_menu_id=current_menu_id;
			current_menu_id=next_menu_id;
			menu_arr[current_menu_id]->current_menu_item_id=0;
			break;
		default:
			break;
	}
}

handler *handler_arr[]={handler_submenu,handler_led,handler_servo,NULL};
show *show_arr[]={NULL,NULL,show_servo};

static void read_menuconf(void){
	FILE *fp;
	long len;
	char *data;
	cJSON *root;

	if((fp=fopen("menuconf.json","rb"))==NULL){
		data=deconf;
	}else{
		fseek(fp, 0, SEEK_END);
		len=ftell(fp);
		data=malloc(len+1);
		rewind(fp);
		if(fread(data,1,len,fp)!=len){
			perror("fread");
			goto end;
		}
		fclose(fp);
	}
	root=cJSON_Parse(data);
	if(!root){
		fputs("cJSON_Paese failed\n",stderr);
		goto end;	
	}
	cJSON *json_menu_arr=cJSON_GetObjectItem(root, "menu");
	menu_cnt=cJSON_GetArraySize(json_menu_arr);
	menu_arr = malloc(menu_cnt*sizeof(struct menu*));

	int i;
	cJSON *json_menu_arr_fence;
	for(i=0;i<menu_cnt;++i){
		menu_arr[i]=malloc(sizeof(struct menu));
		json_menu_arr_fence=cJSON_GetArrayItem(json_menu_arr,i);
		strncpy(menu_arr[i]->title,cJSON_GetObjectItem(json_menu_arr_fence,"title")->valuestring, 64);
		menu_arr[i]->pre_menu_id=-1;
		menu_arr[i]->current_menu_item_id=0;
		cJSON *json_menu_item_arr=cJSON_GetObjectItem(json_menu_arr_fence,"menu_item");
		int menu_item_cnt=menu_arr[i]->item_cnt=cJSON_GetArraySize(json_menu_item_arr);

		menu_arr[i]->menu_item=malloc(sizeof(struct menu_item*)*menu_item_cnt);

		int j;
		cJSON *json_menu_item_arr_fence;
		char *tmp[64];
		for(j=0;j<menu_item_cnt;++j){
			menu_arr[i]->menu_item[j]=malloc(sizeof(struct menu_item));
			json_menu_item_arr_fence=cJSON_GetArrayItem(json_menu_item_arr, j);
			menu_arr[i]->menu_item[j]->device_id=cJSON_GetObjectItem(json_menu_item_arr_fence,"device_id")->valueint;
			strncpy(menu_arr[i]->menu_item[j]->title,cJSON_GetObjectItem(json_menu_item_arr_fence,"title")->valuestring,64);
			strncpy(menu_arr[i]->menu_item[j]->status,cJSON_GetObjectItem(json_menu_item_arr_fence,"status")->valuestring,64);
			menu_arr[i]->menu_item[j]->phandler=handler_arr[menu_arr[i]->menu_item[j]->device_id];
			menu_arr[i]->menu_item[j]->pshow=show_arr[menu_arr[i]->menu_item[j]->device_id];
			if(menu_arr[i]->menu_item[j]->phandler)
				menu_arr[i]->menu_item[j]->phandler(IRKEY_UPDATE, menu_arr[i]->menu_item[j]);
		}
	}

	cJSON_Delete(root);

end:
	if(data!=deconf){
		free(data);
	}
}

static void write_menuconf(void){
	FILE *fp;
	cJSON *root;
	cJSON *menu;
	char *out;

	fp=fopen("menuconf.json","wb");
	if(fp==NULL){
		perror("fopen");
		return;
	}

	root=cJSON_CreateObject();
	menu=cJSON_CreateArray();
	cJSON_AddItemToObject(root, "menu", menu);

	int i;
	cJSON *json_menu_arr_fence;
	for(i=0;i<menu_cnt;++i){
		json_menu_arr_fence=cJSON_CreateObject();
		cJSON_AddItemToArray(menu, json_menu_arr_fence);
		cJSON_AddItemToObject(json_menu_arr_fence, "title", cJSON_CreateString(menu_arr[i]->title));
		cJSON *json_menu_item_arr=cJSON_CreateArray();
		cJSON_AddItemToObject(json_menu_arr_fence,"menu_item", json_menu_item_arr);
		int j;
		cJSON *json_menu_item_arr_fence;
		for(j=0;j<menu_arr[i]->item_cnt;++j){
			struct menu_item *menu_item_tmp=menu_arr[i]->menu_item[j];
			if(menu_item_tmp->pshow) menu_item_tmp->pshow(menu_item_tmp);

			json_menu_item_arr_fence=cJSON_CreateObject();
			cJSON_AddItemToArray(json_menu_item_arr, json_menu_item_arr_fence);
			cJSON_AddNumberToObject(json_menu_item_arr_fence, "device_id", menu_item_tmp->device_id);
			cJSON_AddItemToObject(json_menu_item_arr_fence, "title", cJSON_CreateString(menu_item_tmp->title));
			cJSON_AddItemToObject(json_menu_item_arr_fence, "status",cJSON_CreateString( menu_item_tmp->status));
		}

	}
	out=cJSON_Print(root);
	int len=strlen(out);
	if(fwrite(out, 1, strlen(out), fp)!=len){
		perror("fwrite");
		return;
	}

	cJSON_Delete(root);
	free(out);

	fclose(fp);
}

void release_menu(void){
	int i;
	for(i=0;i<menu_cnt;++i){
		int j;
		for(j=0;j<menu_arr[i]->item_cnt;++j){
			free(menu_arr[i]->menu_item[j]);
		}
		free(menu_arr[i]->menu_item);
		free(menu_arr[i]);
	}
	free(menu_arr);
}

void curses_exit(void){
	write_menuconf();
	led_dark();
	release_menu();
	endwin();
	exit(0);
}

static void show_quick_menu(void){
	int i;
	int time_left=5;
	int tmp_level=servo_get_level();

	move(0,0);
	for(i=0;i<134;++i){
		addch('=');
	}
	mvprintw(1,60,"LEVEL = %d",tmp_level);
	mvprintw(2,60,"time left = %d(s)",time_left);
	move(3,0);
	for(i=0;i<134;++i){
		addch('=');
	}
	refresh();

	while(time_left){
		for(i=0;i<10;++i){
			if(irflag){
				irflag=0;
				time_left=5;
				if(record[2]==IRKEY_OK){
					tmp_level=(tmp_level+1)%10;
					mvprintw(1,60,"LEVEL = %d", tmp_level);
				}else if (record[2]==IRKEY_POWER){
					curses_exit();
				}
				
				mvprintw(2,60,"time left = %d(s)",time_left);
				refresh();
				i=0;
				continue;
				
			}
			delay(100);
			
		}
		time_left--;
		mvprintw(2,60,"time left = %d(s)",time_left);
		refresh();
	}

	servo_level(tmp_level);
	clear();
	refresh();
	
}

int main(void){
	if(wiringPiSetup()==-1){
		perror("wiringPiSetup");
		return 1;
	}
	ir_config();
	curses_config();
	led_config();
	servo_config();
	read_menuconf();
	current_menu_id=-1;

	while(1){
		if(irflag){
			irflag=0;
			switch(record[2]){
				case IRKEY_MENU:
					if(current_menu_id<0){
						current_menu_id=0;
						menu_arr[current_menu_id]->current_menu_item_id=0;
						show_menu();
					}
					break;
				case IRKEY_1:
					if(current_menu_id>=0){
						current_menu_id=menu_arr[current_menu_id]->pre_menu_id;
						show_menu();
					}
					break;
				case IRKEY_DOWN:
					if(current_menu_id!=-1){
						menu_arr[current_menu_id]->current_menu_item_id=(menu_arr[current_menu_id]->current_menu_item_id+1)%(menu_arr[current_menu_id]->item_cnt);
						show_menu();
					}
					break;
				case IRKEY_UP:
					if(current_menu_id!=-1){
						menu_arr[current_menu_id]->current_menu_item_id=(menu_arr[current_menu_id]->current_menu_item_id-1+menu_arr[current_menu_id]->item_cnt)%menu_arr[current_menu_id]->item_cnt;
						show_menu();
					}
					break;
				case IRKEY_POWER:
					curses_exit();
					break;	
				case IRKEY_OK:
					if(current_menu_id==-1){
						show_quick_menu();
					}
					break;
				default:
					if(menu_arr[current_menu_id]->menu_item[menu_arr[current_menu_id]->current_menu_item_id]->phandler)
						menu_arr[current_menu_id]->menu_item[menu_arr[current_menu_id]->current_menu_item_id]->phandler(record[2],menu_arr[current_menu_id]->menu_item[menu_arr[current_menu_id]->current_menu_item_id]);
					show_menu();
					break;

			}
		}
	}
}

