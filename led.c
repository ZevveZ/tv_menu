#include <stdio.h>
#include <wiringPi.h>
#include "ir_decode.h"
#include "led.h"

#define BLUE 27
#define GREEN 28
#define RED 29

static char status[3];

void led_config(void){
    pinMode(BLUE,OUTPUT);
    pinMode(GREEN,OUTPUT);
    pinMode(RED,OUTPUT);
}

void led_dark(void){
	digitalWrite(BLUE, LOW);
	digitalWrite(GREEN, LOW);
	digitalWrite(RED, LOW);
}

void handler_led(const char signal, struct menu_item *menu_item){
	int update_light_id;
	char negative;

	if(signal==IRKEY_UPDATE){
		negative=0;
	}else if(signal == IRKEY_LEFT||signal == IRKEY_RIGHT){
		negative=1;
	}else{
		return;
	}

	if(strcmp(menu_item->title, "LED-R")==0){
		update_light_id=2;
	}else if(strcmp(menu_item->title, "LED-G")==0){
		update_light_id=1;
	}else{
		update_light_id=0;
	}
	
	if(negative){
		status[update_light_id]=!status[update_light_id];
		if(status[update_light_id]){
			strcpy(menu_item->status,"TRUE");
		}else{
			strcpy(menu_item->status, "FALSE");
		}
	}else{
		if(strcmp(menu_item->status, "TRUE")==0){
			status[update_light_id]=1;
		}else{
			status[update_light_id]=0;
		}
	}
	digitalWrite(BLUE+update_light_id, status[update_light_id]?HIGH:LOW);
}
