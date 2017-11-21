#ifndef MONOTIDE_EVENT_QUEUE_H
#define MONOTIDE_EVENT_QUEUE_H

#include <windows.h>
#include <vector>
#include <memory>

template<class T>
struct EventQueue
{
	EventQueue();
	virtual ~EventQueue();

	void push_back(T * t, bool notify=true);
	T * pop_back(bool clear=false);
	T * get_back();
	void clear();

	void notify() const;
	bool wait(DWORD msec=INFINITE) const;

	bool is_empty();
	size_t get_count();

private:
	CRITICAL_SECTION mutex;
	HANDLE queue_event;
	std::vector<std::unique_ptr<T>> tt;
};

#include <stdexcept>

template<class T>
EventQueue<T>::EventQueue()
{
	queue_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (!queue_event) throw std::runtime_error("EventQueue | CreateEvent failed: unable to create queue_event");
	InitializeCriticalSection(&mutex);
}

template<class T>
EventQueue<T>::~EventQueue()
{
	DeleteCriticalSection(&mutex);
	CloseHandle(queue_event);
	queue_event = nullptr;
}

template<class T>
void EventQueue<T>::push_back(T * t, bool notify)
{
	EnterCriticalSection(&mutex);
	tt.push_back(std::unique_ptr<T>(t));
	LeaveCriticalSection(&mutex);
	if (notify) SetEvent(queue_event);
}

template<class T>
T * EventQueue<T>::pop_back(bool clear)
{
	T * t = nullptr;
	EnterCriticalSection(&mutex);
	if (!tt.empty())
	{
		t = tt.back().release();
		if (clear) tt.clear();
		else tt.pop_back();
	}
	LeaveCriticalSection(&mutex);
	return t;
}

template<class T>
T * EventQueue<T>::get_back()
{
	T * t = nullptr;
	EnterCriticalSection(&mutex);
	if (!tt.empty())
	{
		t = tt.back().get();
	}
	LeaveCriticalSection(&mutex);
	return t;
}

template<class T>
void EventQueue<T>::clear()
{
	EnterCriticalSection(&mutex);
	tt.clear();
	LeaveCriticalSection(&mutex);
}

template<class T>
void EventQueue<T>::notify() const
{
	SetEvent(queue_event);
}

template<class T>
bool EventQueue<T>::wait(DWORD msec) const
{
	return WAIT_OBJECT_0 == WaitForSingleObject(queue_event, msec);
}

template<class T>
bool EventQueue<T>::is_empty()
{
	EnterCriticalSection(&mutex);
	auto const _ = tt.empty();
	LeaveCriticalSection(&mutex);
	return _;
}

template<class T>
size_t EventQueue<T>::get_count()
{
	EnterCriticalSection(&mutex);
	auto const _ = tt.size();
	LeaveCriticalSection(&mutex);
	return _;
}

#endif // !MONOTIDE_EVENT_QUEUE_H
