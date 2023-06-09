#include "threads.h"
#include <fstream>
#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <set>
#include <atomic>
#include <yaml-cpp\yaml.h>
//#define _CRT_SECURE_NO_WARNINGS 1

namespace alarm   { extern std::set<Alarm, Compare> alarms; }
namespace threads { extern std::mutex mutex; }

namespace io {
	std::string appdata;
	std::ifstream alarms_in, config_file;
	std::ofstream alarms_out;
	YAML::Node config;

	void init() {

		/* appdata = getenv("LOCALAPPDATA"); */ /* == */ {
			char* path = new char[128];
			size_t buflen = 128;
			getenv_s(&buflen, path, 128, "LOCALAPPDATA");
			appdata = path;
		}
		

		puts(appdata.c_str());

		CreateDirectoryA((appdata + "\\Ult1").c_str(), NULL);
		CreateDirectoryA((appdata + "\\Ult1\\Alarm").c_str(), NULL);
		CreateFileA(
			(appdata + "\\Ult1\\Alarm\\alarms.txt").c_str(), 
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		alarms_in  = std::ifstream(appdata + "\\Ult1\\Alarm\\alarms.txt");
		alarms_out = std::ofstream(appdata + "\\Ult1\\Alarm\\alarms.txt", std::ios::app);

		config = YAML::LoadFile(appdata + "\\Ult1\\Alarm\\config.yml");


		//alarms_in.ignore();

		while (alarms_in.good()) {
			Alarm alarm;
			alarms_in >> alarm.name >> alarm.end_time >> alarm.created;
			if (alarms_in.eof() || alarms_in.bad()) break;
			alarm::register_alarm(alarm, false);
			LOG2("fileio", "New alarm(\"%s\", %lld) read", alarm.name.c_str(), alarm.end_time);

		}
		update_alarm_file();

	}

	int get_server_port() {
		if (config["server_port"])
			return config["server_port"].as<int>();
		else return 348;
	}

	int get_client_port() {
		if (config["client_port"])
			return config["client_port"].as<int>();
		else return 347;
	}

	/**
	  This function ONLY appends the alarm instance on to the output file!
	*/
	void append_alarm(Alarm& alarm) {
		alarms_out << alarm.name.c_str() << ' ' << alarm.end_time << ' ' << alarm.created << std::endl;
	}

	void update_alarm_file() {
		LOG("fileio", "Started updating alarm file...");

		std::ofstream out = std::ofstream(appdata + "\\Ult1\\Alarm\\alarms.txt");
		for (Alarm alarm : alarm::alarms)
			append_alarm(alarm);
		LOG("fileio", "Finished updating alarm file.")
	}


}