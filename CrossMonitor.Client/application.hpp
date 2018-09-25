#pragma once

#include <boost/noncopyable.hpp>

#include <memory>
#include <string>
#include <chrono>

namespace crossover {
namespace monitor {
namespace client {

/**
 * Class handling main application logic.
 * Call run() after construction to run main logic.
 */
class application final: public boost::noncopyable {
public:
	/**
	 * Constructs a ready to use application object.
	 * May throw std::exception derived exceptions.
	 * @param period minutes between reports.
	 */
	application(const std::chrono::minutes& period);
	~application();

	/**
	 * Runs the application logic. Blocking.
	 * Call stop() from any thread or signal handler to break from this
	 * call.
	 * May throw std::exception derived classes.
	 */
	void run();
	/**
	 * Call this from any thread or signal handler 
	 * to stop executing after using run().
	 */
	void stop() noexcept;

private:
	struct impl;

	std::unique_ptr<impl> pimpl_;
	const std::chrono::minutes period_;
}; //class application

} //namespace client
} //namespace monitor
} //namespace crossover
