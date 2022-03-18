#include <CyUSBSerial/CyController.hpp>
#include <iostream>
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

struct AppSettings
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

static ExitCodeEnum parse_cmdline_args(int argc, char* argv[], AppSettings& r_app_settings_out)
{
  ExitCodeEnum enum_exit_code = ExitCodeEnum::SUCCESS;

  static constexpr char a_short_options[] = "-:aXc:d:";

  int option_index = 0;  // O
  static struct option a_long_options[] = {
    { "help", no_argument, nullptr, 0 },
    { nullptr, 0, nullptr, 0 }
  };

  int opt = ::getopt_long(argc, argv, a_short_options, a_long_options, &option_index);

  while (opt != -1)
  {
    switch (opt)
    {
      case 0:  // long option
        std::cout << "[ INFO] Option " << a_long_options[option_index].name << " was provided";
        if(::optarg != nullptr)
        {
          std::cout << " with argument " << ::optarg;
        }
        std::cout << std::endl;
        break;

      case 1:  // regular non-option argument
        std::cout << "[ERROR] Argument " << ::optarg << " unrecognized--Aborting!" << std::endl;
        enum_exit_code = ExitCodeEnum::UNKNOWN_CMDLINE_ARG;
        break;

      case 'a':
        std::cout << "[ INFO] Option a was provided" << std::endl;
        r_app_settings_out.a = true;
        break;

      case 'X':
        std::cout << "[ INFO] Option X was provided" << std::endl;
        r_app_settings_out.X = true;
        break;

      case 'c':
        std::cout << "[ INFO] Option c has argument " << ::optarg << std::endl;
        enum_exit_code = ::parse_arg_as_integer(opt, ::optarg, r_app_settings_out.c);
        break;

      case 'd':
        std::cout << "[ INFO] Option d has argument " << ::optarg << std::endl;
        enum_exit_code = ::parse_arg_as_integer(opt, ::optarg, r_app_settings_out.d);
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

    opt = ::getopt_long(argc, argv, a_short_options, a_long_options, &option_index);
  }

  return enum_exit_code;
}

static void print_usage()
{
  std::cout << "Usage: cygpio-hammer [options]...\n"
               "Hammer CyUSB GPIO lines, 0->1->0->1...\n"
               "  -n <name>    Hammer CyUSB GPIOs on a named device (must be stated)\n"
               "  -o <n>       Offset[s] to hammer, at least one, several can be stated\n"
               " [-c <n>]      Do <n> loops (optional, infinite loop if not stated)\n"
               "  -?, --help   This helptext\n"
               "\n"
               "Example:\n"
               "cygpio-hammer -n gpiochip0 -o 4\n"
            << std::endl;
}

int main(int argc, char* argv[])
{
  AppSettings r_app_settings;  // O

  {
    ExitCodeEnum exit_code = ::parse_cmdline_args(argc, argv, r_app_settings);
    if (exit_code != ExitCodeEnum::SUCCESS)
    {
      return static_cast<int>(exit_code);
    }
  }

  cyusb::CyController cyusb_controller;

  std::cout << "[ INFO] Command-line options: {"
            << (r_app_settings.a ? " a," : " !a,")
            << (r_app_settings.X ? " X," : " !X,")
            << " c=" << r_app_settings.c
            << ", d=" << r_app_settings.d
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

  return static_cast<int>(ExitCodeEnum::SUCCESS);
}
