#include "os.hpp"

#include "log.hpp"

#include <Windows.h>
#include <Psapi.h>
#include <Pdh.h>
#include <pdhmsg.h>

#include <vector>
#include <mutex>
#include <thread>

#define LOG CROSSOVER_MONITOR_LOG

using namespace std;

namespace crossover {
namespace monitor {
namespace client {
namespace os {

PDH_HQUERY query = NULL;
PDH_HCOUNTER cpu_counter;
std::vector<IO_stat<__int64>> volumes_data;

static unsigned process_count_helper(size_t max) noexcept {
	//This vector should always be of type DWORD, beware of the code below
	//that uses the size of DWORD to compute the max size of the internal
	//buffer if you change this!
	auto process_ids = std::make_unique<vector<DWORD>>(max);
	DWORD needed = 0;

	if (!EnumProcesses(process_ids->data(),
					   static_cast<DWORD>(process_ids->size() * sizeof(DWORD)),
					   &needed)) {
		LOG(error) << "Failed to enumerate processes, code: "
				   << GetLastError();
		return 0;
	}

	if (process_ids->size() * sizeof(DWORD) == needed) {
		//Increase the maximum number of processes and repeat the call.
		LOG(info) << "process_count_helper called with a maximum number of "
					 "processes too small (" << max << "), repeating the "
					 "call with a bigger limit (" << max * 2 << ")";
		process_ids.release();
		return process_count_helper(max * 2);
	}

	process_ids.release();
	return needed / sizeof(DWORD);
}

unsigned process_count() noexcept {
	return process_count_helper(1024 * 5);
}

bool init_cpu_use_percent() noexcept {
	PDH_STATUS status;
	
	status = PdhOpenQuery(NULL, NULL, &query);
	if (status != ERROR_SUCCESS) {
		LOG(error) << "PdhOpenQuery returned error code: " << status;
		return false;
	}

	status = PdhAddEnglishCounter(query,
		L"\\Processor(_Total)\\% Processor Time",
		NULL,
		&cpu_counter);
	if (status != ERROR_SUCCESS) {
		LOG(error) << "PdhAddEnglishCounter returned error code: " << status;
		return false;
	}

	status = PdhCollectQueryData(query);
	if (status != ERROR_SUCCESS) {
		LOG(error) << "PdhCollectQueryData returned error code: " << status;
		return false;
	}

	return true;
}

void uninit_cpu_use_percent() noexcept {
	if (query) {
		PDH_STATUS status = PdhCloseQuery(query);
		if (status != ERROR_SUCCESS)
			LOG(error) << "PdhCloseQuery returned error code: " << status;

		query = NULL;
	}
}

float cpu_use_percent() noexcept {
	static mutex m;
	const lock_guard<mutex> guard(m);

	PDH_FMT_COUNTERVALUE value;
	PDH_STATUS status;
	DWORD counter_type;
	if ((status = PdhCollectQueryData(query)) != ERROR_SUCCESS) {
		LOG(error) << "Error collecting CPU usage data, code: " << status;
		return 0;
	}
	if((status = PdhGetFormattedCounterValue(cpu_counter, 
											PDH_FMT_DOUBLE, 
											&counter_type,
											&value)) != ERROR_SUCCESS) {
		LOG(error) << "Error formatting CPU usage data, code: " << status;
		return 0;
	}
	return static_cast<float>(value.doubleValue);
}

float memory_use_percent() noexcept {
	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(mem);
	
	if (!GlobalMemoryStatusEx(&mem)) {
		LOG(error) << "Failed to get memory info, code: " << GetLastError();
		return 0;
	}

	const float available = 100.0f * mem.ullAvailPhys / mem.ullTotalPhys;
	const float used = 100 - available;
	return used;
}

void init_disk_io_stats() noexcept {
	const DWORD mydrives = 64;
	wchar_t lpBuffer[mydrives];
	DWORD test = GetLogicalDriveStringsW(mydrives, lpBuffer);

	volumes_data.clear();
	DWORD index = 0;
	while (index < test) {
		volumes_data.push_back({-1, -1, lpBuffer[index] });
		index += wcslen(lpBuffer + index) + 1;
	}
}

void disk_io_stats(IO_stats &io_stats) noexcept {
	io_stats.clear();
	io_stats.reserve(volumes_data.size());

	for (auto &single_volume_data : volumes_data) {
		wchar_t query[16] = L"";
		wsprintf(query, L"\\\\.\\%c:", single_volume_data.partition_name);
		HANDLE dev = CreateFile(query,
			FILE_READ_ATTRIBUTES,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, 0, NULL);
		if (dev == INVALID_HANDLE_VALUE) {
			continue;
		}

		DISK_PERFORMANCE disk_info;
		DWORD bytes;

		if(!DeviceIoControl(dev, IOCTL_DISK_PERFORMANCE, NULL,
			0, &disk_info, sizeof(disk_info), &bytes, NULL)) {
			continue;
		}

		if (disk_info.BytesRead.QuadPart == 0 && disk_info.BytesWritten.QuadPart == 0)
			continue;

		IO_stat<unsigned> res_data;
		res_data.partition_name = single_volume_data.partition_name;
		if (single_volume_data.bytes_read < 0 || single_volume_data.bytes_written < 0) {
			res_data.bytes_read = 0;
			res_data.bytes_written = 0;
		}
		else {
			res_data.bytes_read = static_cast<unsigned>(disk_info.BytesRead.QuadPart - single_volume_data.bytes_read);
			res_data.bytes_written = static_cast<unsigned>(disk_info.BytesWritten.QuadPart - single_volume_data.bytes_written);
		}
		io_stats.push_back(std::move(res_data));
		single_volume_data.bytes_read = disk_info.BytesRead.QuadPart;
		single_volume_data.bytes_written = disk_info.BytesWritten.QuadPart;
	}
}

} //namespace os
} //namespace client
} //namespace monitor
} //namespace crossover

