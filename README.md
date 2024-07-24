# RSOC-2024-Day3    
---
*首先，这是今天RT培训内容的主要总结,写的不是很好，还是以RT-Thread的文档为蓝本，写了一些东西*  
## IPC机制
## 线程间同步    
---
同步是指按预定的先后次序进行运行，线程同步是指多个线程通过特定的机制（如互斥量，事件对象，临界区）来控制线程之间的执行顺序，也可以说是在线程之间通过同步建立起执行顺序的关系，如果没有同步，那线程之间将是无序的。

多个线程操作 / 访问同一块区域（代码），这块代码就称为临界区，上述例子中的共享内存块就是临界区。线程互斥是指对于临界区资源访问的排它性。当多个线程都要使用临界区资源时，任何时刻最多只允许一个线程去使用，其它要使用该资源的线程必须等待，直到占用资源者释放该资源。线程互斥可以看成是一种特殊的线程同步。

线程的同步方式有很多种，其核心思想都是：在访问临界区的时候只允许一个 (或一类) 线程运行。进入 / 退出临界区的方式有很多种：

1）调用 rt_hw_interrupt_disable() 进入临界区，调用 rt_hw_interrupt_enable() 退出临界区；详见《中断管理》的全局中断开关内容。

2）调用 rt_enter_critical() 进入临界区，调用 rt_exit_critical() 退出临界区。    

### 信号量（semaphore）、互斥量（mutex）、和事件集（event）      
---
## 1.信号量  
---
信号量是一种轻型的用于解决线程间同步问题的内核对象，线程可以获取或释放它，从而达到同步或互斥的目的。

