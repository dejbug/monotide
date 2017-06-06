#include "lib_worker.h"
#include <stdio.h>

namespace lib {
namespace worker {

Worker::Worker(bool auto_start)
		: error(E_OK)
		, handle(nullptr)
		, running(false)
{
	handle = CreateThread(nullptr, 0, main, (LPVOID) this,
		auto_start ? 0 : CREATE_SUSPENDED , &thread_id);

	if (!handle) error = E_CREATE;
	else running = auto_start;
}

void Worker::start()
{
	if (error) return;
	if (running) return;
	if (!handle)
	{
		error = E_NO_RESTART;
		return;
	}

	if ((DWORD) -1 == ResumeThread(handle))
	{
		error = E_RESUME;
		return;
	}

	running = true;
}

void Worker::stop()
{
	running = false;
}

bool Worker::wait(DWORD msec) const
{
	if (error) return false;
	if (!handle) return false;
	return WAIT_OBJECT_0 == WaitForSingleObject(handle, msec);
}

void Worker::task()
{
	Sleep(1000);
	printf(" [ worker %08x ] tick\n", (size_t) this);
	running = false;
}

DWORD Worker::run()
{
	printf(" [ Worker %08x ] enter\n", (size_t) this);

	while (running)
	{
		task();
	}

	printf(" [ Worker %08x ] leave\n", (size_t) this);

	return 0;
}

DWORD WINAPI Worker::main(LPVOID param)
{
	Worker * worker = (Worker *) param;
	return worker->run();
}

}
}
