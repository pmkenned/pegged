#         | Input from system           | Makefile variables        | Preprocessor #defines
#         | $OS           uname         | KERNEL      ENV           |  MINGW   CYGWIN  LINUX    OSX
# ========|=============================|===========================|==============================
# mingw   | Windows_NT    MINGW32_NT-*  | Windows     MINGW32_NT    |  1
# cygwin  | Windows_NT    CYGWIN_NT-*   | Windows     CYGWIN_NT     |          1
# WSL     | (undefined)   Linux         | Linux       Linux         |                  1
# Mac     | (undefined)   Darwin        | Darwin      Darwin        |                           1

UNAME := $(shell uname | grep -oE "MINGW32_NT|CYGWIN_NT|Linux|Darwin")

ifeq ($(OS),Windows_NT)
	KERNEL := Windows
else
	KERNEL := $(UNAME)
endif
ENV := $(UNAME)

ifeq ($(KERNEL),Windows)
	ifeq ($(ENV), CYGWIN_NT)
		CPPFLAGS += -D CYGWIN
	endif
	ifeq ($(ENV), MINGW32_NT)
		CPPFLAGS += -D MINGW -D _WIN32_WINNT=_WIN32_WINNT_WIN7
	endif
endif
ifeq ($(KERNEL),Linux)
	CPPFLAGS += -D LINUX
endif
ifeq ($(KERNEL),Darwin)
	CPPFLAGS += -D OSX
endif
