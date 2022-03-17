#include <CyUSBSerial/CyController.hpp>
#include <iostream>
#include <type_traits>
#include <getopt.h>

enum class ExitCodeEnum : int
{
  SUCCESS = 0,
  FAILED_TO_INITIALIZE = -100,
  UNKNOWN_CMDLINE_ARG,
  UNKNOWN_CMDLINE_OPTION,
  OPTION_ARG_PARSE_ERROR,
  MISSING_OPTION_ARG,
  CMDLINE_PARSE_ERROR
};

struct CommandLineOptions
{
  bool a{};
  bool X{};
  int c{ 1 };
  int d{ 1234 };
};

static ExitCodeEnum parse_arg_as_integer(char argname, const char* p_argvalue, int& r_value_out)
{
  errno = 0;
  char* p_endptr;  // O

  r_value_out = std::strtol(p_argvalue, &p_endptr, 10);

  if ((errno != 0) || (p_endptr == p_argvalue))
  {
    std::cout << "[ERROR] Failed to parse option " << argname
              << " with value " << p_argvalue << " as integer--Aborting!" << std::endl;
    return ExitCodeEnum::OPTION_ARG_PARSE_ERROR;
  }

  return ExitCodeEnum::SUCCESS;
}

static ExitCodeEnum parse_cmdline_args(int argc, char* argv[], CommandLineOptions& r_command_line_options_out)
{
  ExitCodeEnum enum_exit_code = ExitCodeEnum::SUCCESS;

  static constexpr char aOptions[] = "-:aXc:d:";
  int opt = ::getopt(argc, argv, aOptions);

  while (opt != -1)
  {
    switch (opt)
    {
      case 1:
        std::cout << "[ERROR] Argument " << ::optarg << " unrecognized--Aborting!" << std::endl;
        enum_exit_code = ExitCodeEnum::UNKNOWN_CMDLINE_ARG;
        break;

      case 'a':
        std::cout << "[ INFO] Option a was provided" << std::endl;
        r_command_line_options_out.a = true;
        break;

      case 'X':
        std::cout << "[ INFO] Option X was provided" << std::endl;
        r_command_line_options_out.X = true;
        break;

      case 'c':
        std::cout << "[ INFO] Option c has argument " << ::optarg << std::endl;
        enum_exit_code = ::parse_arg_as_integer(opt, ::optarg, r_command_line_options_out.c);
        break;

      case 'd':
        std::cout << "[ INFO] Option d has argument " << ::optarg << std::endl;
        enum_exit_code = ::parse_arg_as_integer(opt, ::optarg, r_command_line_options_out.d);
        break;

      case '?':
        if (std::isprint(::optopt))
        {
          std::cout << "[ERROR] Option " << static_cast<char>(::optopt) << " unrecognized--Aborting!" << std::endl;
        }
        else
        {
          std::cout << "[ERROR] Option 0x" << std::hex << ::optopt << " unrecognized--Aborting!" << std::endl;
        }
        enum_exit_code = ExitCodeEnum::UNKNOWN_CMDLINE_OPTION;
        break;

      case ':':
        if (std::isprint(::optopt))
        {
          std::cout << "[ERROR] Option " << static_cast<char>(::optopt) << " missing an argument--Aborting!" << std::endl;
        }
        else
        {
          std::cout << "[ERROR] Option 0x" << std::hex << ::optopt << " missing an argument--Aborting!" << std::endl;
        }
        enum_exit_code = ExitCodeEnum::MISSING_OPTION_ARG;
        break;

      default:
        std::cout << "[ERROR] Command-line argument parse error--Aborting!" << std::endl;
        enum_exit_code = ExitCodeEnum::CMDLINE_PARSE_ERROR;
        break;
    }

    if (enum_exit_code != ExitCodeEnum::SUCCESS)
    {
      std::cout << "[ERROR] Command-line argument parse error--Aborting!" << std::endl;
      return enum_exit_code;
    }

    opt = ::getopt(argc, argv, aOptions);
  }

  return enum_exit_code;
}

int main(int argc, char* argv[])
{
  CommandLineOptions command_line_options;  // O

  {
    ExitCodeEnum exit_code = ::parse_cmdline_args(argc, argv, command_line_options);
    if (exit_code != ExitCodeEnum::SUCCESS)
    {
      return static_cast<int>(exit_code);
    }
  }

  cyusb::CyController cyusb_controller;

  std::cout << "[ INFO] Command-line options: {"
            << (command_line_options.a ? " a," : " !a,")
            << (command_line_options.X ? " X," : " !X,")
            << " c=" << command_line_options.c
            << ", d=" << command_line_options.d
            << " }" << std::endl;

  if (!cyusb_controller.initialize())
  {
    if (cyusb_controller.get_number_of_devices() > 0)
    {
      std::cout << "[ WARN] No connected CyUSB devices found" << std::endl;
    }

    std::cout << "[ERROR] Failed to initialize CyUSB controller--Aborting!" << std::endl;
    return static_cast<int>(ExitCodeEnum::FAILED_TO_INITIALIZE);
  }

  const std::string str_serial = "RTF001";
  cyusb_controller.set_working_device_by_serial(str_serial);

  std::cout << "::::::::::::::::::::::::GPIO Value for GPIO 5 ::::::::::::::::::::::::::::::::" << std::endl;
  std::cout << cyusb_controller.get_gpio_value(5) << std::endl;

  cyusb_controller.switch_gpio_state(5);
  std::cout << "::::::::::::::::::::::::GPIO Value for GPIO 5 ::::::::::::::::::::::::::::::::" << std::endl;
  std::cout << cyusb_controller.get_gpio_value(5) << std::endl;

  return 0;
}
