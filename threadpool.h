#pragma once
#include <iostream>
#include<vector>
#include<queue>
#include<list>
#include<windows.h>
#include<process.h>
using std::queue;
using std::list;
using std::vector;
class Thread;//申明线程
class key;//申明锁
class Threadpool;//线程池主类

typedef void(*run)();


//信号量


class Thread {

public:
	Thread(Threadpool*);
	Threadpool *threadPool;   // 所属线程池
	bool busy = false;//是否工作中
	bool exit = false;//是否退出
	HANDLE thread = NULL;//线程句柄

	static unsigned int __stdcall ThreadFun(PVOID pM); //静态线程函数
	~Thread();//析构函数主要是退出线程
};


class key {
private:
	CRITICAL_SECTION g_cs;
public:
	key() { InitializeCriticalSection(&g_cs); };//构造函数初始化
	void lock() { EnterCriticalSection(&g_cs); };//加锁
	void unlock() { LeaveCriticalSection(&g_cs); };//去锁
	~key() { DeleteCriticalSection(&g_cs); };//释放
};



//任务操作放在主类中

class Threadpool {
private:
	HANDLE manger = NULL;
	int start_num = 0; //初始线程数量
	//int free_num = 0;  //空闲线程数量
	//int busy_num = 0;  //忙碌线程数量
	//int task_num = 100;//控制任务大小
	list<run> tasks; //任务函数列表

	key threadLock;      //线程列表锁
	list<Thread *> freeThreadList;  // 空闲线程列表

	list<Thread *> busyThreadList;    // 忙碌线程列表
	key taskLock; //任务列表锁
	

public:
	Threadpool(int num);//构造函数初始化(参数为初始化线程数)
	void start();           //开启线程池
	void close();          //关闭线程池 
	void notify();          //通知线程函数
	void mv_busy(Thread*t); //移入非空线程队列
	void mv_free(Thread*);  //移入空线程队列 
	void add_task(run fun); //添加任务列表
	void pop_task();        //执行任务列表并清除对应的任务
	bool isEmpty();         //判断任务是否为空
	static unsigned int __stdcall GetTask(PVOID pM); //静态线程函数
	~Threadpool();           //清理各大资源

};