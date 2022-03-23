#include <CyUSBSerial/CyController.hpp>
#include <CyUSBSerial/CyUSBSerial.hpp>
#include <vector>
#include <utility>
#include <iostream>
#include <sstream>
#include <string>
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
  PRINT_USAGE_AND_EXIT,
  MISSING_REQUIRED_ARG,
  GPIOCHIP_NOT_FOUND,
  FAILED_DEVICE_SELECTION,
  RUNTIME_EXCEPTION
};

using OffsetValuePair = std::pair<int, int>;
using OffsetValuePairArray = std::vector<OffsetValuePair>;

struct AppSettings
{
  int gpiochip_num{ -1 };
  OffsetValuePairArray offset_value_pair_array;
};

static void print_usage()
{
  std::cout << "Usage: cygpioset [OPTIONS] <gpiochip-num> <offset1>=[0|1] [<offset2>=[0|1]]...\n"
               "\n"
               "Set GPIO line values of a GPIO chip and maintain the state until the process exits\n"
               "\n"
               "Options:\n"
               "  -h, --help        Display this message and exit\n"
               //"  -m, --mode=[exit|wait|time|signal] (defaults to 'exit'):\n"
               //"  -m, --mode=[exit|wait|time] (defaults to 'exit'):\n"
               //"                    Tell the program what to do after settings values\n"
               //"  -s, --sec=SEC     Specify the number of seconds to wait (only valid for --mode=time)\n"
               //"  -u, --usec=USEC   Specify the number of microseconds to wait (only valid for --mode=time)\n"
               //"\n"
               //"Modes:\n"
               //"  exit   Set values and exit immediately\n"
               //"  wait   Set values and wait for user to press ENTER\n"
               //"  time   Set values and sleep for a specified amount of time\n"
               //"  signal:  Set values and wait for SIGINT or SIGTERM\n"
               "\n"
               "Example:\n"
               //"cygpioset -m=time -s=1 gpiochip0 2=0 3=1 4=1 5=0\n"
               "cygpioset gpiochip0 2=0 3=1 4=1 5=0\n"
            << std::endl;
}

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

static ExitCodeEnum parse_cmdline_args(int argc, char* argv[], AppSettings& r_app_settings_out)
{
  ExitCodeEnum enum_exit_code = ExitCodeEnum::SUCCESS;

  int regular_arg_index{};

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
      {
        const std::string str_optarg{ ::optarg };
        std::istringstream istream_optarg{ str_optarg };

        if (regular_arg_index == 0)  // gpiochip-num
        {
          if (str_optarg.compare(0, std::strlen("gpiochip"), "gpiochip") == 0)
          {
            enum_exit_code = ::parse_arg_as_integer("gpiochip-num", ::optarg + std::strlen("gpiochip"), r_app_settings_out.gpiochip_num);
          }
          else
          {
            std::cout << "[ERROR] Argument " << ::optarg << " unrecognized--Aborting!" << std::endl;
            enum_exit_code = ExitCodeEnum::UNKNOWN_CMDLINE_ARG;
          }
        }
        else  // offsetX=[0|1]
        {
          ::OffsetValuePair offset_value_pair;

          std::string str_offset_out;
          if (std::getline(istream_optarg, str_offset_out, '='))
          {
            enum_exit_code = ::parse_arg_as_integer("offset", str_offset_out.c_str(), offset_value_pair.first);
          }

          if (enum_exit_code == ExitCodeEnum::SUCCESS)
          {
            std::string str_value_out;
            if (std::getline(istream_optarg, str_value_out))
            {
              enum_exit_code = ::parse_arg_as_integer("value", str_value_out.c_str(), offset_value_pair.second);
            }
          }

          if (enum_exit_code == ExitCodeEnum::SUCCESS)
          {
            r_app_settings_out.offset_value_pair_array.push_back(offset_value_pair);
          }
        }

        ++regular_arg_index;
      }
      break;

      case 'h':
        DEBUGLOG("[DEBUG] Option h was provided" << std::endl);
        enum_exit_code = ExitCodeEnum::PRINT_USAGE_AND_EXIT;
        break;

        /*
        case 'n':
          DEBUGLOG("[DEBUG] Option n was provided with argument " << ::optarg << std::endl);
          r_app_settings_out.str_named_device = ::optarg;
          break;
        */

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

  // Verify required app settings are present
  if (r_app_settings_out.gpiochip_num < 0)
  {
    std::cout << "[ERROR] Missing required argument <gpiochip-num>--Aborting!" << std::endl;
    return ExitCodeEnum::MISSING_REQUIRED_ARG;
  }

  return enum_exit_code;
}

