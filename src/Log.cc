#include <m2/Log.h>
#include <m2/M2.h>
#include <cstdarg>
#include <thread>
#include <execinfo.h>

namespace {
	bool unexpected_event_occured = false;
	std::vector<std::pair<std::thread::id, std::string>> thread_names;

	const char* lookup_thread_name() {
		auto id = std::this_thread::get_id();
		for (const auto& pair : thread_names) {
			if (pair.first == id) {
				return pair.second.c_str();
			}
		}
		return "  ";
	}
}

std::mutex m2::detail::log_mutex;
thread_local int m2::detail::thread_indentation{0};

void m2::set_thread_name_for_logging(const char* thread_name) {
	thread_names.emplace_back(std::this_thread::get_id(), thread_name);
}

void m2::log_stacktrace() {
#ifdef _WIN32
	// Not yet supported
#else
	void* callstack[128];
	int frames = backtrace(callstack, 128);
	char** strs = backtrace_symbols(callstack, frames);
	for (int i = 0; i < frames; ++i) {
		fprintf(stderr, "%s\n", strs[i]);
	}
	free(strs);
#endif
}

const std::string& m2::to_string(const pb::LogLevel& lvl) {
	return pb::LogLevel_Name(lvl);
}

void m2::detail::log_header(pb::LogLevel lvl, const char *file, int line) {
	auto lvl_int = int(lvl);

	// Set unexpected event
	if (int(pb::LogLevel::WRN) <= lvl_int && !unexpected_event_occured) {
		unexpected_event_occured = true;
	}

	// Get time
	auto now = std::time(nullptr) - 1672531200ull; // 2023

	// Convert log level into char
	char lvl_char = 'U';
	if (lvl_int <= int(pb::LogLevel::FTL)) {
		static const char lvl_chars[] = {0, 'T', 'D', 'I', 'W', 'E', 'F'};
		static const char lvl_chars_un[] = {0, 't', 'd', 'i', 'w', 'e', 'f'};
		lvl_char = unexpected_event_occured ? lvl_chars_un[lvl_int] : lvl_chars[lvl_int];
	}

	// Get file name
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
	auto file_name_wo_dirs = strrchr(file, '\\');
#else
	auto file_name_wo_dirs = strrchr(file, '/');
#endif
	const char* file_name = (file_name_wo_dirs == nullptr) ? file : file_name_wo_dirs;
	// Simplify file name
	char file_name_capitals[4] = { 0 };
	int file_name_capitals_len = 0;
	for (size_t i = 0; i < strlen(file_name) && file_name_capitals_len < 3; i++) {
		char c = file_name[i];
		if (isupper(c) || isdigit(c)) {
			file_name_capitals[file_name_capitals_len++] = c;
		}
	}
	static const char* file_name_paddings[] = {
		"---", "--", "-", ""
	};
	const char* file_name_padding = file_name_paddings[file_name_capitals_len];

	fprintf(stderr, "[%c %09lld %s%s %03d %s] ", lvl_char, (long long)now, file_name_padding, file_name_capitals, line % 1000, lookup_thread_name());
}

#if _MSC_VER > 1400
void m2::detail::logf(pb::LogLevel lvl, const char* file, int line, _Printf_format_string_ const char* fmt, ...) {
#else
void m2::detail::logf(pb::LogLevel lvl, const char* file, int line, const char* fmt, ...) {
#endif
	if (lvl < current_log_level) {
		return;
	}
	log_header(lvl, file, line);
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");
}
