/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-07-05     RT-Thread    first version
 */

#include <rtthread.h>
#include <rtdbg.h>
#include <rtdevice.h>
#include <board.h>
#include <drv_common.h>
#define DBG_TAG "main"
#define DBG_LVL DBG_LOG

/******线程初始化******/
rt_thread_t Key;
rt_thread_t Key_ZX;
rt_thread_t LS165_read;
rt_thread_t PowerProtect;

rt_device_t u2_dev;
rt_thread_t u2_th;

rt_device_t u4_dev;
rt_thread_t u4_th;

/******定义开始实验&&智能评价按键******/
#define KEY1 GET_PIN(B,5)      //开始实验，进入实验状态
#define KEY2 GET_PIN(E,3)      //开始评价，进入评价状态

/******实验电源******/
#define Power GET_PIN(D,7)     //低电平有效，实验电源上电

/******定义拨码开关输出******/
#define A3 GET_PIN(B,13)
#define A2 GET_PIN(E,14)
#define A1 GET_PIN(E,13)
#define A0 GET_PIN(E,12)
#define B3 GET_PIN(E,11)
#define B2 GET_PIN(E,10)
#define B1 GET_PIN(E,9)
#define B0 GET_PIN(E,8)

/******定义8LED移位寄存器******/
#define CLK1  GET_PIN(B,9)
#define LOAD1 GET_PIN(E,0)
#define DATA1 GET_PIN(E,4)

/******定义拨码开关移位寄存器******/
#define CLK2  GET_PIN(B,2)
#define LOAD2 GET_PIN(E,7)
#define DATA2 GET_PIN(C,8)

/******定义电源保护******/
#define PB14 GET_PIN(B,14)

unsigned char mode = 0;//按键标志位

struct serial_configure u2_configs = RT_SERIAL_CONFIG_DEFAULT; //串口2配置（默认）
struct rt_semaphore sem2;//定义信号量
uint8_t  LEDstate;

struct serial_configure u4_configs = RT_SERIAL_CONFIG_DEFAULT; //串口4配置（默认）
struct rt_semaphore sem4;//定义信号量

/******UART2接收中断回调******/
rt_err_t rx_callback2(rt_device_t dev,rt_size_t size)
{
    rt_sem_release(&sem2); //释放信号量
    return RT_EOK;
}

/******UART4接收中断回调******/
rt_err_t rx_callback4(rt_device_t dev,rt_size_t size)
{
    rt_sem_release(&sem4); //释放信号量
    return RT_EOK;
}

/******智能评价******/
int znpj(void)
{
    int flag=1;
    for(int i=0;i<8;i++)
    {
    int key0=0,key1=0,key2=0;
        key0=i&0x01;
        key1=(i&0x02)/2;
        key2=(i&0x04)/4;
        rt_pin_write(A0,key0);
        rt_pin_write(A1,key1);
        rt_pin_write(A2,key2);
         rt_thread_mdelay(15);
        int led_test;
        led_test = LEDstate & 64;
        if(((key0&&key1)||(key0&&key2)||(key1&&key2)||(key0&&key1&&key2)))
        {
            if(led_test==0)
            flag=0;
        }
        if((key0==0&&key1==0)||(key0==0&&key2==0)||(key0==1&&key2==0)||(key1==0&&key2==0&&key0==0))
        {
        if(led_test==1)
            flag=0;
        }
    }
    return flag;

}

/******初始化引脚模式******/
void pin_init(void)
{
    //初始化按键引脚
    rt_pin_mode(KEY1, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY2, PIN_MODE_INPUT_PULLUP);

    //初始化电源引脚
    rt_pin_mode(Power,PIN_MODE_OUTPUT_OD);

    //初始化拨码开关输出引脚
    rt_pin_mode(A3,PIN_MODE_OUTPUT);
    rt_pin_mode(A2,PIN_MODE_OUTPUT);
    rt_pin_mode(A1,PIN_MODE_OUTPUT);
    rt_pin_mode(A0,PIN_MODE_OUTPUT);
    rt_pin_mode(B3,PIN_MODE_OUTPUT);
    rt_pin_mode(B2,PIN_MODE_OUTPUT);
    rt_pin_mode(B1,PIN_MODE_OUTPUT);
    rt_pin_mode(B0,PIN_MODE_OUTPUT);

    //初始化LED开关移位寄存器引脚
    rt_pin_mode(LOAD1,PIN_MODE_OUTPUT);
    rt_pin_mode(CLK1,PIN_MODE_OUTPUT);
    rt_pin_mode(DATA1,PIN_MODE_INPUT_PULLUP);

    //初始化拨码开关移位寄存器引脚
    rt_pin_mode(LOAD2,PIN_MODE_OUTPUT);
    rt_pin_mode(CLK2,PIN_MODE_OUTPUT);
    rt_pin_mode(DATA2,PIN_MODE_INPUT_PULLUP);

    //初始化电源保护引脚
    rt_pin_mode(PB14,PIN_MODE_INPUT);

}

