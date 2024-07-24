/*
 * Copyright (c) 2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-06     Supperthomas first version
 * 2023-12-03     Meco Man     support nano version
 */

#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#ifndef RT_USING_NANO
#include <rtdevice.h>
#include <rtdbg.h>
#endif /* RT_USING_NANO */

rt_thread_t th1 = RT_NULL, th2 = RT_NULL, th3 = RT_NULL;

void th1_entry(void *parameter)
{
    int count = 20; // 只打印20次
    while (count--)
    {
        rt_kprintf("Thread 1 is running, count: %d\n", 20 - count);
        rt_thread_mdelay(1000); // 延迟1000毫秒
    }
    rt_kprintf("Thread 1 finished.\n");
}

void th2_entry(void *parameter)
{
    int count = 30; // 只打印30次
    while (count--)
    {
        rt_kprintf("Thread 2 is running, count: %d\n", 30 - count);
        rt_thread_mdelay(2000); // 延迟2000毫秒
    }
    rt_kprintf("Thread 2 finished.\n");
}

void th3_entry(void *parameter)
{
    int count = 40; // 只打印40次
    while (count--)
    {
        rt_kprintf("Thread 3 is running, count: %d\n", 40 - count);
        rt_thread_mdelay(3000); // 延迟3000毫秒
    }
    rt_kprintf("Thread 3 finished.\n");
}

int main(void)
{
    // 创建线程 th1，优先级最高
    th1 = rt_thread_create("th1", th1_entry, RT_NULL, 1024, 10, 10);
    if (th1 == RT_NULL)
    {
        LOG_E("Thread 1 create failed...\n");
    }
    else
    {
        rt_thread_startup(th1);
        LOG_D("Thread 1 create success!\n");
    }

    // 创建线程 th2，优先级第二高
    th2 = rt_thread_create("th2", th2_entry, RT_NULL, 1024, 20, 10);
    if (th2 == RT_NULL)
    {
        LOG_E("Thread 2 create failed...\n");
    }
    else
    {
        rt_thread_startup(th2);
        LOG_D("Thread 2 create success!\n");
    }

    // 创建线程 th3，优先级最低
    th3 = rt_thread_create("th3", th3_entry, RT_NULL, 1024, 30, 10);
    if (th3 == RT_NULL)
    {
        LOG_E("Thread 3 create failed...\n");
    }
    else
    {
        rt_thread_startup(th3);
        LOG_D("Thread 3 create success!\n");
    }

    return 0;
}



