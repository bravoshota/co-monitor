#pragma once

namespace crossover {
namespace monitor {
namespace client {
namespace os {

void set_process_count(unsigned int n);
void set_cpu_use_percent(float percent);
void set_memory_use_percent(float percent);
void set_disk_io_stats(const IO_stats &io_stats);

} //namespace os
} //namespace client
} //namespace monitor
} //namespace crossover
