#pragma once

#include <boost/thread.hpp>
#include "../singleton/singleton.h"

class AsynTask
{
public:
	AsynTask(){};
	virtual ~AsynTask(){};

	virtual void ExecuteTask()=0;
	virtual void FinishTask()=0;
};

typedef boost::shared_ptr<AsynTask> AsynTaskPtr;

class AsynTaskMgr : public Singleton<AsynTaskMgr>
{
public:
    AsynTaskMgr();
    virtual ~AsynTaskMgr();
	
	void update(int dt);
	void addTask(AsynTaskPtr task);
	void addTaskFinish(AsynTaskPtr task);
	void taskThread();

private: 
	boost::mutex mu;  
	boost::condition_variable_any cond;

	boost::mutex			taskListLock;
	std::list<AsynTaskPtr>	taskList;

	boost::mutex			taskFinishListLock;
	std::list<AsynTaskPtr>	taskFinishList;
};

#define sAsynTaskMgr (*AsynTaskMgr::instance())
