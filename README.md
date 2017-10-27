# 概览
tv_menu/
├── cJSON.c	     //JSON格式的菜单配置
├── cJSON.h
├── curses.c         //UI使用curses图形库
├── ir_decode.c      //手动解析NEC协议
├── ir_decode.h
├── led.c            //RGB灯控制
├── led.h
├── Makefile         //各功能模块独立c/h文件，支持Makefile编译
├── m_curses.h
├── servo.c          //舵机控制
└── servo.h

# JSON配置文件示例
```
{
    "menu": [
        {
            "title": "FACTORY MENU",
            "menu_item": [
                {
                    "device_id": 1,
                    "title": "LED-R",
                    "status": "FALSE"
                },
                {
                    "device_id": 0,
                    "title": "SUB MENU",
                    "status": "1"
                }
            ]
        },
        {
            "title": "SUB MENU",
            "menu_item": [
                {
                    "device_id": 3,
                    "title": "sub-item1",
                    "status": "FALSE"
                }
              ]
          }
     ]
}
```
# 与菜单有关的结构体
```
struct menu_item{                                                                                                                      
        unsigned long device_id;        //设备id，根据设备id绑定对应的信号处理函数和刷新函数                                           
        char title[MENU_MAX_TITLE];                                                                                                    
        char status[MENU_MAX_STATUS];                                                                                                  
        struct menu *sub_menu;         //指向子菜单，实现菜单跳转                                                                                                
        void (*phandler)(const char signal, struct menu_item *menu_item);     //信号处理函数                                                         
        void (*pshow)(struct menu_item *menu_item);    //刷新函数                                                                                                          
};                                                                                                                                     
                                                                                                                                       
struct menu{                                                                                                                           
        char title[MENU_MAX_TITLE];                                                                                                                                                                                                                               
        struct menu_item **menu_item;    //菜单项列表                                                                                              
        int item_cnt;                                                                                                                  
        int pre_menu_id;                             //指向上一个菜单，实现菜单跳转                                                                                  
        int current_menu_item_id;             //保存当前菜单选中的菜单项                                                                                         
};
```
# 与NEC协议解析相关
```
static void ir_int(void){
    int byt=0;
    int i,j;
    int tmp = read_low();//通过轮询获取低电平时间
    //判断9ms的引导码
    if(tmp<8500||tmp>9500){
        return;
    }
	    tmp=read_high();
    //判断4.5ms的引导码
    if(tmp<4000||tmp>5000){
        return;
    }
    for(i=0;i<4;++i){	//读取4个字节
        for(j=0;j<8;++j){	//读取每个字节的8个位
            tmp=read_low();
            if(tmp<340||tmp>780){
                return;
            }

            tmp=read_high();
            if(tmp>340&&tmp<780){
                byt>>=1;	//读取到0
            }else if(tmp>1460&&tmp<1900){
                byt>>=1;	//读取到1
                byt|=0x80;
            }else{
                return;
            }
        }
        record[i]=byt;
        byt=0;
    }
    irflag=1;	//收到信号标志位
}
```
