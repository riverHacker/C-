# 一个正在完善的线程池类库
windows,c++线程池
# 使用方法
示例代码：
#include"threadpool.h"

void bs() {

	printf("我i是现成骄傲和防空雷达啊回复\n");
}

int main() {
	int c = 0;
	Threadpool a(4);
	a.start();
	for (int i = 0; i < 10; i++) {
		a.add_task(bs);
	}
	while (true) {
		
		a.add_task(bs);
		Sleep(1000);
	
	}

	
	
	a.close();
	
	return 0;
}
