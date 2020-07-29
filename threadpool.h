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
class Thread;//�����߳�
class key;//������
class Threadpool;//�̳߳�����

typedef void(*run)();


//�ź���


class Thread {

public:
	Thread(Threadpool*);
	Threadpool *threadPool;   // �����̳߳�
	bool busy = false;//�Ƿ�����
	bool exit = false;//�Ƿ��˳�
	HANDLE thread = NULL;//�߳̾��

	static unsigned int __stdcall ThreadFun(PVOID pM); //��̬�̺߳���
	~Thread();//����������Ҫ���˳��߳�
};


class key {
private:
	CRITICAL_SECTION g_cs;
public:
	key() { InitializeCriticalSection(&g_cs); };//���캯����ʼ��
	void lock() { EnterCriticalSection(&g_cs); };//����
	void unlock() { LeaveCriticalSection(&g_cs); };//ȥ��
	~key() { DeleteCriticalSection(&g_cs); };//�ͷ�
};



//�����������������

class Threadpool {
private:
	HANDLE manger = NULL;
	int start_num = 0; //��ʼ�߳�����
	//int free_num = 0;  //�����߳�����
	//int busy_num = 0;  //æµ�߳�����
	//int task_num = 100;//���������С
	list<run> tasks; //�������б�

	key threadLock;      //�߳��б���
	list<Thread *> freeThreadList;  // �����߳��б�

	list<Thread *> busyThreadList;    // æµ�߳��б�
	key taskLock; //�����б���
	

public:
	Threadpool(int num);//���캯����ʼ��(����Ϊ��ʼ���߳���)
	void start();           //�����̳߳�
	void close();          //�ر��̳߳� 
	void notify();          //֪ͨ�̺߳���
	void mv_busy(Thread*t); //����ǿ��̶߳���
	void mv_free(Thread*);  //������̶߳��� 
	void add_task(run fun); //��������б�
	void pop_task();        //ִ�������б������Ӧ������
	bool isEmpty();         //�ж������Ƿ�Ϊ��
	static unsigned int __stdcall GetTask(PVOID pM); //��̬�̺߳���
	~Threadpool();           //���������Դ

};