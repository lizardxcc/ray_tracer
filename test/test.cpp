#define BOOST_TEST_MODULE SpectrumTest
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <iostream>
#include "spectrum.h"
#include "materialnode.h"

const double TOLERANCE = 0.0000001;


BOOST_AUTO_TEST_SUITE(simple_test_suite)
BOOST_AUTO_TEST_CASE(simple_test) {
	BOOST_CHECK_EQUAL(2+2, 4);
}


#define TEST1_SEQ (Type0)(Type1)(Type2)(Type3)
DEFINE_ENUM(TestType1, TEST1_SEQ, 0)
DEFINE_ENUM_FUNCTION(TestType1, TEST1_SEQ)
BOOST_AUTO_TEST_CASE(enumtype_test) {
	BOOST_CHECK_EQUAL("Type0", TestType1ToString(Type0));
	BOOST_CHECK_EQUAL("Type1", TestType1ToString(Type1));
	BOOST_CHECK_EQUAL("Type2", TestType1ToString(Type2));
	BOOST_CHECK_EQUAL("Type3", TestType1ToString(Type3));
	BOOST_CHECK_EQUAL(Type0, StringToTestType1("Type0"));
	BOOST_CHECK_EQUAL(Type1, StringToTestType1("Type1"));
	BOOST_CHECK_EQUAL(Type2, StringToTestType1("Type2"));
	BOOST_CHECK_EQUAL(Type3, StringToTestType1("Type3"));
}

