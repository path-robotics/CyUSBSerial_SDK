//#define ENABLE_DEBUGLOG
#include <cyusb-tools/cygpio-common.h>
#include <CyUSBSerial/CyController.hpp>
#include <CyUSBSerial/CyUSBSerial.hpp>
#include <vector>
#include <sstream>
#include <cstring>
#include <getopt.h>

using OffsetArray = std::vector<int>;

struct AppSettings
{
  int gpiochip_num{ -1 };
  OffsetArray offset_array;
};

static void print_usage()
{
  std::cout << "Usage: cygpioget [OPTIONS] <gpiochip-num> <offset1> [<offset2>]...\n"
               "\n"
               "Read line value(s) from a GPIO chip\n"
               "\n"
               "Options:\n"
               "  -h, --help        Display this message and exit\n"
               "\n"
               "Example:\n"
               "cygpioget gpiochip0 2 3 4 5\n"
            << std::endl;
}

static ExitCodeEnum parse_cmdline_args(int argc, char* argv[], AppSettings& r_app_settings_out)
{
  ExitCodeEnum enum_exit_code = ExitCodeEnum::SUCCESS;

  int regular_arg_index{};

  static constexpr char a_short_options[] = "-:h";

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
          int offset_out;  // O

          std::string str_offset_out;
          if (std::getline(istream_optarg, str_offset_out, '='))
          {
            enum_exit_code = ::parse_arg_as_integer("offset", str_offset_out.c_str(), offset_out);
          }

          if (enum_exit_code == ExitCodeEnum::SUCCESS)
          {
            r_app_settings_out.offset_array.push_back(offset_out);
          }
        }

        ++regular_arg_index;
      }
      break;

      case 'h':
        DEBUGLOG("[DEBUG] Option h was provided" << std::endl);
        enum_exit_code = ExitCodeEnum::PRINT_USAGE_AND_EXIT;
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

ExitCodeEnum print_gpio_values(cyusb::CyController& r_cyusb_controller, const AppSettings& r_app_settings, const char* p_cstr_serial_number)
{
  if (!r_cyusb_controller.set_working_device_by_serial(p_cstr_serial_number))
  {
    std::cout << "[ERROR] Failed to select desired gpiochip" << r_app_settings.gpiochip_num
              << " (serial# " << p_cstr_serial_number << ")--Aborting!" << std::endl;
    return ExitCodeEnum::FAILED_DEVICE_SELECTION;
  }

  // Print list of gpio line values separated by spaces
  {
    for (const auto offset : r_app_settings.offset_array)
    {
      std::cout << r_cyusb_controller.get_gpio_value(offset) << " ";
    }

    std::cout << std::endl;
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
                return ::print_gpio_values(cyusb_controller, r_app_settings, p_cstr_serial_number);
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
