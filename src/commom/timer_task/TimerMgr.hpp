#pragma once
#include "..\singleton\singleton.h"
#include <list>
#include <time.h>


class DeadLineTimeTask
{
public:
	DeadLineTimeTask()
	{
		setIsValid(true);
	};
	virtual ~DeadLineTimeTask(){};

	virtual void ExecuteTask()=0;
	virtual bool SingleTask(){return true;};//只允许单一任务

	bool isBefor(time_t	time_)
	{
		return DeadLineTime < time_;
	}

	bool isBefor(DeadLineTimeTask&	task)
	{
		return DeadLineTime < task.DeadLineTime;
	}

	void setDeadLineTime(time_t	time_)
	{
		DeadLineTime = time_;
	}
	void setDeadLineTimeFromNowOn(int time_delay)
	{
		time_t now = time(NULL);  
		DeadLineTime = now + time_delay;
	}

	void setIsValid(bool isValid_)
	{
		isValid = isValid_;
	}

	bool getIsValid()
	{
		return isValid;
	}

	std::string taskName;
	time_t	DeadLineTime;
	bool	isValid;//用于标识 重复任务
};


class TimerMgr : public Singleton<TimerMgr>
{
public:
	TimerMgr();
	~TimerMgr();

	void update(int dt);
	void addTask(DeadLineTimeTask* task);
private:
	std::list<DeadLineTimeTask*> taskList;	
};

#define sTimerMgr (*TimerMgr::instance())