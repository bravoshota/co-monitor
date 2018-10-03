#include <os.hpp>

namespace crossover {
namespace monitor {
namespace client {
namespace os {

// Mock values
unsigned int _process_count = 0;
float _cpu_use_percent = 0;
float _memory_use_percent = 0;
IO_stats _disk_io_stats;

bool init_cpu_use_percent() noexcept {
	return true;
}

void init_disk_io_stats() noexcept {
}

void set_process_count(unsigned int n) {
	_process_count = n;
}

unsigned process_count() noexcept {
	return _process_count;
}

void set_cpu_use_percent(float percent) {
	_cpu_use_percent = percent;
}

float cpu_use_percent() noexcept {
	return _cpu_use_percent;
}

void set_memory_use_percent(float percent) {
	_memory_use_percent = percent;
}

float memory_use_percent() noexcept {
	return _memory_use_percent;
}

IO_stats disk_io_stats() noexcept {
	return _disk_io_stats;
}

void uninit_cpu_use_percent() noexcept {
}

} //namespace os
} //namespace client
} //namespace monitor
} //namespace crossover