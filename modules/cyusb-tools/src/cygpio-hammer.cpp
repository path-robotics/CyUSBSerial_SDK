#include <CyUSBSerial/CyController.hpp>
#include <CyUSBSerial/CyUSBSerial.hpp>
#include <iostream>
#include <cstring>
#include <getopt.h>

//#define ENABLE_DEBUGLOG

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
  PRINT_USAGE_AND_EXIT
};

struct AppSettings
{
  /*
  bool a{};
  bool X{};
  int c{ 1 };
  int d{ 1234 };
  */
  std::string str_named_device;
};

static void print_usage()
{
  std::cout << "Usage: cygpio-hammer [options]...\n"
               "Hammer CyUSB GPIO lines, 0->1->0->1...\n"
               "  -n <name>    Hammer CyUSB GPIOs on a named device (must be stated)\n"
               "  -o <n>       Offset[s] to hammer, at least one, several can be stated\n"
               " [-c <n>]      Do <n> loops (optional, infinite loop if not stated)\n"
               "  -h, --help   This helptext\n"
               "\n"
               "Example:\n"
               "cygpio-hammer -n gpiochip0 -o 4\n"
            << std::endl;
}

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

  static constexpr char a_short_options[] = "-:hn:o:c:";  //"-:aXc:d:";

  int option_index = 0;  // O
  static struct option a_long_options[] = {
    { "help", no_argument, nullptr, 'h' },
    { nullptr, 0, nullptr, 0 }
  };

  int opt = ::getopt_long(argc, argv, a_short_options, a_long_options, &option_index);

  while (opt != -1)
  {
    switch (opt)
    {
      case 0:  // long option
      {
        const char* p_cstr_long_argument_name = a_long_options[option_index].name;
        DEBUGLOG("[DEBUG] Option " << p_cstr_long_argument_name << " was provided");
        if (::optarg == nullptr)
        {
          DEBUGLOG(std::endl);
          /*
          if (std::strcmp(p_cstr_long_argument_name, "help") == 0)
          {
            enum_exit_code = ExitCodeEnum::PRINT_USAGE_AND_EXIT;
          }
          else
          */
          {
            std::cout << "[ERROR] Argument " << p_cstr_long_argument_name << " unrecognized--Aborting!" << std::endl;
            enum_exit_code = ExitCodeEnum::CMDLINE_PARSE_ERROR;
          }
        }
        else  // if(::optarg != nullptr)
        {
          DEBUGLOG(" with argument " << ::optarg << std::endl);
          std::cout << "[ERROR] Error parsing command-line argument \"--" << p_cstr_long_argument_name
                    << " with argument " << ::optarg << "\"--Aborting!" << std::endl;
          enum_exit_code = ExitCodeEnum::CMDLINE_PARSE_ERROR;
        }
      }
      break;

      case 1:  // regular non-option argument
        std::cout << "[ERROR] Argument " << ::optarg << " unrecognized--Aborting!" << std::endl;
        enum_exit_code = ExitCodeEnum::UNKNOWN_CMDLINE_ARG;
        break;

      case 'h':
        DEBUGLOG("[DEBUG] Option h was provided" << std::endl);
        enum_exit_code = ExitCodeEnum::PRINT_USAGE_AND_EXIT;
        break;

      case 'n':
        DEBUGLOG("[DEBUG] Option n was provided with argument " << ::optarg << std::endl);
        r_app_settings_out.str_named_device = ::optarg;
        break;

        /*
        case 'a':
          DEBUGLOG("[DEBUG] Option a was provided" << std::endl);
          //r_app_settings_out.a = true;
          break;

        case 'X':
          DEBUGLOG("[DEBUG] Option X was provided" << std::endl);
          //r_app_settings_out.X = true;
          break;

        case 'c':
          DEBUGLOG("[DEBUG] Option c has argument " << ::optarg << std::endl);
          //enum_exit_code = ::parse_arg_as_integer(opt, ::optarg, r_app_settings_out.c);
          break;

        case 'd':
          DEBUGLOG("[DEBUG] Option d has argument " << ::optarg << std::endl);
          //enum_exit_code = ::parse_arg_as_integer(opt, ::optarg, r_app_settings_out.d);
          break;
        */

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
      return enum_exit_code;
    }

    opt = ::getopt_long(argc, argv, a_short_options, a_long_options, &option_index);
  }

  return enum_exit_code;
}

int main(int argc, char* argv[])
{
  AppSettings app_settings;  // O

  {
    ExitCodeEnum enum_exit_code = ::parse_cmdline_args(argc, argv, app_settings);

    if (enum_exit_code != ExitCodeEnum::SUCCESS)
    {
      switch (enum_exit_code)
      {
        case ExitCodeEnum::PRINT_USAGE_AND_EXIT:
          enum_exit_code = ExitCodeEnum::SUCCESS;  // report success to caller
          break;
        default:
          std::cout << "[ERROR] Command-line argument parse error--Aborting!" << std::endl;
          break;
      }

      ::print_usage();

      return static_cast<int>(enum_exit_code);
    }
  }

  cyusb::CyController cyusb_controller;

  DEBUGLOG("[DEBUG] Command-line options: {"
           /*
           << (r_app_settings.a ? " a," : " !a,")
           << (r_app_settings.X ? " X," : " !X,")
           << " c=" << r_app_settings.c
           << ", d=" << r_app_settings.d
           */
           << " str_named_device=\"" << app_settings.str_named_device << "\""
           << " }" << std::endl);

  if (!cyusb_controller.initialize())
  {
    if (cyusb_controller.get_number_of_devices() > 0)
    {
      std::cout << "[ WARN] No connected CyUSB devices found" << std::endl;
    }

    std::cout << "[ERROR] Failed to initialize CyUSB controller--Aborting!" << std::endl;
    return static_cast<int>(ExitCodeEnum::FAILED_TO_INITIALIZE);
  }

  {
    int gpiochip_count{};

    uint8_t device_count;  // O
    CY_RETURN_STATUS status = ::CyGetListofDevices(&device_count);
    if (status == CY_SUCCESS)
    {
      for (uint8_t i = 0; i < device_count; ++i)
      {
        // TODO: Get device info directly from CyUSB API

        CY_DEVICE_INFO device_info;  // O
        status = ::CyGetDeviceInfo(i, &device_info);

        if (status == CY_SUCCESS)
        {
          for (uint8_t n = 0; n < device_info.numInterfaces; ++n)
          {
            if (device_info.deviceType[n] == CY_TYPE_I2C)
            {
              const char* p_cstr_serial_number = reinterpret_cast<const char*>(device_info.serialNum);

              if (app_settings.str_named_device.empty() || (app_settings.str_named_device == p_cstr_serial_number))
              {
                std::cout << "GPIO chip: gpiochip" << gpiochip_count << ", \""
                          << p_cstr_serial_number << "\", ? GPIO lines" << std::endl;
              }

              ++gpiochip_count;
            }
          }
        }
      }
    }
  }

  /*
  const std::string str_serial = "RTF001";
  cyusb_controller.set_working_device_by_serial(str_serial);

  std::cout << "::::::::::::::::::::::::GPIO Value for GPIO 5 ::::::::::::::::::::::::::::::::" << std::endl;
  std::cout << cyusb_controller.get_gpio_value(5) << std::endl;

  cyusb_controller.switch_gpio_state(5);
  std::cout << "::::::::::::::::::::::::GPIO Value for GPIO 5 ::::::::::::::::::::::::::::::::" << std::endl;
  std::cout << cyusb_controller.get_gpio_value(5) << std::endl;
  */

  return static_cast<int>(ExitCodeEnum::SUCCESS);
}
