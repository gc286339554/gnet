#pragma once

//#define ASIO_STANDALONE
#include "gnet_def.h"
#include <string>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <stdio.h>
#include <functional>
#include <atomic>
#include <assert.h>

#define SAFE_DEL(x) do { delete x; x = nullptr;} while (0);


