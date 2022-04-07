#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "m2/Def.h"

typedef void ThreadPool;

ThreadPool* ThreadPool_Create();

typedef struct _ThreadPoolJob {
	void (*func)(void*);
	void* arg;
} ThreadPoolJob;

void ThreadPool_QueueJob(ThreadPool *pool, ThreadPoolJob job);
void ThreadPool_WaitJobs(ThreadPool *pool);

void ThreadPool_Destroy(ThreadPool *pool);

#endif
