#ifndef MONOTIDE_LIB_WORKER_H
#define MONOTIDE_LIB_WORKER_H

#include <windows.h>

namespace lib {
namespace worker {

struct Worker
{
	enum {
		E_OK, E_CREATE, E_NO_RESTART, E_RESUME,
	} error;

	HANDLE handle;
	DWORD thread_id;
	bool running;

	Worker(bool auto_start=false);

	void start();
	void stop();
	bool wait(DWORD msec=INFINITE) const;

	virtual void task() = 0;
	virtual DWORD run();

private:
	static DWORD WINAPI main(LPVOID);
};

} // namespace worker
} // namespace lib

#endif // !MONOTIDE_LIB_WORKER_H
