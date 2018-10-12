#include "CppUnitTest.h"

#include <data.hpp>
#include <os_mock.hpp>
#include <application.hpp>

#include <thread>
#include <iostream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CrossMonitorClientTests
{
	/**
	 * fill and return utils::data instance
	 */
	TEST_CLASS(application_client_UnitTests)
	{
		const crossover::monitor::data &getData() {
			static crossover::monitor::data res;
			res.set_cpu_percent(1.f);
			res.set_process_count(34);
			res.set_memory_percent(55.f);
			res.set_io_stats({
				{ 1123, 3321, L'C' },
				{ 0, 3321, L'D' },
				{ 1, 0, L'E' },
				{ 0, 0, L'F' }
			});

			return res;
		}
	public:
		
		/**
		* check utils::data::to_json()
		*
		* 1. call utils::data::to_json()
		* 2. parse converted json::value and compare to each value held in utils::data
		*/
		TEST_METHOD(DataToJson)
		{
			const crossover::monitor::data &original_data = getData();
			web::json::value converted_json = original_data.to_json();

			Assert::IsTrue(converted_json.is_object(), L"converted_json is not object");

			const web::json::object &obj = converted_json.as_object();
			Assert::IsTrue(obj.size() == 4, L"obj.size() != 4");

			// keys should be in alphabetical order
			auto iter = obj.cbegin();
			Assert::AreEqual(iter->first.c_str(), L"cpu_percent", L"iter->first != cpu_percent");
			Assert::IsTrue(iter->second.as_double() == original_data.get_cpu_percent(),
				L"iter->second.as_double() != original_data.get_cpu_percent()");
			++iter;
			Assert::AreEqual(iter->first.c_str(), L"memory_percent", L"iter->first != memory_percent");
			Assert::IsTrue(iter->second.as_double() == original_data.get_memory_percent(),
				L"iter->second.as_double() != original_data.get_memory_percent()");
			++iter;
			Assert::AreEqual(iter->first.c_str(), L"process_count", L"iter->first != process_count");
			Assert::IsTrue(iter->second.as_integer() == original_data.get_process_count(),
				L"iter->second.as_integer() != original_data.get_process_count()");
			++iter;
			Assert::AreEqual(iter->first.c_str(), L"volumme_io", L"(iter->first != volumme_io");
			Assert::IsTrue(iter->second.is_array(), L"iter->second is not array");

			// now check the conversion of volume IO statistics in details
			const auto &arr_orig = original_data.get_io_stats();
			web::json::array arr_conv = iter->second.as_array();
			Assert::IsTrue(arr_conv.size() == arr_orig.size(), L"arr_conv.size() != arr_orig.size()");

			// and for every record
			for (size_t i = 0; i < arr_conv.size(); ++i) {
				Assert::IsTrue(arr_conv[i].is_object(), L"arr_conv[i] is not object");
				const web::json::object &obj1 = arr_conv[i].as_object();
				Assert::IsTrue(obj1.size() == 1, L"obj1.size() != 1");
				auto iter1 = obj1.cbegin();
				Assert::AreEqual(std::wstring(&arr_orig[i].partition_name, 1), iter1->first,
					L"arr_orig[i].partition_name != iter1->first");
				Assert::IsTrue(iter1->second.is_object(), L"iter1->second is not object");
				const web::json::object &obj2 = iter1->second.as_object();
				Assert::IsTrue(obj2.size() == 2, L"obj2.size() != 2");
				auto iter2 = obj2.cbegin();
				Assert::AreEqual(iter2->first.c_str(), L"bytes_read", L"iter2->first != bytes_read");
				Assert::IsTrue(iter2->second.as_integer() == arr_orig[i].bytes_read);
				++iter2;
				Assert::AreEqual(iter2->first.c_str(), L"bytes_written", L"bytes_read", L"iter2->first != bytes_written");
				Assert::IsTrue(iter2->second.as_integer() == arr_orig[i].bytes_written,
					L"(iter2->second.as_integer() != arr_orig[i].bytes_written");
			}
		}

		/**
		 * check application::run() if it throws exception on incorrect period value passed
		 */
		TEST_METHOD(HavingZeroPeriod_ShouldThrow)
		{
			using crossover::monitor::client::application;

			Assert::ExpectException<std::invalid_argument>([]() {
				application a{ std::chrono::minutes(0) };
			});
		}

		/**
		 * check application::run() functionality
		 *
		 * 1. initialize queries at the beginning
		 * 2. call run method in different thread and pass the user defined handler
		 *    to check correctness of calculated collected data
		 * 3. request application::stop() after first check
		 * 4. wait while run()'s thread is joined to main thread
		 */
		TEST_METHOD(CheckCollectData)
		{
			using namespace crossover::monitor;
			using namespace crossover::monitor::client;

			Logger::WriteMessage("start of unit test");

			const data &expected_data = getData();
			os::set_process_count(expected_data.get_process_count());
			os::set_cpu_use_percent(expected_data.get_cpu_percent());
			os::set_memory_use_percent(expected_data.get_memory_percent());
			os::set_disk_io_stats(expected_data.get_io_stats());

			bool request_stop = false;

			application app {std::chrono::minutes(1), [&](const web::json::value &collected_data) {
				Logger::WriteMessage("lambda: enter");

				if (request_stop)
					return;

				auto collected_str = collected_data.serialize();
				auto expected_str = expected_data.to_json().serialize();
				wchar_t str[1024];
				swprintf_s(str, L"collected_data = %s", collected_str.c_str());
				Logger::WriteMessage(str);
				swprintf_s(str, L"expected_data = %s", expected_str.c_str());
				Logger::WriteMessage(str);

				Assert::AreEqual(collected_str, expected_str, L"collected_str != expected_str");

				request_stop = true;
				Logger::WriteMessage("lambda: leave");
			}};

			std::thread thr([&]() {
				app.run();
			});

			// wait for first data collection call
			while (!request_stop) {
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			// and request stop
			app.stop();
			Logger::WriteMessage("stop requested");

			// wait until app is fully stopped
			thr.join();

			Logger::WriteMessage("thread joined. end of unit test");
		}

	};
}