#include <juliandate.h>
#include <gtest/gtest.h>

using namespace times;



class jd_test : public ::testing::Test {
protected:
	struct jd_values {
		llong_t day;
		double time;
	};
	void verify_jd(const juliandate& jd, const jd_values& values) {
		EXPECT_EQ(jd.day, values.day);
		EXPECT_TRUE(std::fabs(jd.time - values.time) < std::numeric_limits<double>::epsilon());
	}

	void SetUp() override {
		dt1 = make_datetime(2000, 01, 03, 23, 23, 23, 423);
		dt2 = make_datetime(2013, 12, 23, 11, 05, 07, 000);
	}
protected:
	datetime dt1, dt2;
};

TEST_F(jd_test, handling_expected_initialization) {
	verify_jd(make_juliandate(JD2000), { llong_t(std::floor(JD2000)), 0.5 });
	verify_jd(make_juliandate(dt1), { 2451547, 0.9745766550925926142 });
	verify_jd(make_juliandate(dt2), { 2456650, 0.4618865740740740189 });
}

TEST_F(jd_test, conversion_to_datetime) {
	EXPECT_EQ(make_datetime(make_juliandate(dt1)), dt1);
	EXPECT_EQ(make_datetime(make_juliandate(dt2)), dt2);
}
