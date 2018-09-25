#pragma once

#include <functional>

namespace crossover {
namespace monitor {
namespace os {

/**
 * Registers a single function object to be called when 
 * the OS requests the program to exit. Ctrl-C and system 
 * shutdown are handled.
 */
void set_termination_handler(const std::function<void()>& handler) noexcept;

} //namespace os
} //namespace monitor
} //namespace crossover
