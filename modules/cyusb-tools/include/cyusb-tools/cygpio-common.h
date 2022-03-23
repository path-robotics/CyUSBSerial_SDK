#ifndef __CYUSB_TOOLS__CYGPIO_COMMON_H
#define __CYUSB_TOOLS__CYGPIO_COMMON_H

#include <iostream>
#include <string>
#include <errno.h>

#ifdef ENABLE_DEBUGLOG
#define DEBUGLOG(x) std::cout << x
#else
#define DEBUGLOG(x)
#endif

enum class ExitCodeEnum : int
{
  SUCCESS = 0,
  FAILED_TO_INITIALIZE = -100,
  UNKNOWN_CMDLINE_ARG,
  UNKNOWN_CMDLINE_OPTION,
  OPTION_ARG_PARSE_ERROR,
  MISSING_OPTION_ARG,
  CMDLINE_PARSE_ERROR,
  PRINT_USAGE_AND_EXIT,
  MISSING_REQUIRED_ARG,
  GPIOCHIP_NOT_FOUND,
  FAILED_DEVICE_SELECTION,
  RUNTIME_EXCEPTION
};

static ExitCodeEnum parse_arg_as_integer(const char* p_argname, const char* p_argvalue, int& r_value_out)
{
  errno = 0;
  char* p_endptr;  // O

  r_value_out = std::strtol(p_argvalue, &p_endptr, 10);

  if ((errno != 0) || (p_endptr == p_argvalue))
  {
    std::cout << "[ERROR] Failed to parse option " << p_argname
              << " with value " << p_argvalue << " as integer--Aborting!" << std::endl;
    return ExitCodeEnum::OPTION_ARG_PARSE_ERROR;
  }

  return ExitCodeEnum::SUCCESS;
}

static ExitCodeEnum parse_arg_as_integer(char argname, const char* p_argvalue, int& r_value_out)
{
  const char a_argname[] = { argname, '\0' };
  return ::parse_arg_as_integer(a_argname, p_argvalue, r_value_out);
}

#endif
