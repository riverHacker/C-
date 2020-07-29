#include"threadpool.h"

void Threadpool::mv_busy(Thread*t)
{

	freeThreadList.remove(t);//�ӿ����߳��б���ɾ��
	busyThreadList.remove(t);//��æµ�߳��б���ɾ��
	busyThreadList.push_back(t);

	return;
}

void Threadpool::mv_free(Thread*t)
{

	threadLock.lock();
	freeThreadList.remove(t);
	busyThreadList.remove(t);//��æµ�߳��б���ɾ��
	freeThreadList.push_back(t);
	threadLock.unlock();
	printf("���ǽ�����ж���\n");
	return;
}

Threadpool::Threadpool(int num)//����Ҫ�̵߳���
{
	//�̳߳�ʼ��
	printf("��ӭʹ��\n");
	manger = (HANDLE)_beginthreadex(0, 0, Threadpool::GetTask, this, 0, 0);
	start_num = num;
}

void Threadpool::start()
{
	//�����̳߳�
	for (int i = 0; i < start_num; i++) {
		Thread *temp = new Thread(this);
		busyThreadList.push_back(temp); //���뵽æµ�߳�
	}

}

void Threadpool::close()
{
	threadLock.lock();      //�߳��б���
	list<Thread *>::iterator iter;
	if (!freeThreadList.empty()) {
		for (iter = freeThreadList.begin(); iter != freeThreadList.end(); iter++)
		{
			(*iter)->exit = true;
			//�����߳�
			while (ResumeThread((*iter)->thread) > 0)
			{
				continue;
			}
		}
	}
	if (!busyThreadList.empty()) {
		for (iter = busyThreadList.begin(); iter != busyThreadList.end(); iter++)
		{
			(*iter)->exit = true;
			//�����߳�
			while (ResumeThread((*iter)->thread) > 1)
			{
				continue;
			}
		}

	}

	threadLock.unlock();
}

void Threadpool::notify()
{
	if (isEmpty()) {
		//����������ȵ�
		return;
	}
	threadLock.lock();
	Thread*temp = NULL;
	if (!freeThreadList.empty()) {
		temp = freeThreadList.back();
		mv_busy(temp);
		threadLock.unlock();
		while (ResumeThread(temp->thread) > 1) {
			continue;
		}
	}
	else {

		threadLock.unlock();
	}

}

void Threadpool::add_task(run fun)
{

	taskLock.lock();
	tasks.push_back(fun);                                             //����һ������
	taskLock.unlock();
}

void Threadpool::pop_task()
{
	run temp = NULL;
	taskLock.lock();
	if (!tasks.empty())
	{
		temp = tasks.front();
		tasks.pop_front();
	}

	taskLock.unlock();
	if (temp != NULL) {
		temp();
	}
}

bool Threadpool::isEmpty()
{
	//�߳���
	taskLock.lock();
	if (tasks.empty()) {
		taskLock.unlock();
		return true;
	}
	taskLock.unlock();
	return false;
}

unsigned int __stdcall Threadpool::GetTask(PVOID pM)
{
	Threadpool* p = (Threadpool*)pM;
	//�����߳�

	while (true)
	{
		//����һ���߳�
		p->notify();
		Sleep(1);
	}

	return 0;
}

Threadpool::~Threadpool()
{

	printf("�ұ�����");
	Thread* temp = NULL;//��ʱ����
	while (true) {
		threadLock.lock();
		if (!freeThreadList.empty()) {

			temp = freeThreadList.back();
			freeThreadList.pop_back();
			threadLock.unlock();
			if (temp != NULL) {
				delete temp;
				temp = NULL;
			}
		}
		else {
			threadLock.unlock();
			break;
		}

	}
	while (true) {
		threadLock.lock();
		if (!busyThreadList.empty()) {

			temp = busyThreadList.back();
			busyThreadList.pop_back();
			threadLock.unlock();
			if (temp != NULL) {
				delete temp;
				temp = NULL;
			}
		}
		else {
			threadLock.unlock();
			break;
		}

	}
	//threadLock.unlock();
}

Thread::Thread(Threadpool *pool)
{
	busy = false;
	exit = false;
	threadPool = pool;
	thread = (HANDLE)_beginthreadex(0, 0, Thread::ThreadFun, this, NULL, 0);
	if (thread == 0) {
		printf("�̴߳���ʧ��");
		thread = NULL;
	}
}

unsigned int __stdcall Thread::ThreadFun(PVOID pM)
{
	printf("ThreadID: %d\n", GetCurrentThreadId());
	Thread *pool = (Thread*)pM;

	while (true)
	{

		if (pool->exit) {
			break;//�˳��߳�
		}
		////����������Ϊ��

		if (pool->threadPool->isEmpty()) {
			pool->busy = false;//����Ϊ����
			pool->threadPool->mv_free(pool);
			//��������߳�����
			printf("��˯������\n");
			if (SuspendThread(pool->thread) == -1)
			{
				break;
			}
			printf("���Ѿ�����\n");
			continue;
		}

		//ִ���̺߳���      ��Ҫ���������

		printf("����: %d\n", GetCurrentThreadId());
		pool->threadPool->pop_task();
		printf("�Ҳ�Ը���˳�|\n");

	}
	return 0;
}

Thread::~Thread()
{
	if (thread != NULL) {
		printf("���Ѿ�����ȷʾ�ͷ�\n");
		WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread); //�����߳���Դ

		thread = NULL;
	}

}