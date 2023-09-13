#pragma once

#include <ctime>
#include <iostream>
#include <string>

#include "CTList.h"

static class ErrorLog {
public:
	struct error {
		error(std::string time, std::string _message){
			timestamp = time;
			message = _message;
		}
		std::string timestamp;
		std::string message;
	};
	static void log_error(std::string log_entry) {
		std::time_t t = std::time(0);
		std::tm now;
		localtime_s(&now, &t);

		char* stringchars = new char[64];
		ctime_s(stringchars, 64, &t);

		std::string time = std::to_string(now.tm_year + 1900) + '-'
						 + std::to_string(now.tm_mon + 1) + '-'
						 + std::to_string(now.tm_mday) + '-'
						 + std::string(stringchars);
		log.Append(new ErrorLog::error(time, log_entry));
	};
	static CTList<error> log;
};