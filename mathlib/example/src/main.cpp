#include <exception>
#include <iostream>

void test_vec();
void test_mat();
void test_vector();
void test_matrix();
void test_poly();
void test_quaternion();

int main()
{
	try
	{
		test_vec();
		test_mat();
		test_vector();
		test_matrix();
		test_poly();
		test_quaternion();
		std::cout << "All tests are completed.\n";
	}
	catch (std::exception const &ex)
	{
		std::cout << ex.what() << std::endl;
	}
}
