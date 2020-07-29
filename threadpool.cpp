#include"threadpool.h"

void Threadpool::mv_busy(Thread*t)
{

	freeThreadList.remove(t);//从空闲线程列表中删除
	busyThreadList.remove(t);//从忙碌线程列表中删除
	busyThreadList.push_back(t);

	return;
}

void Threadpool::mv_free(Thread*t)
{

	threadLock.lock();
	freeThreadList.remove(t);
	busyThreadList.remove(t);//从忙碌线程列表中删除
	freeThreadList.push_back(t);
	threadLock.unlock();
	printf("我是进入空闲队列\n");
	return;
}

Threadpool::Threadpool(int num)//不需要线程担心
{
	//线程初始化
	printf("欢迎使用\n");
	manger = (HANDLE)_beginthreadex(0, 0, Threadpool::GetTask, this, 0, 0);
	start_num = num;
}

void Threadpool::start()
{
	//开启线程池
	for (int i = 0; i < start_num; i++) {
		Thread *temp = new Thread(this);
		busyThreadList.push_back(temp); //加入到忙碌线程
	}

}

void Threadpool::close()
{
	threadLock.lock();      //线程列表锁
	list<Thread *>::iterator iter;
	if (!freeThreadList.empty()) {
		for (iter = freeThreadList.begin(); iter != freeThreadList.end(); iter++)
		{
			(*iter)->exit = true;
			//唤醒线程
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
			//唤醒线程
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
		//管理任务调度的
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
	tasks.push_back(fun);                                             //加入一个任务
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
	//线程锁
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
	//唤醒线程

	while (true)
	{
		//唤醒一个线程
		p->notify();
		Sleep(1);
	}

	return 0;
}

Threadpool::~Threadpool()
{

	printf("我被析构");
	Thread* temp = NULL;//临时变量
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
		printf("线程创建失败");
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
			break;//退出线程
		}
		////如果任务队列为空

		if (pool->threadPool->isEmpty()) {
			pool->busy = false;//设置为空闲
			pool->threadPool->mv_free(pool);
			//这里进行线程休眠
			printf("我睡着了了\n");
			if (SuspendThread(pool->thread) == -1)
			{
				break;
			}
			printf("我已经醒了\n");
			continue;
		}

		//执行线程函数      主要是任务队列

		printf("工号: %d\n", GetCurrentThreadId());
		pool->threadPool->pop_task();
		printf("我不愿意退出|\n");

	}
	return 0;
}

Thread::~Thread()
{
	if (thread != NULL) {
		printf("我已经被正确示释放\n");
		WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread); //清理线程资源

		thread = NULL;
	}

}