#define TEST2_SEQ (Type20)(Type21)(Type22)(Type23)
DEFINE_ENUM(TestType2, TEST2_SEQ, 0)
DEFINE_ENUM_FUNCTION(TestType2, TEST2_SEQ)
BOOST_AUTO_TEST_CASE(enumtype_test2) {
	BOOST_CHECK_EQUAL("Type20", TestType2ToString(Type20));
	BOOST_CHECK_EQUAL("Type21", TestType2ToString(Type21));
	BOOST_CHECK_EQUAL("Type22", TestType2ToString(Type22));
	BOOST_CHECK_EQUAL("Type23", TestType2ToString(Type23));
	BOOST_CHECK_EQUAL(Type20, StringToTestType2("Type20"));
	BOOST_CHECK_EQUAL(Type21, StringToTestType2("Type21"));
	BOOST_CHECK_EQUAL(Type22, StringToTestType2("Type22"));
	BOOST_CHECK_EQUAL(Type23, StringToTestType2("Type23"));
}
/*
BOOST_AUTO_TEST_CASE(spectrum_test1) {
	FrequencyArbitrarySpectrum s;
	s.AddLast(1.0, FSPECTRUM_START, FSPECTRUM_END);
	FSpectrum result = s.ToFSpectrum();
	for (size_t i = 0; i < result.data.size(); i++) {
		//std::cout << i << " " << result.data[i] << std::endl;
		BOOST_TEST(result.data[i] == 1.0, boost::test_tools::tolerance(TOLERANCE));
	}
}
BOOST_AUTO_TEST_CASE(spectrum_test2) {
	FrequencyArbitrarySpectrum s;
	s.AddLast(1.0, FSPECTRUM_START, FSPECTRUM_START+FSPECTRUM_RANGE/2.0);
	s.AddLast(0.0, FSPECTRUM_START+FSPECTRUM_RANGE/2.0, FSPECTRUM_END);
	FSpectrum result = s.ToFSpectrum();
	for (size_t i = 0; i < result.data.size()/2; i++) {
		//std::cout << i << " " << result.data[i] << std::endl;
		BOOST_TEST(result.data[i] == 1.0, boost::test_tools::tolerance(TOLERANCE));
	}
	for (size_t i = result.data.size()/2; i < result.data.size(); i++) {
		//std::cout << i << " " << result.data[i] << std::endl;
		BOOST_TEST(result.data[i] == 0.0, boost::test_tools::tolerance(TOLERANCE));
	}
}
BOOST_AUTO_TEST_CASE(spectrum_test3) {
	FrequencyArbitrarySpectrum s;
	s.AddLast(1.0, FSPECTRUM_START, FSPECTRUM_START+FSPECTRUM_RANGE/2.0+FSPECTRUM_SAMPLE_SIZE/2.0);
	s.AddLast(0.0, FSPECTRUM_START+FSPECTRUM_RANGE/2.0+FSPECTRUM_SAMPLE_SIZE/2.0, FSPECTRUM_END);
	FSpectrum result = s.ToFSpectrum();
	for (size_t i = 0; i < result.data.size()/2; i++) {
		//std::cout << i << " " << result.data[i] << std::endl;
		BOOST_TEST(result.data[i] == 1.0, boost::test_tools::tolerance(TOLERANCE));
	}
	size_t ii = result.data.size()/2;
	//std::cout << ii << " " << result.data[ii] << std::endl;
	BOOST_TEST(result.data[ii] == 0.5, boost::test_tools::tolerance(TOLERANCE));
	for (size_t i = result.data.size()/2+1; i < result.data.size(); i++) {
		//std::cout << i << " " << result.data[i] << std::endl;
		BOOST_TEST(result.data[i] == 0.0, boost::test_tools::tolerance(TOLERANCE));
	}
}
BOOST_AUTO_TEST_CASE(spectrum_test4) {
	FrequencyArbitrarySpectrum s;
	s.AddLast(1.0, FSPECTRUM_START, FSPECTRUM_START+FSPECTRUM_RANGE/2.0+FSPECTRUM_SAMPLE_SIZE/2.0);
	s.AddLast(1.0, FSPECTRUM_START+FSPECTRUM_RANGE/2.0+FSPECTRUM_SAMPLE_SIZE/2.0, FSPECTRUM_END);
	FSpectrum result = s.ToFSpectrum();
	for (size_t i = 0; i < result.data.size(); i++) {
		//std::cout << i << " " << result.data[i] << std::endl;
		BOOST_TEST(result.data[i] == 1.0, boost::test_tools::tolerance(TOLERANCE));
	}
}
BOOST_AUTO_TEST_CASE(wavelength_constexpr_test) {
	for (size_t i = 0; i < FSPECTRUM_N_SAMPLE; i++) {
		BOOST_CHECK_EQUAL(wl_range_start_array.array[i], wl_range_start(i));
		BOOST_CHECK_EQUAL(wl_range_end_array.array[i], wl_range_end(i));
		BOOST_CHECK_EQUAL(wl_range_size_array.array[i], wl_range_size(i));
	}
}

BOOST_AUTO_TEST_CASE(data_conversion_test) {
	double freq_based_data = 2.5;
	double freq = 600; // THz
	double wl_based_data = convert_spec_data_from_freq_to_wl(freq_based_data, freq);
	double result= convert_spec_data_from_wl_to_freq(result, freq_to_wavelength(freq));
	BOOST_TEST(freq_based_data == result, boost::test_tools::tolerance(TOLERANCE));
}

BOOST_AUTO_TEST_CASE(spectrum_equallity_test) {
	FSpectrum a;
	a.Add(1.0, FSPECTRUM_START, FSPECTRUM_END);
	FSpectrum b = a.ToWSpectrum().ToFSpectrum();
	for (size_t i = 0; i < b.data.size(); i++) {
		BOOST_TEST(a.data[i] == b.data[i], boost::test_tools::tolerance(TOLERANCE));
	}
}

BOOST_AUTO_TEST_CASE(spectrum_equallity_test2) {
	FSpectrum a;
	a.Add(1.0, FSPECTRUM_START, FSPECTRUM_END);
	WSpectrum tmp = a.ToWSpectrum();
	FSpectrum c = a*0.5;
	FSpectrum b = (tmp * 0.5).ToFSpectrum();
	for (size_t i = 0; i < b.data.size(); i++) {
		BOOST_TEST(c.data[i] == b.data[i], boost::test_tools::tolerance(TOLERANCE));
	}
}

//BOOST_AUTO_TEST_CASE(arbitrary_wavelength_spectrum_test) {
//	WavelengthArbitrarySpectrums;
//	s.AddLast(1.0, WSPECTRUM_START, WSPECTRUM_END);
//	FSpectrum result = s.ToWSpectrum().ToFSpectrum();
//	for (size_t i = 0; i < result.data.size(); i++) {
//		//std::cout << i << " " << result.data[i] << std::endl;
//		BOOST_TEST(result.data[i] == 1.0, boost::test_tools::tolerance(0.0001));
//	}
//}
*/
BOOST_AUTO_TEST_SUITE_END()
