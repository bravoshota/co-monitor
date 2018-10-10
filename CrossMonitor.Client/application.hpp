#pragma once

#include <cpprest/json.h>
#include <boost/noncopyable.hpp>

#include <memory>
#include <chrono>
#include <functional>

namespace crossover {
namespace monitor {
namespace client {

/**
 * Class handling main application logic.
 * Call run() after construction to run main logic.
 */
class application final: public boost::noncopyable {
public:
	typedef std::function<void(const web::json::value &collected_data)> OnCollectedDataHandler;

private:
	static void collectedDataDefaultHandler(const web::json::value &collected_data);

public:
	/**
	 * Constructs a ready to use application object.
	 * May throw std::exception derived exceptions.
	 * @param period minutes between reports.
	 * @param onCollectedData called each time when data is collected.
	 * The caller should take care what to do with collected performance data.
	 * In case of scipping this parameter the default handler will be used.
	 */
	application(const std::chrono::minutes& period,
				OnCollectedDataHandler onCollectedData = collectedDataDefaultHandler);
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
	class impl;

	std::unique_ptr<impl> m_impl;
}; //class application

} //namespace client
} //namespace monitor
} //namespace crossover
