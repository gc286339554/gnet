#include "TimerMgr.hpp"

TimerMgr::TimerMgr()
{
	
};
TimerMgr::~TimerMgr()
{

};

void TimerMgr::update(int dt)
{
	if (!taskList.empty())
	{
		DeadLineTimeTask* task = *(taskList.begin());
		time_t now = time(NULL);
		if (task->isBefor(now))
		{
			if (task->getIsValid())
			{
				task->ExecuteTask();	
			}
			taskList.pop_front();
			delete task;
		}
	}
}

void TimerMgr::addTask(DeadLineTimeTask* task)
{
	if (task->SingleTask())
	{
		for (auto it = taskList.begin(); it != taskList.end(); it++)
		{
			if (task->taskName.compare((*(*it)).taskName.c_str()) == 0)
			{
				(*it)->setIsValid(false);
			}
		}
	}
	
	auto it = taskList.begin();
	for (; it != taskList.end(); it++)
	{
		if (task->isBefor(*(*it)))
		{
			break;
		}
	}
	taskList.insert(it,task);
}