#include "AsynTaskMgr.h"

AsynTaskMgr::AsynTaskMgr()
{
	boost::thread t(boost::bind(&AsynTaskMgr::taskThread,this));
	t.detach();
}

AsynTaskMgr::~AsynTaskMgr()
{
}

void AsynTaskMgr::update(int dt)
{
	while(true)
	{
		AsynTaskPtr tempTask;
		{
			boost::unique_lock<boost::mutex> lock(taskFinishListLock);
			if (taskFinishList.empty())
			{
				break;
			}
			else
			{
				tempTask = taskFinishList.front();
				taskFinishList.pop_front();
			}
		}
		if (tempTask)
		{
			tempTask->FinishTask();
		}
	}
}

void AsynTaskMgr::addTask(AsynTaskPtr task)
{
	boost::unique_lock<boost::mutex> lock(taskListLock);
	taskList.push_back(task);
	cond.notify_one();  
}
void AsynTaskMgr::addTaskFinish(AsynTaskPtr task)
{
	boost::unique_lock<boost::mutex> lock(taskFinishListLock);
	taskFinishList.push_back(task);
}

void AsynTaskMgr::taskThread()
{
	while(true)
	{
		AsynTaskPtr tempTask;
		{
			boost::unique_lock<boost::mutex> lock(taskListLock);
			if (!taskList.empty())
			{
				tempTask = taskList.front();
				taskList.pop_front();
			}
		}
		if (tempTask)
		{
			tempTask->ExecuteTask();
			addTaskFinish(tempTask);
		}
		else
		{
			boost::mutex::scoped_lock lock(mu);  
			cond.wait(mu);
		}
	}
}
