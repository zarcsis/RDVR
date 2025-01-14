#pragma once

#include <jni.h>

#include <string>

std::string jStringToString(JNIEnv *const env, jstring str);