/******读取HC165的值******/
void readShiftRegister()
{
    uint8_t incoming1 = 0,incoming2 = 0;

        // 拉低LOAD_PIN，将并行数据加载到移位寄存器
        rt_pin_write(LOAD1, PIN_LOW);
        rt_pin_write(LOAD2, PIN_LOW);

        rt_thread_mdelay(1);  // 短暂延时

        rt_pin_write(LOAD1, PIN_HIGH);
        rt_pin_write(LOAD2, PIN_HIGH);

        rt_thread_mdelay(1);  // 短暂延时

        // 依次读取每一位的值
        for (int i = 0; i < 8; i++)
        {
            // 将DATA_PIN上的数据读取并存储到incoming变量中
            incoming1 |= rt_pin_read(DATA1) << (7 - i);
            incoming2 |= rt_pin_read(DATA2) << (7 - i);

            // 产生时钟脉冲，将移位寄存器中的数据依次输出到DATA_PIN
            rt_pin_write(CLK1, PIN_HIGH);
            rt_pin_write(CLK2, PIN_HIGH);

            rt_thread_mdelay(1);  // 短暂延时

            rt_pin_write(CLK1, PIN_LOW);
            rt_pin_write(CLK2, PIN_LOW);

            rt_thread_mdelay(1);  // 短暂延时
        }
        // 打印读取到的值，以十六进制显示
        //rt_kprintf("LEDHC165: %02X\n", incoming1);
        LEDstate = incoming1;
        //rt_kprintf("BOMAHC165: %02X\n", incoming2);
        // 翻转读取到的8位数据
        uint8_t reversedData = ~incoming2;
        rt_kprintf("~BOMAHC165: %02X\n", reversedData);
        // 将翻转后的数据分别赋值给A3，A2，A1，A0，B3，B2，B1，B0
        rt_pin_write(A3, (reversedData & (1 << 7)) ? PIN_HIGH : PIN_LOW);
        rt_pin_write(A2, (reversedData & (1 << 6)) ? PIN_HIGH : PIN_LOW);
        rt_pin_write(A1, (reversedData & (1 << 5)) ? PIN_HIGH : PIN_LOW);
        rt_pin_write(A0, (reversedData & (1 << 4)) ? PIN_HIGH : PIN_LOW);
        rt_pin_write(B3, (reversedData & (1 << 3)) ? PIN_HIGH : PIN_LOW);
        rt_pin_write(B2, (reversedData & (1 << 2)) ? PIN_HIGH : PIN_LOW);
        rt_pin_write(B1, (reversedData & (1 << 1)) ? PIN_HIGH : PIN_LOW);
        rt_pin_write(B0, (reversedData & (1 << 0)) ? PIN_HIGH : PIN_LOW);

}


/******UART2接收入口函数******/
void serial_thread_entry2(void*parameter)
{
    char buffer;
       rt_size_t ret;
       while(1)
       {
           ret = rt_device_read(u2_dev, 0, &buffer, 1);
           if(ret == 1){
               rt_sem_take(&sem2,RT_WAITING_FOREVER);
               rt_kprintf("%c",buffer);
           }

           rt_thread_mdelay(10);
       }
}

/******UART4接收入口函数******/
void serial_thread_entry4(void*parameter)
{

    char buffer;
           rt_size_t ret;
           while(1)
           {
               ret = rt_device_read(u4_dev, 0, &buffer, 1);
               if(ret == 1){
                   rt_sem_take(&sem4,RT_WAITING_FOREVER);
                   rt_kprintf("%c",buffer);
               }

               rt_thread_mdelay(10);
           }



//    char buffer[8];
//    rt_size_t ret;
//    while (1)
//    {
//
//       if (ret == 1)
//       {
//           if (buffer[0]==0x0A && buffer[5]==0xA0)
//           {
//               rt_sem_take(&sem4, RT_WAITING_FOREVER);
//               rt_kprintf("ESP32....");
//           }
//       }
//       rt_thread_mdelay(10);
//
//       }
}

/******按键线程入口函数******/
void thread1_entry(void *parameter)
{

unsigned char key_num,key_down,key_old;
    while (1)
    {
        if(rt_pin_read(KEY1)==0)
        {
            key_num = 1;

        }
        else if(rt_pin_read(KEY2)==0)
        {
            key_num = 2;
        }
        else
            key_num = 0;
        key_down = key_num&(key_num^key_old);
        key_old = key_num;

        switch(key_down){
        case 1:mode = 1;break;
        case 2:mode = 2;break;
        }

        rt_thread_mdelay(15);

    }
}

/******实验/按键执行入口函数******/
void thread2_entry(void *parameter)
{
    char tx_data1[30]={0xee,0xb5,0xa2,0x01,0xff,0xfc,0xff,0xff};
    char tx_data2[30]={0xee,0xb5,0xa1,0x02,0xff,0xfc,0xff,0xff};
    while(1)
    {
        if (mode == 1)  //开始实验按键按下
            {
                rt_kprintf("Key1 ON/OFF\n");

                rt_pin_write(Power, !rt_pin_read(Power));
                mode=0;

            }

        if (mode == 2 )  //智能评价按键按下
            {
            //rt_device_write(u4_dev, 0,tx_data1, rt_strlen(tx_data1));
                if (znpj() == 0) {
                    rt_device_write(u2_dev, 0,tx_data1, rt_strlen(tx_data1));
                }
                else if (znpj() == 1) {
                    rt_device_write(u2_dev, 0,tx_data1, rt_strlen(tx_data2));
                                }
                mode=0;
            }
    }
    rt_thread_mdelay(15);

}

