#include "servo.h"
#include <wiringPi.h>
#include <string.h>
#include "ir_decode.h"

#define SERVO_OUT 5

static int status;

void servo_config(void){
    pinMode(SERVO_OUT, OUTPUT);
}

void servo_level(int level){
    status=level;
    int high_us=500+level*200;
    int i;

    //make sure servo to turn properly
    for(i=0;i<10;++i){
        digitalWrite(SERVO_OUT, HIGH);
        delayMicroseconds(high_us);
        digitalWrite(SERVO_OUT,LOW);
        delayMicroseconds(20000-high_us);
    }
}

void handler_servo(const char signal, struct menu_item *menu_item){
	if(signal==IRKEY_UPDATE){
		status=atoi(menu_item->status);
	}else if(signal == IRKEY_LEFT){
		status=--status<0?0:status;
		menu_item->status[0]='0'+status;
		menu_item->status[1]='\0';
	}else if(signal == IRKEY_RIGHT){
		status=++status>9?9:status;
		menu_item->status[0]='0'+status;
		menu_item->status[1]='\0';
	}else{
		return;
	}
	
	servo_level(status);
}

void show_servo(struct menu_item *menu_item){
	menu_item->status[0]='0'+status;
	menu_item->status[1]='\0';
}

int  servo_get_level(void){
	return status;
}
