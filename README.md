# margs - Maipa's Args Analyzer

## About

**margs** (Maipa's Args Analyzer) is a C++ library for parsing command-line arguments. It supports flags (`--flag`), values (`--value value`), and grouped arguments (e.g., `group --group-flag`). The library allows easy definition of valid arguments, automatically generates error messages for invalid input, and provides built-in help message generation.

## Features

- **Argument Parsing**: Supports flags, values, and grouped arguments.
- **Automatic Help Message Generation**: No extra setup required.
- **Custom Argument Decoders**: Define custom types for argument parsing.
- **Custom Help Messages**: Override default help messages.
- **Header-Only Library**: Easy to integrate.
- **Platform Compatibility**: Designed for **Windows, macOS, and Linux**.
- **C++20 Compatibility**: Uses modern C++ features.

## Requirements

- **C++ Standard**: The library is designed for **C++20** and later.
- **Dependencies**: This library depends on [**mstd**](https://github.com/MAIPA01/mstd) (Maipa’s Standard Library).

## Installation

No additional dependencies are required apart from [**mstd**](https://github.com/MAIPA01/mstd). Simply include the **margs** headers in your project.

## Usage Examples

### Basic Argument Parsing

```cpp
#include <margs/margs.hpp>
#include <iostream>

using namespace std;
using namespace margs;

int main(int argc, char** argv) {
    // Create program args
    args_analizer args = args_analizer(
        help_data{ .description = "Args analizer sample description" },
        args_builder::makeValue("value", 'v', 1,  
            help_data{.description = "sample value description"}
        ), 
        args_builder::makeFlag("flag", 'f',  
            help_data{ .description = "sample flag description" }
        ),
        args_builder::makeGroup("group",
            help_data { .description = "sample group description" },
            args_builder::makeFlag("group-flag", 'f'
                help_data { .description = "sample group flag description" }
            )
        )
    );

    // Get program args
    args_map arg_values;
    try {
        arg_values = args.analize(argc, argv);
    }
    catch (const args_exception& e) {
        cout << e.what() << endl;
        return 1;
    }

    // Get Value Arg 
    string value;
    if (arg_values.contains_value("value")) { 
        value = arg_values.get_value("value").as<string>(); 
    }

    if (arg_values.contains_flag("group::group-flag")) {
        cout << "Group flag" << endl;
    }
}
```

### Custom Argument Decoders

You can define custom decoders for your own data types by specializing `decoder<T>` inside the `margs` namespace:

```cpp
namespace margs {
    template<>
    struct decoder<custom_type> {
        static bool decode(const arg_value& lhs, custom_type& rhs) { 
            if (!lhs.isScalar()) return false;
            rhs = custom_type(lhs.as<string>()); 
            return true;
        }
    };
}
```

### Custom Help Message

You can define custom help message structures that will be used when `--help` is passed:

```cpp
struct sample_help_message {  
    using data_type = help_data;
    static std::string getMessage(const std::string& program_name, 
    const arg_info_sptr<help_data>& info,  
    const std::vector<arg_info_sptr<help_data>>& args) { 
        return "Default Help Message"; 
    } 
};
```

By default, `help_data` is used to provide additional argument information, but you can override it for more customization.

## License

This project is licensed under the **BSD 3-Clause License with Attribution Requirement**. See the [`LICENSE`](./LICENSE) file for more details.

