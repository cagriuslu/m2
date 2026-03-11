#include <m2/Log.h>
#include <m2/LogHelpers.h>
#include <m2/M2.h>
#include <cstdarg>
#include <thread>
#ifdef _WIN32
// Not yet supported
#else
#include <execinfo.h>
#endif

namespace {
	bool gUnexpectedEventOccurred = false;
	std::vector<std::pair<std::thread::id, std::string>> thread_names;

	const char* LookupThreadName() {
		const auto id = std::this_thread::get_id();
		for (const auto& pair : thread_names) {
			if (pair.first == id) {
				return pair.second.c_str();
			}
		}
		return "  ";
	}

	void SimplifyFileName(const char* fileName, std::array<char, 4>& out) {
		const auto fileNameLen = strlen(fileName);
		auto outIndex = 0zu;

		// Fill with capital letters
		for (auto i = 0zu; i < fileNameLen; ++i) {
			if (const char c = fileName[i]; isupper(c) || isdigit(c)) {
				out[outIndex++] = c;
				if (outIndex == out.size() - 1) {
					break;
				}
			}
		}
		// Fill remaining spaces with '-'
		for (auto i = outIndex; i < out.size() - 1; ++i) {
			out[i] = '-';
		}
		// Final null character
		out[out.size() - 1] = 0;
	}
}

std::mutex m2::detail::gLogMutex;

void m2::SetThreadNameForLogging(const char* thread_name) {
	thread_names.emplace_back(std::this_thread::get_id(), thread_name);
}
void m2::LogStacktrace() {
#ifdef _WIN32
	// Not yet supported
#else
	void* callstack[128];
	const int frames = backtrace(callstack, 128);
	char** strs = backtrace_symbols(callstack, frames);
	for (int i = 0; i < frames; ++i) {
		fprintf(stderr, "%s\n", strs[i]);
	}
	free(strs);
#endif
}
const std::string& m2::ToString(const pb::LogLevel& lvl) {
	return LogLevel_Name(lvl);
}

void m2::detail::LogHeader(const pb::LogLevel lvl, const char *filePath, const int line) {
	const auto lvl_int = static_cast<int>(lvl);

	// Set unexpected event
	if (static_cast<int>(pb::LogLevel::WRN) <= lvl_int && !gUnexpectedEventOccurred) {
		gUnexpectedEventOccurred = true;
	}

	// Get time
	const auto now = std::time(nullptr) - 1609459200ull; // 2021

	// Convert log level into char
	char lvl_char = 'U';
	if (lvl_int <= static_cast<int>(pb::LogLevel::FTL)) {
		static constexpr char lvl_chars[] = {0, 'T', 'G', 'P', 'N', 'D', 'I', 'W', 'E', 'F'};
		static constexpr char lvl_chars_un[] = {0, 't', 'g', 'p', 'n', 'd', 'i', 'w', 'e', 'f'};
		lvl_char = gUnexpectedEventOccurred ? lvl_chars_un[lvl_int] : lvl_chars[lvl_int];
	}

	// File name
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
	const auto fileName = strrchr(filePath, '\\');
#else
	const auto fileName = strrchr(filePath, '/');
#endif
	std::array<char, 4> simplifiedFileName;
	SimplifyFileName(fileName == nullptr ? filePath : fileName, simplifiedFileName);

	fprintf(stderr, "[%c %010lld %s %03d %s] ", lvl_char, static_cast<long long>(now), simplifiedFileName.data(), line % 1000, LookupThreadName());
}

#if _MSC_VER > 1400
void m2::detail::LogF(const pb::LogLevel lvl, const char* file, const int line, _Printf_format_string_ const char* fmt, ...) {
#else
void m2::detail::LogF(const pb::LogLevel lvl, const char* file, const int line, const char* fmt, ...) {
#endif
	if (lvl < current_log_level) {
		return;
	}
	std::unique_lock lock{gLogMutex};
	LogHeader(lvl, file, line);
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}
