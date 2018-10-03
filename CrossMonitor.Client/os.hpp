#pragma once

#include "../CrossMonitor.Shared/os.hpp"
#include "../CrossMonitor.Shared/data.hpp"

namespace crossover {
namespace monitor {
namespace client {
namespace os {

/**
* Init CPU use percent. Call it once on start.
*/
bool init_cpu_use_percent() noexcept;
/**
* Init IO statistics for logical drives.
*/
void init_disk_io_stats() noexcept;

/**
 * Gets the number of currently running processes.
 */
unsigned process_count() noexcept;
/**
* Gets CPU use percentage (0 to 100).
*/
float cpu_use_percent() noexcept;
/**
* Gets physical memory use percentage (0 to 100).
*/
float memory_use_percent() noexcept;
/**
* Gets IO statistics of logical drives.
*/
void disk_io_stats(IO_stats &io_stats) noexcept;

/**
* Uninit CPU use percent. Call it once on finish.
*/
void uninit_cpu_use_percent() noexcept;

} //namespace os
} //namespace client
} //namespace monitor
} //namespace crossover
