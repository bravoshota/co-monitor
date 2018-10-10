#include "log.hpp"

#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
//#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/expressions/formatters/format.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
//#include <boost/date_time/posix_time/posix_time_types.hpp>


#define LOG CROSSOVER_MONITOR_LOG

namespace crossover {
namespace monitor {
namespace log {

using namespace boost::log;

const formatter logFmt = expressions::format("<%1%> %2%")
//	% expressions::format_date_time<boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
	% trivial::severity
	% expressions::smessage;

void init() noexcept {
	//add_common_attributes();
	auto file_logger = add_console_log(std::clog);
	file_logger->set_formatter(logFmt);
}
void set_file(const std::string& filename) noexcept {
	try {
		auto file_logger = add_file_log(
			keywords::file_name = filename,
			keywords::auto_flush = true
		);
		file_logger->set_formatter(logFmt);

		LOG(info) << "Added file log: " << filename;
	} catch (const std::exception& e) {
		LOG(error) << e.what();
	}
}

} //namespace log
} //namespace monitor
} //namespace crossover