ExitCodeEnum set_gpio_values(cyusb::CyController& r_cyusb_controller, const AppSettings& r_app_settings, const char* p_cstr_serial_number)
{
  if (!r_cyusb_controller.set_working_device_by_serial(p_cstr_serial_number))
  {
    std::cout << "[ERROR] Failed to select desired gpiochip" << r_app_settings.gpiochip_num
              << " (serial# " << p_cstr_serial_number << ")--Aborting!" << std::endl;
    return ExitCodeEnum::FAILED_DEVICE_SELECTION;
  }

  for (const auto offset_value_pair : r_app_settings.offset_value_pair_array)
  {
    const auto offset = static_cast<size_t>(offset_value_pair.first);
    const auto value = static_cast<size_t>(offset_value_pair.second);

    try
    {
      r_cyusb_controller.set_gpio_value(offset, value);
      DEBUGLOG("[INFO ] Set gpiochip" << r_app_settings.gpiochip_num
                                      << " line " << offset << " to value " << value << std::endl);
    }
    catch (const std::exception& r_exc)
    {
      std::cout << "[ERROR] Unable to set GPIO line " << offset << " to " << value << "--Exception thrown! " << r_exc.what();
    }
  }

  r_cyusb_controller.close_working_device();

  return ExitCodeEnum::SUCCESS;
}

ExitCodeEnum run_application(const AppSettings& r_app_settings)
{
  cyusb::CyController cyusb_controller;

  if (!cyusb_controller.initialize())
  {
    if (cyusb_controller.get_number_of_devices() > 0)
    {
      std::cout << "[ WARN] No connected CyUSB devices found" << std::endl;
    }

    std::cout << "[ERROR] Failed to initialize CyUSB controller--Aborting!" << std::endl;
    return ExitCodeEnum::FAILED_TO_INITIALIZE;
  }

  // Find selected gpiochip-num and execute desired operation
  {
    int gpiochip_count{};

    uint8_t device_count;  // O
    CY_RETURN_STATUS status = ::CyGetListofDevices(&device_count);
    if (status == CY_SUCCESS)
    {
      for (uint8_t i = 0; i < device_count; ++i)
      {
        CY_DEVICE_INFO device_info;  // O
        status = ::CyGetDeviceInfo(i, &device_info);

        if (status == CY_SUCCESS)
        {
          for (uint8_t n = 0; n < device_info.numInterfaces; ++n)
          {
            if (device_info.deviceType[n] == CY_TYPE_I2C)
            {
              if (r_app_settings.gpiochip_num == gpiochip_count)
              {
                const char* p_cstr_serial_number = reinterpret_cast<const char*>(device_info.serialNum);

                DEBUGLOG("GPIO chip: gpiochip" << gpiochip_count << ", \"" << p_cstr_serial_number << "\"" << std::endl);
                return ::set_gpio_values(cyusb_controller, r_app_settings, p_cstr_serial_number);
              }

              ++gpiochip_count;
            }
          }
        }
      }
    }
  }

  std::cout << "[ERROR] Failed to locate desired gpiochip--Aborting!" << std::endl;
  return ExitCodeEnum::GPIOCHIP_NOT_FOUND;
}

int main(int argc, char* argv[])
{
  AppSettings app_settings;  // O

  // Parse command-line arguments
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

  try
  {
    return static_cast<int>(::run_application(app_settings));
  }
  catch (const std::exception& r_exc)
  {
    std::cout << "[ERROR] Exception thrown! " << r_exc.what();
  }

  return static_cast<int>(ExitCodeEnum::RUNTIME_EXCEPTION);
}
