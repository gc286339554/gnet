#pragma once
#include "pre_header.h"
#include "../../commom/singleton.h"
#include "glog/logging.h"

namespace gnet {
	class glog : public singleton<glog>
	{
	public:
		glog();
		~glog();

		static void newInstance(const char* name, const char* path) {
			singleton::newInstance();
			google::InitGoogleLogging(name);
			FLAGS_log_dir = path;
			FLAGS_colorlogtostderr = true;
			FLAGS_stderrthreshold = google::INFO;

			/*
			LOG(INFO) << "INFO";
			LOG(INFO) << "INFO1";
			LOG(WARNING) << "WARNING";
			LOG(WARNING) << "WARNING1";
			LOG(ERROR) << "ERROR";
			LOG(ERROR) << "ERROR1";
			LOG(FATAL) << "FATAL";
			*/
		}

	private:

	};
};