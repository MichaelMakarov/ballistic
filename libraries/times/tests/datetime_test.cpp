#include <datetime.h>
#include <gtest/gtest.h>

using namespace times;

class datetime_test : public ::testing::Test {
protected:
	struct datetime_values {
		llong_t year;
		ushort_t month, day, hour, minute, second, millisec;
	};
	datetime make_datetime(const datetime_values& values) {
		return times::make_datetime(values.year, values.month, values.day, values.hour, values.minute, values.second, values.millisec);
	}
	void verify_datetime(const datetime& dt, const datetime_values& values) {
		EXPECT_EQ(dt.d.y, values.year);
		EXPECT_EQ(dt.d.m, values.month);
		EXPECT_EQ(dt.d.d, values.day);
		EXPECT_EQ(dt.t.h, values.hour);
		EXPECT_EQ(dt.t.m, values.minute);
		EXPECT_EQ(dt.t.s, values.second);
		EXPECT_EQ(dt.t.ms, values.millisec);
	}

protected:
	const datetime_values init_values[5]{
		{ 2000, 01, 03, 23, 23, 23, 423 },
		{ 2000, 01, 03, 00, 00, 01, 000 },
		{ 2013, 11, 23, 01, 56, 34, 100 },
		{ 2013, 11, 23, 11, 05, 07, 000 },
		{ 2013, 11, 23, 20, 23, 19, 000 }
	};

protected:
	void SetUp() override {

	}
};


TEST_F(datetime_test, handling_expected_initialization) {
	for (size_t i{}; i < sizeof(init_values) / sizeof(init_values[0]); ++i) {
		verify_datetime(make_datetime(init_values[i]), init_values[i]);
	}
	EXPECT_ANY_THROW(times::make_datetime(1, -1, 1, 1, 1, 1, 1));
	EXPECT_ANY_THROW(times::make_datetime(1, 1, -1, 1, 1, 1, 1));
	EXPECT_ANY_THROW(times::make_datetime(1, 1, 1, -1, 1, 1, 1));
	EXPECT_ANY_THROW(times::make_datetime(1, 1, 1, 1, -1, 1, 1));
	EXPECT_ANY_THROW(times::make_datetime(1, 1, 1, 1, 1, -1, 1));
	EXPECT_ANY_THROW(times::make_datetime(1, 1, 1, 1, 1, 1, -1));
	EXPECT_NO_THROW(times::make_datetime(1, 1, 1, 1, 1, 1, 1));
}
//
TEST_F(datetime_test, handling_expected_comparison) {
	auto dt1 = make_datetime(init_values[1]);
	auto dt2 = dt1;
	auto dt3 = make_datetime(init_values[0]);

	EXPECT_TRUE(dt1 == dt2);
	EXPECT_FALSE(dt1 != dt2);
	EXPECT_TRUE(dt3 > dt1);
	EXPECT_FALSE(dt2 > dt3);
	EXPECT_TRUE(dt2 < dt3);
	EXPECT_FALSE(dt1 < dt2);
	EXPECT_TRUE(dt1 <= dt2);
	EXPECT_TRUE(dt3 >= dt2);
}

TEST_F(datetime_test, handling_parsing_from_string) {
	datetime dt;
	EXPECT_NO_THROW(dt = times::make_datetime("2010-12-11 23:34:56.23", "y-M-d h:m:s.f"));
	verify_datetime(dt, { 2010, 12, 11, 23, 34, 56, 230 });
	EXPECT_NO_THROW(dt = times::make_datetime("13/11/2021 13:1:3.111111", "d/M/y h:m:s.f"));
	verify_datetime(dt, { 2021, 11, 13, 13, 01, 03, 111 });
	EXPECT_NO_THROW(dt = times::make_datetime("30.06.1997", "d.M.y"));
	verify_datetime(dt, { 1997, 06, 30, 00, 00, 00, 000 });
	EXPECT_NO_THROW(dt = times::make_datetime("08:43:55.005", "h:m:s.f"));
	verify_datetime(dt, { 0000, 01, 01, 8, 43, 55, 005 });
	EXPECT_NO_THROW(dt = times::make_datetime("45:|: 07 :: 0", "m:|: h :: s"));
	verify_datetime(dt, { 0000, 01, 01, 07, 45, 00, 000 });
	EXPECT_NO_THROW(dt = times::make_datetime("06/01/2001 06:10:31", "dd/MM/yyyy hh:mm:ss"));
	verify_datetime(dt, { 2001, 01, 06, 06, 10, 31, 000 });
}
