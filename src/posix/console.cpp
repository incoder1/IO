#include "stdafx.hpp"
#include "console.hpp"

namespace io {

static const char* NAVY_BLUE = "";
static const char* NAVY_GREEN = "";
static const char* NAVY_AQUA = "";
static const char* NAVY_RED = "";
static const char* MAGENTA = "";
static const char* BROWN = "";
static const char* WHITE = "";
static const char* GRAY = "";
static const char* LIGHT_BLUE = "";
static const char* LIGHT_GREEN = "";
static const char* LIGHT_AQUA = "";
static const char* LIGHT_RED = "";
static const char* LIGHT_PURPLE = "";
static const char* YELLOW = "";
static const char* BRIGHT_WHITE = "";

//text_color

// console
std::atomic<console*> console::_instance(nullptr);
critical_section console::_init_cs;


} // namespace io
