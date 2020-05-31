#ifndef ANSITERM_H
#define ANSITERM_H

#include <string>

namespace ansi
{
  constexpr auto off = "\033[0m";


  constexpr auto black = "\033[30m";
  constexpr auto darkRed = "\033[31m";
  constexpr auto darkGreen = "\033[32m";
  constexpr auto darkYellow = "\033[33m";
  constexpr auto darkBlue = "\033[34m";
  constexpr auto darkMagenta = "\033[35m";
  constexpr auto darkCyan = "\033[36m";
  constexpr auto darkGray = "\033[37m";
  constexpr auto lightGray = "\033[90m";
  constexpr auto lightRed = "\033[91m";
  constexpr auto lightGreen = "\033[92m";
  constexpr auto lightYellow = "\033[93m";
  constexpr auto lightBlue = "\033[94m";
  constexpr auto lightMagenta = "\033[95m";
  constexpr auto lightCyan = "\033[96m";
  constexpr auto white = "\033[97m";

  constexpr auto darkDarkRedBg      = "\033[48;2;63;0;0m";
  constexpr auto darkDarkGreenBg    = "\033[48;2;0;63;0m";
  constexpr auto darkDarkYellowBg   = "\033[48;2;63;63;0m";
  constexpr auto darkDarkOrangeBg   = "\033[48;2;63;31;0m";
  constexpr auto darkDarkBlueBg     = "\033[48;2;0;0;63m";
  constexpr auto darkDarkMagentaBg  = "\033[48;2;63;0;63m";
  constexpr auto darkDarkCyanBg     = "\033[48;2;0;63;63m";
  constexpr auto darkDarkGrayBg     = "\033[48;2;63;63;63m";

  constexpr auto blackBg = "\033[48;2;0;0;0m";
  constexpr auto darkRedBg = "\033[41m";
  constexpr auto darkGreenBg = "\033[42m";
  constexpr auto darkYellowBg = "\033[43m";
  constexpr auto darkBlueBg = "\033[44m";
  constexpr auto darkMagentaBg = "\033[45m";
  constexpr auto darkCyanBg = "\033[46m";
  constexpr auto darkGrayBg = "\033[47m";
  constexpr auto lightGrayBg = "\033[100m";
  constexpr auto lightRedBg = "\033[101m";
  constexpr auto lightGreenBg = "\033[102m";
  constexpr auto lightYellowBg = "\033[103m";
  constexpr auto lightBlueBg = "\033[104m";
  constexpr auto lightMagentaBg = "\033[105m";
  constexpr auto lightCyanBg = "\033[106m";
  constexpr auto whiteBg = "\033[107m";


/*
  constexpr auto black = "\033[0;30m";
  constexpr auto darkRed = "\033[0;31m";
  constexpr auto darkGreen = "\033[0;32m";
  constexpr auto darkYellow = "\033[0;33m";
  constexpr auto darkBlue = "\033[0;34m";
  constexpr auto darkMagenta = "\033[0;35m";
  constexpr auto darkCyan = "\033[0;36m";
  constexpr auto lightGray = "\033[0;37m";
  constexpr auto darkGray = "\033[1;30m";
  constexpr auto lightRed = "\033[1;31m";
  constexpr auto lightGreen = "\033[1;32m";
  constexpr auto lightYellow = "\033[1;33m";
  constexpr auto lightBlue = "\033[1;34m";
  constexpr auto lightMagenta = "\033[1;35m";
  constexpr auto lightCyan = "\033[1;36m";
  constexpr auto white = "\033[1;37m";

  constexpr auto blackBg = "\033[0;40m";
  constexpr auto darkRedBg = "\033[0;41m";
  constexpr auto darkGreenBg = "\033[0;42m";
  constexpr auto darkYellowBg = "\033[0;43m";
  constexpr auto darkBlueBg = "\033[0;44m";
  constexpr auto darkMagentaBg = "\033[0;45m";
  constexpr auto darkCyanBg = "\033[0;46m";
  constexpr auto lightGrayBg = "\033[0;47m";
  constexpr auto darkGrayBg = "\033[1;100m";
  constexpr auto lightRedBg = "\033[1;101m";
  constexpr auto lightGreenBg = "\033[1;102m";
  constexpr auto lightYellowBg = "\033[1;103m";
  constexpr auto lightBlueBg = "\033[1;104m";
  constexpr auto lightMagentaBg = "\033[1;105m";
  constexpr auto lightCyanBg = "\033[1;106m";
  constexpr auto whiteBg = "\033[1;107m";
  */
}

#endif // #ifndef ANSITERM_H
