#include <m2/Log.h>
#include <m2/M2.h>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cstddef>
#include <cstdarg>

m2::LogLevel m2::current_log_level = m2::LogLevel::Debug;
bool m2::unexpected_event_occured = false;

std::string m2::to_string(const LogLevel& lvl) {
	auto lvl_unsigned = to_unsigned(lvl);
	if (lvl_unsigned <= to_unsigned(LogLevel::Fatal)) {
		static const std::string lvls[] = {"Trace", "Debug", "Info", "Warn", "Error", "Fatal"};
		return lvls[lvl_unsigned];
	}
	return "Unknown";
}

void m2::log_stacktrace() {
#ifdef _WIN32
	// Not yet supported
#elif __APPLE__
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

void m2::internal::log_header(LogLevel lvl, const char *file, int line) {
	auto lvl_unsigned = to_unsigned(lvl);

	// Set unexpected event
	if (to_unsigned(LogLevel::Warn) <= lvl_unsigned && !unexpected_event_occured) {
		unexpected_event_occured = true;
	}

	// Get time
	auto now = std::time(nullptr);

	// Convert log level into char
	char lvl_char = 'U';
	if (lvl_unsigned <= to_unsigned(LogLevel::Fatal)) {
		static const char lvl_chars[] = {'T', 'D', 'I', 'W', 'E', 'F'};
		static const char lvl_chars_un[] = {'t', 'd', 'i', 'w', 'e', 'f'};
		lvl_char = unexpected_event_occured ? lvl_chars_un[lvl_unsigned] : lvl_chars[lvl_unsigned];
	}

	// Get file name
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
	auto file_name_wo_dirs = strrchr(file, '\\');
#else
	auto file_name_wo_dirs = strrchr(file, '/');
#endif
	const char* file_name = (file_name_wo_dirs == nullptr) ? file : file_name_wo_dirs;
	// Simplify file name
	char file_name_capitals[6] = { 0 };
	int file_name_capitals_len = 0;
	for (size_t i = 0; i < strlen(file_name) && file_name_capitals_len < 5; i++) {
		char c = file_name[i];
		if (isupper(c) || isdigit(c)) {
			file_name_capitals[file_name_capitals_len++] = c;
		}
	}
	static const char* file_name_paddings[] = {
		"-----", "----", "---", "--", "-", ""
	};
	const char* file_name_padding = file_name_paddings[file_name_capitals_len];

	fprintf(stderr, "[%010lld:%c:%s%s:%05d] ", (long long)now, lvl_char, file_name_padding, file_name_capitals, line);
}

#if _MSC_VER > 1400
void m2::internal::logf(LogLevel lvl, const char* file, int line, _Printf_format_string_ const char* fmt, ...) {
#else
void m2::internal::logf(LogLevel lvl, const char* file, int line, const char* fmt, ...) {
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
