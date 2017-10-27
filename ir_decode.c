#include <stdio.h>
#include <wiringPi.h>
#include <sys/time.h>

#define IR_INPUT 4

static struct timeval tv_last_end, tv_present_start; 
static int repeat_cnt;
char record[4];
int irflag;

static int read_low(void){
    struct timeval tv_start,tv_end, tv_res;

    if(gettimeofday(&tv_start, NULL)==-1){
        perror("gettimeofday");
        return -1;
    }
    while(!digitalRead(IR_INPUT));
    if(gettimeofday(&tv_end, NULL)==-1){
        perror("gettimeofday");
        return -1;
    }

    timersub(&tv_end, &tv_start, &tv_res);

    return tv_res.tv_usec;
}

static int read_high(void){
    struct timeval tv_start,tv_end, tv_res;

    if(gettimeofday(&tv_start, NULL)==-1){
        perror("gettimeofday");
        return -1;
    }
    while(digitalRead(IR_INPUT));
    if(gettimeofday(&tv_end, NULL)==-1){
        perror("gettimeofday");
        return -1;
    }

    timersub(&tv_end, &tv_start, &tv_res);

    return tv_res.tv_usec;
}

static void ir_int(void){
    gettimeofday(&tv_present_start, NULL);

    int byt=0;
    int i,j;
    int tmp = read_low();
    if(tmp<8500||tmp>9500){
        return;
    }

    tmp=read_high();

    if(tmp>1750&&tmp<2750){
        struct timeval tv_tmp;
        timersub(&tv_present_start, &tv_last_end, &tv_tmp);
        tv_last_end=tv_present_start;
        if(tv_tmp.tv_usec>44500&&tv_tmp.tv_usec<45500 || tv_tmp.tv_usec>107500||tv_tmp.tv_usec>107500&&tv_tmp.tv_usec<108500){
	    if(++repeat_cnt==2){
	    	irflag=1;
		repeat_cnt=0;
	    }
            return;
        }
    }

    if(tmp<4000||tmp>5000){
        return;
    }

    for(i=0;i<4;++i){
        for(j=0;j<8;++j){
            tmp=read_low();
            if(tmp<340||tmp>780){
                return;
            }

            tmp=read_high();
            if(tmp>340&&tmp<780){
                byt>>=1;
            }else if(tmp>1460&&tmp<1900){
                byt>>=1;
                byt|=0x80;
            }else{
                return;
            }
        }
        record[i]=byt;
        byt=0;
    }

    irflag=1;
    repeat_cnt=0;
    gettimeofday(&tv_last_end, NULL);
}

void ir_config(void){
	if(wiringPiISR(IR_INPUT, INT_EDGE_FALLING, &ir_int)<0){
		perror("wiringPiISR");
   	}
}

