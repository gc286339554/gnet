#include "glog.h"
using namespace gnet;
glog::glog()
{
}

glog::~glog()
{
	google::ShutdownGoogleLogging();
}