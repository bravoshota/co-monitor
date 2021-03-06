#include "log.hpp"

#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/expressions.hpp>

#define LOG CROSSOVER_MONITOR_LOG

namespace crossover {
namespace monitor {
namespace log {

using namespace boost::log;

const formatter logFmt = expressions::format("[%1%] [%2%] [%3%] %4%")
	% expressions::format_date_time<boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.f")
	% expressions::attr<attributes::current_thread_id::value_type>("ThreadID")
	% trivial::severity
	% expressions::smessage;

void init() noexcept {
	try {
		add_common_attributes();

		auto logger = add_console_log(std::clog);
		logger->set_formatter(logFmt);

		LOG(info) << "Added Console log";
	}
	catch (const std::exception& e) {
		LOG(error) << e.what();
	}
}
void set_file(const std::string& filename) noexcept {
	try {
		auto logger = add_file_log(
			keywords::file_name = filename,
			keywords::auto_flush = true
		);
		logger->set_formatter(logFmt);

		LOG(info) << "Added file log: " << filename;
	} catch (const std::exception& e) {
		LOG(error) << e.what();
	}
}

} //namespace log
} //namespace monitor
} //namespace crossover
