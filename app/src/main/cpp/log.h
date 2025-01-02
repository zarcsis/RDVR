#pragma once

#include <android/log.h>

#include <string>
#include <thread>

static std::string TAG;

#define LOGI(fmt, ...) __android_log_print(ANDROID_LOG_INFO, TAG.c_str(), "%s(tId: %zu, line: %d): " fmt, __FUNCTION__, std::hash<std::thread::id>()(std::this_thread::get_id()), __LINE__, ##__VA_ARGS__)

#define _STRINGIFY(argument) #argument
#define STRINGIFY(argument) _STRINGIFY(argument)
