#include <Windows.h>
#include <iostream>
class console {

private:
	WORD attributes;
	HANDLE std_handle;
	CONSOLE_SCREEN_BUFFER_INFO Info;
public:
	inline void init_console() {
		console::std_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (console::std_handle == INVALID_HANDLE_VALUE)
			return;
		GetConsoleScreenBufferInfo(console::std_handle, &Info);
		attributes = Info.wAttributes;



	}
	inline void set_console_title(const char* title) {
		DWORD console;
		if (GetConsoleMode(console::std_handle, &console) == FALSE) {
			console |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		}
		console |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(console::std_handle, console);
		SetConsoleTitleA(title);

	}
	inline void log_error() {
		SetConsoleTextAttribute(console::std_handle, BACKGROUND_RED | BACKGROUND_INTENSITY);

	}
	inline void log_info() {

		SetConsoleTextAttribute(console::std_handle, BACKGROUND_BLUE | BACKGROUND_INTENSITY);
	}

	inline void reset_console() {
		SetConsoleTextAttribute(console::std_handle, console::attributes);

	}

};

