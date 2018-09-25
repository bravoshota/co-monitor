#include "log.hpp"

#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/expressions/formatters/format.hpp>
#include <iostream>

#define LOG CROSSOVER_MONITOR_LOG

namespace crossover {
namespace monitor {
namespace log {

using namespace boost::log;

void init() noexcept {
	add_console_log(std::clog);
}
void set_file(const std::string& filename) noexcept {
	try {
		//formatter logFmt = expressions::format("%1%") % expressions::smessage;

		auto file_logger = add_file_log
		(
			keywords::file_name = filename,
			keywords::rotation_size = 10 * 1024 * 1024,
			keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0)
		);

		//file_logger->set_formatter(logFmt);
		file_logger->locked_backend()->auto_flush(true);

		LOG(info) << "Added file log: " << filename;
	} catch (const std::exception& e) {
		LOG(error) << e.what();
	}
}

} //namespace log
} //namespace monitor
} //namespace crossover
