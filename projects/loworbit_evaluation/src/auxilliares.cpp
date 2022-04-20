#include <auxilliares.h>


std::istream& operator>>(std::istream& istr, compute_settings& cs)
{
	std::getline(istr, cs.measuring_file);
	std::getline(istr, cs.datetime);
	std::getline(istr, cs.gravity_file);
	istr >> cs.harmonics;
	istr.get();
	std::getline(istr, cs.spaceweather_file);
	istr >> reinterpret_cast<size_t&>(cs.model);
	istr.get();
	std::getline(istr, cs.output_file);
	return istr;
}

std::ofstream open_outfile(const std::string& filename)
{
	std::ofstream fout{ filename };
	if (!fout.is_open()) throw std::invalid_argument("failed to open file " + filename);
	return fout;
}

std::ifstream open_infile(const std::string& filename)
{
	std::ifstream fin{ filename };
	if (!fin.is_open()) throw std::invalid_argument("failed to open file " + filename);
	return fin;
}
