#include "util.h"


/**
 * get current time as a "string"
 * format: HH:MM:SS
 */


string current_local_time() {
	time_t current_time_t = chrono::system_clock::to_time_t(chrono::system_clock::now());

	tm current_time_tm;
	localtime_s(&current_time_tm, &current_time_t);

	char current_strftime_buffer[100];
	strftime(current_strftime_buffer, 50, "%T", &current_time_tm);
	
	return current_strftime_buffer;
}


/**
 * get current time as a "string"
 * format: YYYY-MM-DD HH:MM:SS
 */


string current_local_date_and_time() {
	time_t current_time_t = chrono::system_clock::to_time_t(chrono::system_clock::now());

	tm current_time_tm;
	localtime_s(&current_time_tm, &current_time_t);

	char current_strftime_buffer[100];
	strftime(current_strftime_buffer, 50, "%Y-%m-%d %T", &current_time_tm);

	return current_strftime_buffer;
}
