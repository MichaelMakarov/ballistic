#pragma once
#include <array>
#include <string>
#include <fstream>

enum class model_type {
	MSA_MODEL = 1, MDASM_MODEL = 2
};

struct compute_settings {
	std::string measuring_file;
	std::string spaceweather_file;
	std::string gravity_file;
	std::string output_file;
	std::string datetime;
	size_t harmonics{};
	model_type model{};
};

std::istream& operator>> (std::istream& istr, compute_settings& cs);

std::ofstream open_outfile(const std::string& filename);
std::ifstream open_infile(const std::string& filename);

#define nameof(x) #x