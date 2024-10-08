## Haiku Generic Makefile v2.6 ##

# The name of the binary.
NAME = i2c_touchpad

# The type of binary, must be one of:
#	APP:	Application
#	SHARED:	Shared library or add-on
#	STATIC:	Static library archive
#	DRIVER: Kernel driver
TYPE = DRIVER

# If you plan to use localization, specify the application's MIME signature.
APP_MIME_SIG =

#	Specify the source files to use. Full paths or paths relative to the
#	Makefile can be included. All files, regardless of directory, will have
#	their object files created in the common object directory. Note that this
#	means this Makefile will not work correctly if two source files with the
#	same name (source.c or source.cpp) are included from different directories.
SRCS = \
	i2c_driver.cpp \
	i2c_controller.cpp \
	i2c_device.cpp \
	i2c_touchpad.cpp

#	Specify the resource definition files to use. Full or relative paths can be
#	used.
RDEFS =

#	Specify the resource files to use. Full or relative paths can be used.
#	Both RDEFS and RSRCS can be utilized in the same Makefile.
RSRCS =

# Specify libraries to link against.
LIBS =

#	Specify additional paths to directories following the standard libXXX.so
#	or libXXX.a naming scheme. You can specify full paths or paths relative
#	to the Makefile. The paths included are not parsed recursively, so
#	include all of the paths where libraries must be found. Directories where
#	source files were specified are	automatically included.
LIBPATHS =

#	Additional paths to look for system headers. These use the form
#	"#include <header>". Directories that contain the files in SRCS are
#	NOT auto-included here.

SYSTEM_INCLUDE_PATHS = \
    $(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY kernel) \
    $(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY kernel/drivers) \
    $(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY private/kernel/arch/x86) \
    /boot/system/develop/headers \
    /boot/system/develop/headers/os/drivers/bus \
    /boot/system/develop/headers/clang/Driver \
    /boot/system/develop/headers/bus  # Questo � il percorso per bus/Driver.h

#	Additional paths paths to look for local headers. These use the form
#	#include "header". Directories that contain the files in SRCS are
#	automatically included.
LOCAL_INCLUDE_PATHS = \
	.

#	Specify the level of optimization that you want. Specify either NONE (O0),
#	SOME (O1), FULL (O2), or leave blank (for the default optimization level).
OPTIMIZE :=

# 	Specify the codes for languages you are going to support in this
# 	application. The default "en" one must be provided too. "make catkeys"
# 	will recreate only the "locales/en.catkeys" file. Use it as a template
# 	for creating catkeys for other languages. All localization files must be
# 	placed in the "locales" subdirectory.
LOCALES =

#	Specify all the preprocessor symbols to be defined. The symbols will not
#	have their values set automatically; you must supply the value (if any) to
#	use. For example, setting DEFINES to "DEBUG=1" will cause the compiler
#	option "-DDEBUG=1" to be used. Setting DEFINES to "DEBUG" would pass
#	"-DDEBUG" on the compiler's command line.
DEFINES = \
	_KERNEL_MODE \
	__HAIKU__

#	Specify the warning level. Either NONE (suppress all warnings),
#	ALL (enable all warnings), or leave blank (enable default warnings).
WARNINGS = ALL

#	With image symbols, stack crawls in the debugger are meaningful.
#	If set to "TRUE", symbols will be created.
SYMBOLS := TRUE

#	Includes debug information, which allows the binary to be debugged easily.
#	If set to "TRUE", debug info will be created.
DEBUGGER := TRUE

#	Specify any additional compiler flags to be used.
COMPILER_FLAGS = -fno-exceptions -fno-rtti

#	Specify any additional linker flags to be used.
LINKER_FLAGS =

#	Specify the version of this binary. Example:
#		-app 3 4 0 d 0 -short 340 -long "340 "`echo -n -e '\302\251'`"1999 GNU GPL"
#	This may also be specified in a resource.
APP_VERSION :=

#	(Only used when "TYPE" is "DRIVER"). Specify the desired driver install
#	location in the /dev hierarchy. Example:
#		DRIVER_PATH = video/usb
#	will instruct the "driverinstall" rule to place a symlink to your driver's
#	binary in ~/add-ons/kernel/drivers/dev/video/usb, so that your driver will
#	appear at /dev/video/usb when loaded. The default is "misc".
DRIVER_PATH = input/touchpad

## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine