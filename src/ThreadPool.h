#ifndef THREADPOOL_H
#define THREADPOOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Def.h"

typedef void ThreadPool;

ThreadPool* ThreadPool_Create();

typedef struct _ThreadPoolJob {
	void (*func)(void*);
	void* arg;
} ThreadPoolJob;

void ThreadPool_QueueJob(ThreadPool *pool, ThreadPoolJob job);
void ThreadPool_WaitJobs(ThreadPool *pool);

void ThreadPool_Destroy(ThreadPool *pool);

#ifdef __cplusplus
}
#endif

#endif
