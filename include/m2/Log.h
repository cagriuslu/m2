#ifndef M2_LOG_H
#define M2_LOG_H

#include <sstream>
#include <string>

#if _MSC_VER > 1400
#include <sal.h>
#endif

namespace m2 {
	enum class LogLevel {
		Trace = 0,
		Debug,
		Info,
		Warn,
		Error,
		Fatal
	};

	extern LogLevel current_log_level;

	void _log_header(LogLevel lvl, const char* file, int line);

#if _MSC_VER > 1400
	void logf(LogLevel lvl, const char* file, int line, _Printf_format_string_ const char* fmt, ...);
#else
	void logf(LogLevel lvl, const char* file, int line, const char* fmt, ...) __attribute__ ((format (printf, 4, 5)));
#endif

	template <typename ...Ts>
	void log(LogLevel lvl, const char* file, int line, const char* msg, const Ts& ...ts) {
		if (lvl < current_log_level) {
			return;
		}
		_log_header(lvl, file, line);
		fprintf(stderr, "%s ", msg);
		([&] {
			using namespace std;
			using namespace m2;
			fprintf(stderr, "%s ", to_string(ts).c_str());
		}, ...);
		fprintf(stderr, "\n");
	}

	void log_stacktrace();
}

#endif //M2_LOG_H
