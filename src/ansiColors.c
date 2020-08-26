#include "humon/ansiColors.h"
#include "humon.internal.h"

#if HUMON_TERMINALCOLORS == HUMON_TERMINALCOLORS_NOCOLOR

char const * ansi_off = "";
char const * ansi_black = "";
char const * ansi_darkRed = "";
char const * ansi_darkGreen = "";
char const * ansi_darkYellow = "";
char const * ansi_darkBlue = "";
char const * ansi_darkMagenta = "";
char const * ansi_darkCyan = "";
char const * ansi_darkGray = "";
char const * ansi_lightGray = "";
char const * ansi_lightRed = "";
char const * ansi_lightGreen = "";
char const * ansi_lightYellow = "";
char const * ansi_lightBlue = "";
char const * ansi_lightMagenta = "";
char const * ansi_lightCyan = "";
char const * ansi_white = "";

char const * ansi_blackBg = "";
char const * ansi_darkRedBg = "";
char const * ansi_darkGreenBg = "";
char const * ansi_darkYellowBg = "";
char const * ansi_darkBlueBg = "";
char const * ansi_darkMagentaBg = "";
char const * ansi_darkCyanBg = "";
char const * ansi_darkGrayBg = "";
char const * ansi_lightGrayBg = "";
char const * ansi_lightRedBg = "";
char const * ansi_lightGreenBg = "";
char const * ansi_lightYellowBg = "";
char const * ansi_lightBlueBg = "";
char const * ansi_lightMagentaBg = "";
char const * ansi_lightCyanBg = "";
char const * ansi_whiteBg = "";

char const * ansi_darkDarkRedBg      = "";
char const * ansi_darkDarkGreenBg    = "";
char const * ansi_darkDarkYellowBg   = "";
char const * ansi_darkDarkOrangeBg   = "";
char const * ansi_darkDarkBlueBg     = "";
char const * ansi_darkDarkMagentaBg  = "";
char const * ansi_darkDarkCyanBg     = "";
char const * ansi_darkDarkGrayBg     = "";

#elif HUMON_TERMINALCOLORS == HUMON_TERMINALCOLORS_ANSI

char const * ansi_off = "\033[0m";
char const * ansi_black = "\033[30m";
char const * ansi_darkRed = "\033[31m";
char const * ansi_darkGreen = "\033[32m";
char const * ansi_darkYellow = "\033[33m";
char const * ansi_darkBlue = "\033[34m";
char const * ansi_darkMagenta = "\033[35m";
char const * ansi_darkCyan = "\033[36m";
char const * ansi_darkGray = "\033[37m";
char const * ansi_lightGray = "\033[90m";
char const * ansi_lightRed = "\033[91m";
char const * ansi_lightGreen = "\033[92m";
char const * ansi_lightYellow = "\033[93m";
char const * ansi_lightBlue = "\033[94m";
char const * ansi_lightMagenta = "\033[95m";
char const * ansi_lightCyan = "\033[96m";
char const * ansi_white = "\033[97m";

char const * ansi_blackBg = "\033[48;2;0;0;0m";
char const * ansi_darkRedBg = "\033[41m";
char const * ansi_darkGreenBg = "\033[42m";
char const * ansi_darkYellowBg = "\033[43m";
char const * ansi_darkBlueBg = "\033[44m";
char const * ansi_darkMagentaBg = "\033[45m";
char const * ansi_darkCyanBg = "\033[46m";
char const * ansi_darkGrayBg = "\033[47m";
char const * ansi_lightGrayBg = "\033[100m";
char const * ansi_lightRedBg = "\033[101m";
char const * ansi_lightGreenBg = "\033[102m";
char const * ansi_lightYellowBg = "\033[103m";
char const * ansi_lightBlueBg = "\033[104m";
char const * ansi_lightMagentaBg = "\033[105m";
char const * ansi_lightCyanBg = "\033[106m";
char const * ansi_whiteBg = "\033[107m";

char const * ansi_darkDarkRedBg      = "\033[48;2;63;0;0m";
char const * ansi_darkDarkGreenBg    = "\033[48;2;0;63;0m";
char const * ansi_darkDarkYellowBg   = "\033[48;2;63;63;0m";
char const * ansi_darkDarkOrangeBg   = "\033[48;2;63;31;0m";
char const * ansi_darkDarkBlueBg     = "\033[48;2;0;0;63m";
char const * ansi_darkDarkMagentaBg  = "\033[48;2;63;0;63m";
char const * ansi_darkDarkCyanBg     = "\033[48;2;0;63;63m";
char const * ansi_darkDarkGrayBg     = "\033[48;2;63;63;63m";

#endif
