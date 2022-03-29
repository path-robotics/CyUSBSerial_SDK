//#define ENABLE_DEBUGLOG
#include <cyusb-tools/cygpio-common.h>
#include <CyUSBSerial/CyController.hpp>
#include <CyUSBSerial/CyUSBSerial.hpp>
#include <thread>
#include <chrono>
#include <sstream>
#include <cstring>
#include <getopt.h>

using OffsetArray = std::vector<int>;

struct AppSettings
{
  int gpiochip_num{ -1 };
  int loop_count{ 0 };
  OffsetArray offset_array;
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

      case 'n':  // option: named gpiochip number
      {
        DEBUGLOG("[DEBUG] Option n was provided with argument " << ::optarg << std::endl);

        const std::string str_optarg{ ::optarg };
        std::istringstream istream_optarg{ str_optarg };

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
      break;

      case 'o':  // option: offset
      {
        DEBUGLOG("[DEBUG] Option o has argument " << ::optarg << std::endl);

        int offset_out;
        enum_exit_code = ::parse_arg_as_integer(opt, ::optarg, offset_out);

        if (enum_exit_code == ExitCodeEnum::SUCCESS)
        {
          r_app_settings_out.offset_array.push_back(offset_out);
        }
      }
      break;

      case 'c':  // option: loop count
        DEBUGLOG("[DEBUG] Option c has argument " << ::optarg << std::endl);
        enum_exit_code = ::parse_arg_as_integer(opt, ::optarg, r_app_settings_out.loop_count);
        break;

      case 'h':  // option: help
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

  return enum_exit_code;
}

ExitCodeEnum hammer_device(cyusb::CyController& r_cyusb_controller, const AppSettings& r_app_settings, const char* p_cstr_serial_number)
{
  if (!r_cyusb_controller.set_working_device_by_serial(p_cstr_serial_number))
  {
    std::cout << "[ERROR] Failed to select desired gpiochip" << r_app_settings.gpiochip_num
              << " (serial# " << p_cstr_serial_number << ")--Aborting!" << std::endl;
    return ExitCodeEnum::FAILED_DEVICE_SELECTION;
  }

  ExitCodeEnum enum_exit_code = ExitCodeEnum::SUCCESS;

  const int line_count = static_cast<int>(r_app_settings.offset_array.size());

  // Print header info for hammer operation
  {
    std::cout << "Hammer lines [";

    for (int i = 0; i < line_count; ++i)
    {
      const int offset = r_app_settings.offset_array[i];
      std::cout << offset;

      if (i < (line_count - 1))
      {
        std::cout << ", ";
      }
    }

    std::cout << "] on gpiochip" << r_app_settings.gpiochip_num << ", initial states: [";

    for (int i = 0; i < line_count; ++i)
    {
      const int offset = r_app_settings.offset_array[i];
      std::cout << r_cyusb_controller.get_gpio_value(offset);

      if (i < (line_count - 1))
      {
        std::cout << ", ";
      }
    }

    std::cout << "]\n";
  }

  // Hammer the GPIO lines and output progress to console
  {
    const char a_spinner[] = "-\\|/";  // swirling bar to show variation in console output lines

    for (int i = 0; (r_app_settings.loop_count == 0) || (i < r_app_settings.loop_count); ++i)
    {
      const int value = (i % 2);

      for (const auto offset : r_app_settings.offset_array)
      {
        r_cyusb_controller.set_gpio_value(offset, value);
      }

      const char spinner = a_spinner[i % std::strlen(a_spinner)];
      std::cout << "[" << spinner << "] ";

      for (int i = 0; i < line_count; ++i)
      {
        const int offset = r_app_settings.offset_array[i];
        std::cout << "[" << offset << ": " << r_cyusb_controller.get_gpio_value(offset) << "]";

        if (i < (line_count - 1))
        {
          std::cout << ", ";
        }
      }

      std::cout << "]" << std::endl;

      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  r_cyusb_controller.close_working_device();
  return enum_exit_code;
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

                return ::hammer_device(cyusb_controller, r_app_settings, p_cstr_serial_number);
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