/******拨码移位寄存器读取线程入口函数******/
void thread4_entry(void *parameter)
{
    while (1)
          {
              // 调用函数读取HC165
              readShiftRegister();
              // 延时100毫秒
              rt_thread_mdelay(100);
          }

}

/******电源保护线程入口函数******/
void thread5_entry(void *parameter)
{
    while (1)
          {

            if (rt_pin_read(PB14)==0)
            {
                rt_pin_write(Power, 1);
            }

            rt_thread_mdelay(10);
          }
}

/******UART2初始化******/
void UART2_Init()
{
    rt_err_t ret = 0;
        //寻找UART2
        u2_dev = rt_device_find("uart2");
            if (u2_dev != RT_NULL){
                   rt_kprintf("u2_dev find!\n");
               }
               else{
                   rt_kprintf("u2_dev find failed!\n");
               }
        //开启UART2
        ret = rt_device_open(u2_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
        if (ret <0 ) {
            rt_kprintf("rt_device_open[uart2] falied...\n");
            return ret;
        }
        //设置UART2
        rt_device_control(u2_dev, RT_DEVICE_CTRL_CONFIG, (void*)&u2_configs);

        rt_device_set_rx_indicate(u2_dev, rx_callback2);

        rt_sem_init(&sem2,"rx_sem",0,RT_IPC_FLAG_FIFO);
}

/******UART4初始化******/
void UART4_Init()
{
    rt_err_t ret = 0;
        //寻找UART4
        u4_dev = rt_device_find("uart4");
            if (u4_dev != RT_NULL){
                   rt_kprintf("u4_dev find!\n");
               }
               else{
                   rt_kprintf("u4_dev find failed!\n");
               }
        //开启UART4
        ret = rt_device_open(u4_dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
        if (ret <0 ) {
            rt_kprintf("rt_device_open[uart4] falied...\n");
            return ret;
        }
        //设置UART4
        rt_device_control(u4_dev, RT_DEVICE_CTRL_CONFIG, (void*)&u4_configs);

        rt_device_set_rx_indicate(u4_dev, rx_callback4);

        rt_sem_init(&sem4,"rx_sem",0,RT_IPC_FLAG_FIFO);
}

int main(void)
{

    /******引脚初始化******/
    pin_init();
    rt_pin_write(Power, 1);//上电不带电
    /******创建按键线程******/
    Key =  rt_thread_create("thread1",thread1_entry,RT_NULL,1024,20,5);
        if(Key != RT_NULL){
            rt_kprintf("Key create!\n");
            rt_thread_startup(Key);
        }
        else{
           rt_kprintf("Key create failed!\n");
        }

    /******创建按键执行线程******/
    Key_ZX =  rt_thread_create("thread2",thread2_entry,RT_NULL,1024,21,5);
        if(Key_ZX!= RT_NULL){
            rt_kprintf("Key_ZX create!\n");
            rt_thread_startup(Key_ZX);
        }
        else{
           rt_kprintf("Key_ZX create failed!\n");
        }

    /******创建读取拨码LS165移位寄存器线程******/
    LS165_read = rt_thread_create("thread4",thread4_entry,RT_NULL,1024,10,20);
        if (LS165_read != RT_NULL){
            rt_thread_startup(LS165_read);
            rt_kprintf("LS165_read create!\n");
        }
        else{
            rt_kprintf("LS165_read create failed!\n");
        }
    /******创建电源保护线程******/
    PowerProtect = rt_thread_create("thread5",thread5_entry,RT_NULL,1024,12,20);
        if (PowerProtect != RT_NULL){
            rt_thread_startup(PowerProtect);
            rt_kprintf("PowerProtect create!\n");
        }
        else{
            rt_kprintf("PowerProtect create failed!\n");
        }

    /******创建UART2******/
        UART2_Init();
    /******创建UART4******/
        UART4_Init();

    //创建UART2中断接收线程
    u2_th = rt_thread_create("u2_recv", serial_thread_entry2, NULL, 1024, 20, 5);
    if (u2_th != RT_NULL){
                rt_thread_startup(u2_th);
                rt_kprintf("u2_th create!\n");
            }
            else{
                rt_kprintf("u2_th create failed!\n");
            }
    //创建UART4中断接收线程
    u4_th = rt_thread_create("u4_recv", serial_thread_entry4, NULL, 1024, 20, 5);
    if (u4_th != RT_NULL){
                rt_thread_startup(u4_th);
                rt_kprintf("u4_th create!\n");
            }
            else{
                rt_kprintf("u4_th create failed!\n");
            }


    return RT_EOK;
}


