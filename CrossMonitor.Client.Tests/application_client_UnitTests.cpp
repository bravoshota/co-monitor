#include "CppUnitTest.h"

#include <application.hpp>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CrossMonitorClientTests
{		
	TEST_CLASS(application_client_UnitTests)
	{
	public:
		
		TEST_METHOD(HavingZeroPeriod_ShouldThrow)
		{
			using std::chrono::minutes;
			using crossover::monitor::client::application;

			Assert::ExpectException<std::invalid_argument>([]() {
				application a{ minutes(0) };
			});
		}

	};
}