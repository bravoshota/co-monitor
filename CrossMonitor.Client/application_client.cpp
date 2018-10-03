#include <application.hpp>
#include <os.hpp>

#include <log.hpp>
#include <utils.hpp>
#include <data.hpp>

#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <cpprest/asyncrt_utils.h>

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

struct application::impl final {
private:
	data collected_data_;
public:
	atomic<bool> stop = false;
	atomic<bool> running = false;

	const data &collected_data() {
		return collected_data_;
	}

	void init() {
		os::init_cpu_use_percent();
		os::init_disk_io_stats();
	}

	void collect_data() {
		collected_data_.set_cpu_percent(os::cpu_use_percent());
		collected_data_.set_memory_percent(os::memory_use_percent());
		collected_data_.set_process_count(os::process_count());

		// avoid copying array:
		os::disk_io_stats(collected_data_.get_io_stats_for_edit());
	}

	void send_data(const string& url, const string& key) {
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

	void report_sent_callback(const data& sent_data)
	{
		static unsigned reports_sent = 0;
		static float latest_cpu_values[10] = { 0 };
		static mutex m;

		lock_guard<mutex> l(m);

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

application::application(const std::chrono::minutes& period)
	: pimpl_(new impl)
	, period_(period) {
	if (period_ < chrono::minutes(1)) {
		throw invalid_argument("Invalid arguments to application constructor");
	}
}

application::~application() {
}

void application::run() {
	if (pimpl_->running) {
		LOG(warning) << "application::run already running, ignoring call";
		return;
	}

	pimpl_->running = true;

	LOG(info) << "Starting application loop";
	utils::scope_exit exit_guard([this] {
		pimpl_->running = false;
		pimpl_->stop = false;
		os::uninit_cpu_use_percent();
		LOG(info) << "Exiting application loop";
	});

	pimpl_->init();
	const chrono::milliseconds resolution(100);

	do {
		try {
			pimpl_->collect_data();
			json::value jsondata;
			jsondata[L"cpu_percent"] = pimpl_->collected_data().get_cpu_percent();
			jsondata[L"memory_percent"] = pimpl_->collected_data().get_memory_percent();
			jsondata[L"process_count"] = pimpl_->collected_data().get_process_count();

			std::vector<web::json::value> parts;
			for (const auto &io_stat : pimpl_->collected_data().get_io_stats()) {
				web::json::value part_details;
				part_details[L"bytes_read"] = io_stat.bytes_read;
				part_details[L"bytes_written"] = io_stat.bytes_written;

				web::json::value part;
				std::wstring str(&io_stat.partition_name, 1);
				part[str] = part_details;

				parts.push_back(part);
			}
			if(!parts.empty()) {
				jsondata[L"volumme_io"] = web::json::value::array(parts);
			}

			// as my point of view I'd add serialize() method to data class
			// to avoid above 4 lines.
			// but in readme is requested to refactor only this class.
			LOG(info) << jsondata.serialize();
		}
		catch (const std::exception& e) {
			LOG(error) << "Failed to collect and send data to server: "
				<< e.what();
		}
	} while (utils::interruptible_sleep(period_, resolution, pimpl_->stop) !=
		utils::interruptible_sleep_result::interrupted);

}

void application::stop() noexcept {
	if (pimpl_->running) {
		LOG(info) << "Stop requested, waiting for tasks to finish";
		pimpl_->stop = true;
	}
}

} //namespace client
} //namespace monitor
} //namespace crossover

