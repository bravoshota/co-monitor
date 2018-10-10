#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include <cpprest/json.h>

namespace crossover {
namespace monitor {

template<typename T>
struct IO_stat {
	T bytes_read;
	T bytes_written;
	wchar_t partition_name;
};
typedef std::vector<IO_stat<unsigned>> IO_stats;

/**
 * Class representing the data sent and received 
 * by both client and server components.
 */
class data final {
public:
	/**
	 * Constructor. Throws std::invalid_argument in case any arguments
	 * are out of range.
	 * @param cpu_percent CPU use percentage (0 to 100).
	 * @param memory_percent Physical memory use percentage (0 to 100).
	 * @param process_count Number of processes (1 to UINT_MAX).
	 */
	data(	float cpu_percent,
			float memory_percent,
			unsigned process_count,
			const IO_stats &io_stats) {
		set_cpu_percent(cpu_percent);
		set_memory_percent(memory_percent);
		set_process_count(process_count);
		set_io_stats(io_stats);
	}

	data()
		: cpu_percent_(-1.f) {
	}

	data& operator=(const data& rhs) = default;	

	/**
	 * Setter. Throws std::invalid_argument if the argument is out of range.
	 * @param cpu_percent CPU use percentage (0 to 100).
	 */
	void set_cpu_percent(float cpu_percent) {
		if (cpu_percent < 0 || cpu_percent > 100) {
			throw std::invalid_argument(
				"cpu_percent out of range: " + std::to_string(cpu_percent));
		}
		cpu_percent_ = cpu_percent_ < 0.f ? 0.f : cpu_percent;
	}
	float get_cpu_percent() const noexcept {
		return cpu_percent_;
	}

	/**
	* Setter. Throws std::invalid_argument if the argument is out of range.
	* @param memory_percent Memory use percentage (0 to 100).
	*/
	void set_memory_percent(float memory_percent) {
		if (memory_percent < 0 || memory_percent > 100) {
			throw std::invalid_argument(
				"memory_percent out of range: " + std::to_string(memory_percent));
		}
		memory_percent_ = memory_percent;
	}
	float get_memory_percent() const noexcept {
		return memory_percent_;
	}

	/**
	* Setter. Throws std::invalid_argument if the argument is out of range.
	* @param process_count Process count (1 to UINT_MAX).
	*/
	void set_process_count(unsigned process_count) {
		if (process_count == 0) {
			throw std::invalid_argument("process_count cannot be zero");
		}
		process_count_ = process_count;
	}
	unsigned get_process_count() const noexcept {
		return process_count_;
	}

	/**
	* Setter. Throws std::invalid_argument if the argument is out of range.
	* @param io_stats Bytes read and bytes written (0 to unsigned-max).
	*/
	void set_io_stats(const IO_stats &io_stats) {
		for (auto &stat : io_stats) {
			if (stat.bytes_read < 0) {
				throw std::invalid_argument("bytes_read must be positive number");
			}
			if (stat.bytes_written < 0) {
				throw std::invalid_argument("bytes_written must be positive number");
			}
		}
		io_stats_ = io_stats;
	}

	/**
	* Getter. get IO statistics.
	*/
	const IO_stats &get_io_stats() const noexcept {
		return io_stats_;
	}

	/**
	* Getter. get IO statistics for edit.
	*/
	IO_stats &get_io_stats_for_edit() noexcept {
		return io_stats_;
	}

	/**
	* convert data into JSON format.
	*/
	web::json::value to_json() const {
		web::json::value out;
		out[L"cpu_percent"] = cpu_percent_;
		out[L"process_count"] = process_count_;
		out[L"memory_percent"] = memory_percent_;

		std::vector<web::json::value> parts;
		for (const auto &io_stat : io_stats_) {
			web::json::value part_details;
			part_details[L"bytes_read"] = io_stat.bytes_read;
			part_details[L"bytes_written"] = io_stat.bytes_written;

			web::json::value part;
			std::wstring str(&io_stat.partition_name, 1);
			part[str] = part_details;

			parts.push_back(part);
		}

		if (!parts.empty()) {
			out[L"volumme_io"] = web::json::value::array(parts);
		}

		return out;
	}

private:
	float cpu_percent_;
	float memory_percent_;
	unsigned process_count_;
	IO_stats io_stats_;
}; //struct data

} //namespace monitor
} //namespace crossover
