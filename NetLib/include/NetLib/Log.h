#pragma once

#define ENABLE_LOG
#include <iostream>

template<typename... Args>
void Log(Args&&... args)
{
#ifdef ENABLE_LOG
    LogSingle(std::forward<Args>(args)...);
    std::cout << std::endl;
#endif
}

template<typename T, typename... Args>
void LogSingle(T&& t, Args&&... args)
{
    std::cout << t;
    LogSingle(std::forward<Args>(args)...); // Recursively process each argument
}

template<typename T>
void LogSingle(T&& t)
{
	std::cout << t;
}