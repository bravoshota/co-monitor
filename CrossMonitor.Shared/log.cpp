#include "log.hpp"

#include <boost/log/utility/setup/file.hpp>

#define LOG CROSSOVER_MONITOR_LOG

namespace crossover {
namespace monitor {
namespace log {

void init() noexcept {	
	//Left blank for now
}
void set_file(const std::string& filename) noexcept {
	try {
		boost::log::add_file_log(filename);
		LOG(info) << "Added file log: " << filename;
	} catch (const std::exception& e) {
		LOG(error) << e.what();
	}
}

} //namespace log
} //namespace monitor
} //namespace crossover
