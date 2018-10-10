#include <application.hpp>
#include <os.hpp>

#include <log.hpp>
#include <utils.hpp>
#include <data.hpp>

#include <cpprest/json.h>
#include <cpprest/http_client.h>

#include <atomic>
#include <string>
#include <stdexcept>
#include <numeric>

#define LOG CROSSOVER_MONITOR_LOG

using namespace std;

namespace crossover {
namespace monitor {
namespace client {

using namespace web;

class application::impl final {
private:
	atomic<bool> m_stop;
	atomic<bool> m_running;
	const std::chrono::minutes m_period;
	OnCollectedDataHandler m_onCollectedData;
	data m_collectedData;

public:
	impl(const chrono::minutes& period, OnCollectedDataHandler onCollectedData)
		: m_stop (false)
		, m_running(false)
		, m_period(period)
		, m_onCollectedData(onCollectedData) {
	}

private:
	void collect_data() {
		m_collectedData.set_cpu_percent(os::cpu_use_percent());
		m_collectedData.set_process_count(os::process_count());
		m_collectedData.set_memory_percent(os::memory_use_percent());

		// avoid copying array
		os::disk_io_stats(m_collectedData.get_io_stats_for_edit());
	}

public:
	void run() {
		if (m_running) {
			LOG(warning) << "application::run already running, ignoring call";
			return;
		}

		m_running = true;

		LOG(info) << "Starting application loop";

		const chrono::milliseconds resolution(100);

		do {
			try {
				collect_data();
				m_onCollectedData(m_collectedData.to_json());
			}
			catch (const std::exception& e) {
				LOG(error) << "Failed to collect and send data to server: "
					<< e.what();
			}
		} while (utils::interruptible_sleep(m_period, resolution, m_stop) !=
			utils::interruptible_sleep_result::interrupted);

		m_stop = false;
		m_running = false;

		LOG(info) << "Exiting application loop";
	}

	void stop() noexcept {
		if (m_running) {
			LOG(info) << "Stop requested, waiting for tasks to finish";
			m_stop = true;
		}
	}

	void send_data(const string& url, const string& key) const {
		const uri url_full(utility::conversions::to_string_t(url));

		uri_builder builder;
		builder.set_scheme(url_full.scheme());
		builder.set_user_info(url_full.user_info());
		builder.set_host(url_full.host());
		builder.set_port(url_full.port());

		const uri base(builder.to_uri());

		LOG(debug) << "Base URI: " << base.to_string()
			<< " - URI resource: " << url_full.resource().to_string();

		// Actual sending is out of scope...
		// so reporting unconditional success here
		LOG(info) << "Data sent successfully to " << url;
	}

	void report_sent_callback(const data& sent_data) const {
		static unsigned reports_sent = 0;
		static float latest_cpu_values[10] = { 0 };
		static mutex m;

		lock_guard<mutex> lock(m);

		latest_cpu_values[reports_sent % 10] = sent_data.get_cpu_percent();

		// Every 10 reports sent - show statistics about mean CPU usage
		// Skip first time hit
		if ((++reports_sent) % 10 == 0 && reports_sent != 0)
		{
			auto sum = std::accumulate(std::begin(latest_cpu_values), std::end(latest_cpu_values), 0.0f);
			auto mean = sum / 10.0f;

			LOG(info) << "Mean CPU usage: " << mean;
		}
	}
};

void application::collectedDataDefaultHandler(const web::json::value &collected_data) {
	LOG(info) << collected_data.serialize();
}

application::application(const chrono::minutes& period, OnCollectedDataHandler onCollectedData)
	: m_impl(new impl(period, onCollectedData)) {
	if (period < chrono::minutes(1)) {
		throw invalid_argument("Invalid arguments to application constructor");
	}

	// initialize CPU and HDD performance statistics queries at the beginning
	os::init_cpu_use_percent();
	os::init_disk_io_stats();

	LOG(info) << "application constructed successfully";
}

application::~application() {
	// CPU query needs this call (PdhCloseQuery)
	os::uninit_cpu_use_percent();
	LOG(info) << "application destructed successfully";
}

void application::run() {
	m_impl->run();
}

void application::stop() noexcept {
	m_impl->stop();

	// it is needed if console is terminated via close (red) button
	// because application instance is not destructing that case.
	// the extra call of this function is safe:
	os::uninit_cpu_use_percent();
}

} //namespace client
} //namespace monitor
} //namespace crossover