信号量工作示意图如下图所示，每个信号量对象都有一个信号量值和一个线程等待队列，信号量的值对应了信号量对象的实例数目、资源数目，假如信号量值为 5，则表示共有 5 个信号量实例（资源）可以被使用，当信号量实例数目为零时，再申请该信号量的线程就会被挂起在该信号量的等待队列上，等待可用的信号量实例（资源）。  
### 信号量控制块
在 RT-Thread 中，信号量控制块是操作系统用于管理信号量的一个数据结构，由结构体 struct rt_semaphore 表示。另外一种 C 表达方式 rt_sem_t，表示的是信号量的句柄，在 C 语言中的实现是指向信号量控制块的指针。信号量控制块结构的详细定义如下：  
```
struct rt_semaphore
{
   struct rt_ipc_object parent;  /* 继承自 ipc_object 类 */
   rt_uint16_t value;            /* 信号量的值 */
};
/* rt_sem_t 是指向 semaphore 结构体的指针类型 */
typedef struct rt_semaphore* rt_sem_t;
```
rt_semaphore 对象从 rt_ipc_object 中派生，由 IPC 容器所管理，信号量的最大值是 65535。  
### 信号量的管理方式
信号量控制块中含有信号量相关的重要参数，在信号量各种状态间起到纽带的作用。信号量相关接口如下图所示，对一个信号量的操作包含：创建 / 初始化信号量、获取信号量、释放信号量、删除 / 脱离信号量。  
![copy from RT-Thread](https://github.com/lqr0323/RSOC-2024-Day3/blob/main/06sem_ops.png)  
### 创建和删除信号量
当创建一个信号量时，内核首先创建一个信号量控制块，然后对该控制块进行基本的初始化工作，创建信号量使用下面的函数接口:  
```
 rt_sem_t rt_sem_create(const char *name,
                        rt_uint32_t value,
                        rt_uint8_t flag);
```
当调用这个函数时，系统将先从对象管理器中分配一个 semaphore 对象，并初始化这个对象，然后初始化父类 IPC 对象以及与 semaphore 相关的部分。在创建信号量指定的参数中，信号量标志参数决定了当信号量不可用时，多个线程等待的排队方式。  
当选择 RT_IPC_FLAG_FIFO（先进先出）方式时，那么等待线程队列将按照先进先出的方式排队，先进入的线程将先获得等待的信号量；当选择 RT_IPC_FLAG_PRIO（优先级等待）方式时，等待线程队列将按照优先级进行排队，优先级高的等待线程将先获得等待的信号量。  
系统不再使用信号量时，可通过删除信号量以释放系统资源，适用于动态创建的信号量。删除信号量使用下面的函数接口：    
```
rt_err_t rt_sem_delete(rt_sem_t sem);
```
### 初始化和脱离信号量
对于静态信号量对象，它的内存空间在编译时期就被编译器分配出来，放在读写数据段或未初始化数据段上，此时使用信号量就不再需要使用 rt_sem_create 接口来创建它，而只需在使用前对它进行初始化即可。初始化信号量对象可使用下面的函数接口:  
```
rt_err_t rt_sem_init(rt_sem_t       sem,
                    const char     *name,
                    rt_uint32_t    value,
                    rt_uint8_t     flag)
```
脱离信号量就是让信号量对象从内核对象管理器中脱离，适用于静态初始化的信号量。脱离信号量使用下面的函数接口：  
```
rt_err_t rt_sem_detach(rt_sem_t sem);
```
### 无等待获取信号量
当用户不想在申请的信号量上挂起线程进行等待时，可以使用无等待方式获取信号量，无等待获取信号量使用下面的函数接口：   
```
rt_err_t rt_sem_trytake(rt_sem_t sem);
```

### 释放信号量
释放信号量可以唤醒挂起在该信号量上的线程。释放信号量使用下面的函数接口：  
```
rt_err_t rt_sem_release(rt_sem_t sem);
```
## 2.互斥量  
---
互斥量又叫相互排斥的信号量，是一种特殊的二值信号量。互斥量类似于只有一个车位的停车场：当有一辆车进入的时候，将停车场大门锁住，其他车辆在外面等候。当里面的车出来时，将停车场大门打开，下一辆车才可以进入。

互斥量工作机制
互斥量和信号量不同的是：拥有互斥量的线程拥有互斥量的所有权，互斥量支持递归访问且能防止线程优先级翻转；并且互斥量只能由持有线程释放，而信号量则可以由任何线程释放。

互斥量的状态只有两种，开锁或闭锁（两种状态值）。当有线程持有它时，互斥量处于闭锁状态，由这个线程获得它的所有权。相反，当这个线程释放它时，将对互斥量进行开锁，失去它的所有权。当一个线程持有互斥量时，其他线程将不能够对它进行开锁或持有它，持有该互斥量的线程也能够再次获得这个锁而不被挂起，如下图时所示。这个特性与一般的二值信号量有很大的不同：在信号量中，因为已经不存在实例，线程递归持有会发生主动挂起（最终形成死锁）。  

  ![copy from RT-Thread](https://github.com/lqr0323/RSOC-2024-Day3/blob/main/06mutex_work.png)

  使用信号量会导致的另一个潜在问题是线程优先级翻转问题。所谓优先级翻转，即当一个高优先级线程试图通过信号量机制访问共享资源时，如果该信号量已被一低优先级线程持有，而这个低优先级线程在运行过程中可能又被其它一些中等优先级的线程抢占，因此造成高优先级线程被许多具有较低优先级的线程阻塞，实时性难以得到保证。如下图所示：有优先级为 A、B 和 C 的三个线程，优先级 A> B > C。线程 A，B 处于挂起状态，等待某一事件触发，线程 C 正在运行，此时线程 C 开始使用某一共享资源 M。在使用过程中，线程 A 等待的事件到来，线程 A 转为就绪态，因为它比线程 C 优先级高，所以立即执行。但是当线程 A 要使用共享资源 M 时，由于其正在被线程 C 使用，因此线程 A 被挂起切换到线程 C 运行。如果此时线程 B 等待的事件到来，则线程 B 转为就绪态。由于线程 B 的优先级比线程 C 高，且线程B没有用到共享资源 M ，因此线程 B 开始运行，直到其运行完毕，线程 C 才开始运行。只有当线程 C 释放共享资源 M 后，线程 A 才得以执行。在这种情况下，优先级发生了翻转：线程 B 先于线程 A 运行。这样便不能保证高优先级线程的响应时间  
   ![copy from RT-Thread](https://github.com/lqr0323/RSOC-2024-Day3/blob/main/06priority_inversion.png)
在 RT-Thread 操作系统中，互斥量可以解决优先级翻转问题，实现的是优先级继承协议 (Sha, 1990)。优先级继承是通过在线程 A 尝试获取共享资源而被挂起的期间内，将线程 C 的优先级提升到线程 A 的优先级别，从而解决优先级翻转引起的问题。这样能够防止 C（间接地防止 A）被 B 抢占，如下图所示。优先级继承是指，提高某个占有某种资源的低优先级线程的优先级，使之与所有等待该资源的线程中优先级最高的那个线程的优先级相等，然后执行，而当这个低优先级线程释放该资源时，优先级重新回到初始设定。因此，继承优先级的线程避免了系统资源被任何中间优先级的线程抢占。  
![copy from RT-Thread](https://github.com/lqr0323/RSOC-2024-Day3/blob/main/06priority_inherit.png)  
### 互斥量控制块
在 RT-Thread 中，互斥量控制块是操作系统用于管理互斥量的一个数据结构，由结构体 struct rt_mutex 表示。另外一种 C 表达方式 rt_mutex_t，表示的是互斥量的句柄，在 C 语言中的实现是指互斥量控制块的指针。互斥量控制块结构的详细定义请见以下代码：  
```
struct rt_mutex
    {
        struct rt_ipc_object parent;                /* 继承自 ipc_object 类 */

        rt_uint16_t          value;                   /* 互斥量的值 */
        rt_uint8_t           original_priority;     /* 持有线程的原始优先级 */
        rt_uint8_t           hold;                     /* 持有线程的持有次数   */
        struct rt_thread    *owner;                 /* 当前拥有互斥量的线程 */
    };
    /* rt_mutext_t 为指向互斥量结构体的指针类型  */
    typedef struct rt_mutex* rt_mutex_t;
```
### 创建和删除互斥量
创建一个互斥量时，内核首先创建一个互斥量控制块，然后完成对该控制块的初始化工作。创建互斥量使用下面的函数接口：  
```
rt_mutex_t rt_mutex_create (const char* name, rt_uint8_t flag);
```
当不再使用互斥量时，通过删除互斥量以释放系统资源，适用于动态创建的互斥量。删除互斥量使用下面的函数接口：  
```
rt_err_t rt_mutex_delete (rt_mutex_t mutex);
```
### 初始化和脱离互斥量
静态互斥量对象的内存是在系统编译时由编译器分配的，一般放于读写数据段或未初始化数据段中。在使用这类静态互斥量对象前，需要先进行初始化。初始化互斥量使用下面的函数接口：  
```
rt_err_t rt_mutex_init (rt_mutex_t mutex, const char* name, rt_uint8_t flag);
```
脱离互斥量将把互斥量对象从内核对象管理器中脱离，适用于静态初始化的互斥量。脱离互斥量使用下面的函数接口：  
```
rt_err_t rt_mutex_detach (rt_mutex_t mutex);
```

### 获取互斥量
线程获取了互斥量，那么线程就有了对该互斥量的所有权，即某一个时刻一个互斥量只能被一个线程持有。获取互斥量使用下面的函数接口： 
```
rt_err_t rt_mutex_take (rt_mutex_t mutex, rt_int32_t time);
```
### 无等待获取互斥量
当用户不想在申请的互斥量上挂起线程进行等待时，可以使用无等待方式获取互斥量，无等待获取互斥量使用下面的函数接口：  
```
rt_err_t rt_mutex_trytake(rt_mutex_t mutex);
```
### 释放互斥量
当线程完成互斥资源的访问后，应尽快释放它占据的互斥量，使得其他线程能及时获取该互斥量。释放互斥量使用下面的函数接口:  
```
rt_err_t rt_mutex_release(rt_mutex_t mutex);
```
## 3.事件集工作机制
事件集主要用于线程间的同步，与信号量不同，它的特点是可以实现一对多，多对多的同步。即一个线程与多个事件的关系可设置为：其中任意一个事件唤醒线程，或几个事件都到达后才唤醒线程进行后续的处理；同样，事件也可以是多个线程同步多个事件。这种多个事件的集合可以用一个 32 位无符号整型变量来表示，变量的每一位代表一个事件，线程通过 “逻辑与” 或“逻辑或”将一个或多个事件关联起来，形成事件组合。事件的 “逻辑或” 也称为是独立型同步，指的是线程与任何事件之一发生同步；事件 “逻辑与” 也称为是关联型同步，指的是线程与若干事件都发生同步。

RT-Thread 定义的事件集有以下特点：

1）事件只与线程相关，事件间相互独立：每个线程可拥有 32 个事件标志，采用一个 32 bit 无符号整型数进行记录，每一个 bit 代表一个事件；

2）事件仅用于同步，不提供数据传输功能；

3）事件无排队性，即多次向线程发送同一事件 (如果线程还未来得及读走)，其效果等同于只发送一次。

在 RT-Thread 中，每个线程都拥有一个事件信息标记，它有三个属性，分别是 RT_EVENT_FLAG_AND(逻辑与)，RT_EVENT_FLAG_OR(逻辑或）以及 RT_EVENT_FLAG_CLEAR(清除标记）。当线程等待事件同步时，可以通过 32 个事件标志和这个事件信息标记来判断当前接收的事件是否满足同步条件。  
### 事件集控制块
在 RT-Thread 中，事件集控制块是操作系统用于管理事件的一个数据结构，由结构体 struct rt_event 表示。另外一种 C 表达方式 rt_event_t，表示的是事件集的句柄，在 C 语言中的实现是事件集控制块的指针。事件集控制块结构的详细定义请见以下代码：  
```
struct rt_event
{
    struct rt_ipc_object parent;    /* 继承自 ipc_object 类 */

    /* 事件集合，每一 bit 表示 1 个事件，bit 位的值可以标记某事件是否发生 */
    rt_uint32_t set;
};
/* rt_event_t 是指向事件结构体的指针类型  */
typedef struct rt_event* rt_event_t;
```
## 4.测试代码   
初始化引脚模式：
初始化LED引脚为输出模式和按键引脚为输入模式。

创建信号量和邮箱：
创建一个信号量 key_sem 用于同步按键事件，创建一个邮箱 mb 用于传递按键事件（按下或松开）。

全局变量：
使用 key_press_count 记录按键按下的次数。

按键线程：
key_thread_entry 线程检测按键状态变化（按下或松开），在按下时增加 key_press_count，并发送相应的事件到邮箱，同时释放信号量。

LED线程：
led_thread_entry 线程等待信号量被释放，然后从邮箱中接收按键事件，根据事件控制LED的亮灭，并通过串口打印按键按下次数和LED状态。
```
#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#ifndef RT_USING_NANO
#include <rtdevice.h>
#endif /* RT_USING_NANO */

#define PIN_KEY0      GET_PIN(C, 0)     // PC0:  KEY0         --> KEY
#define GPIO_LED_R    GET_PIN(F, 12)

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
static rt_sem_t key_sem = RT_NULL;
static rt_mailbox_t mb = RT_NULL;
static rt_uint32_t key_press_count = 0;  // 记录按键按下次数

static void key_thread_entry(void *parameter);
static void led_thread_entry(void *parameter);

int main(void)
{
    // 初始化LED引脚为输出模式
    rt_pin_mode(GPIO_LED_R, PIN_MODE_OUTPUT);
    // 初始化按键引脚为输入模式
    rt_pin_mode(PIN_KEY0, PIN_MODE_INPUT_PULLUP);

    // 创建一个动态信号量
    key_sem = rt_sem_create("key_sem", 0, RT_IPC_FLAG_PRIO);
    if (key_sem == RT_NULL)
    {
        rt_kprintf("Create semaphore failed.\n");
        return -1;
    }

    // 创建一个邮箱
    mb = rt_mb_create("mb", 10, RT_IPC_FLAG_PRIO);
    if (mb == RT_NULL)
    {
        rt_kprintf("Create mailbox failed.\n");
        return -1;
    }

    // 创建按键线程
    tid1 = rt_thread_create("key_thread",
                            key_thread_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    // 创建LED线程
    tid2 = rt_thread_create("led_thread",
                            led_thread_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    return 0;
}

static void key_thread_entry(void *parameter)
{
    int key_state = 1; // 初始状态为松开
    int prev_key_state = 1;
    while (1)
    {
        key_state = rt_pin_read(PIN_KEY0);
        if (key_state == PIN_LOW && prev_key_state == PIN_HIGH)
        {
            // 按键按下
            rt_kprintf("KEY0 pressed!\r\n");
            key_press_count++; // 增加按键按下次数
            rt_mb_send(mb, 1); // 发送按下信号
            rt_sem_release(key_sem);
        }
        else if (key_state == PIN_HIGH && prev_key_state == PIN_LOW)
        {
            // 按键松开
            rt_kprintf("KEY0 released!\r\n");
            rt_mb_send(mb, 0); // 发送松开信号
            rt_sem_release(key_sem);
        }
        prev_key_state = key_state;
        rt_thread_mdelay(10);
    }
}

static void led_thread_entry(void *parameter)
{
    rt_uint32_t key_event = 0;
    while (1)
    {
        rt_sem_take(key_sem, RT_WAITING_FOREVER); // 等待信号量
        if (rt_mb_recv(mb, &key_event, RT_WAITING_FOREVER) == RT_EOK)
        {
            if (key_event == 1)
            {
                // 按键按下时点亮LED
                rt_kprintf("LED ON\r\n");
                rt_kprintf("Key press count: %d\r\n", key_press_count);
                rt_pin_write(GPIO_LED_R, PIN_HIGH);
            }
            else
            {
                // 按键松开时熄灭LED
                rt_kprintf("LED OFF\r\n");
            }
        }
    }
}
```
## 5.测试截图    
![today's code ](https://github.com/lqr0323/RSOC-2024-Day3/blob/main/%E5%B1%8F%E5%B9%95%E6%88%AA%E5%9B%BE%202024-07-24%20174116.png)  
## 打卡下班，去搞电赛去咯
