#pragma once
#include "console.h"

//add template and add paramter to console

class Logger :public console {
private:
public:

	inline Logger(const char* console_title) {
		console::init_console();
		console::set_console_title(console_title);
		printf(console_title);
	}
	template<typename... Args>
	inline void Info(const char* message, Args... args) {

		console::log_info();
		printf("[Info]\r\n");
		printf(message, args...);
		printf("\n");
		console::reset_console();
	}

	template<typename... Args>
	inline void Warning(const char* message, Args... args) {

		printf("[Warning]\t");
		printf(message, args...);
		printf("\n");

	}

	template <typename... Args>
	inline void Error(const char* message, Args... args) {
		console::log_error();
		printf("[Error]\r\n");
		printf(message, args...);
		printf("\n");
		console::reset_console();
	}
	template <typename... Args>
	inline void Usage(const char* message, Args... args) {
		console::reset_console();
		printf("[Usage]\r\n");
		printf(message, args...);
		printf("\n");
		console::reset_console();
	}

};