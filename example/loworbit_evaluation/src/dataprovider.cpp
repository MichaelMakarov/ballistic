#include <dataprovider.h>
#include <fstream>

std::map<times::datetime, space_weather> load_spaceweather_data(const std::string& filepath) {
	auto fin{ std::ifstream(filepath) };
	if (fin.is_open()) {
		std::map<times::datetime, space_weather> dict;
		std::string line{ "line" };
		space_weather data{ 0 };
		long long year;
		unsigned short month, day;
		double value;
		auto completeline = [](std::ifstream& fin, char& buf) {
			buf = '\0';
			while (buf != '\n' && !fin.eof()) fin.read(&buf, 1);
		};

		while (!fin.eof()) {
			fin >> line;
			if (line == "BEGIN") break;
		}
		completeline(fin, line[0]);
		auto start = fin.tellg();
		while (!fin.eof()) {
			fin >> line;
			if (line == "END") break;
		}
		auto final = fin.tellg();
		final -= 3;
		fin.clear();
		fin.seekg(start);

		while (!fin.eof()) {
			fin >> year >> month >> day;
			for (size_t i = 0; i < 2; ++i) fin >> value;
			for (size_t i = 0; i < 8; ++i) fin >> data.Kp[i];
			fin >> data.Kpsum;
			for (size_t i = 0; i < 8; ++i) fin >> data.Ap[i];
			fin >> data.Apavg;
			for (size_t i = 0; i < 7; ++i) fin >> value;
			fin >> data.F10_7 >> value >> data.F81;
			dict[times::datetime{ year, month, day, 0, 0, 0 }] = data;
			completeline(fin, line[0]);
			if (fin.tellg() == final) break;
		}
		fin.close();
		return dict;
	} throw std::runtime_error("failed to open file: " + filepath);
}