#pragma once
#include "margs_types.hpp"
#include "arg_value.hpp"

namespace margs {
	// SAMPLE HELP MESSAGE STRUCT (EVERYTHING THAT NEEDS TO BE DONE TO HAVE OWN CUSTOM HELP MESSAGE)
	//struct sample_help_message { 
	//	using data_type = help_data;
	//
	//	static std::string getMessage(const std::string& program_name, const arg_info_sptr<help_data>& info, 
	//		const std::vector<arg_info_sptr<help_data>>& args) {
	//		return "Default Help Message";
	//	}
	//};

	template<class _help_data>
	struct arg_info {
		std::string name = "";
		std::optional<_short_name_t> short_name = std::optional<_short_name_t>();
		bool is_default = false;
		size_t values_num = 0;
		std::optional<arg_value> default_values = std::optional<arg_value>();
		_help_data info = {};
	};

	template<>
	struct arg_info<void> {
		std::string name = "";
		std::optional<_short_name_t> short_name = std::optional<_short_name_t>();
		bool is_default = false;
		size_t values_num = 0;
		std::optional<arg_value> default_values = std::optional<arg_value>();
	};

	struct help_data {
		std::string description;
		std::optional<std::string> long_description = std::optional<std::string>();
	};

	struct help_message {
	private:
		using _info_sptr = arg_info_sptr<help_data>;

		static constexpr int _options_args_padding = 3;

	public:
		using data_type = help_data;

		// usage
		static std::string printUsage(const std::string& name, const _info_sptr& info, 
			const std::vector<_info_sptr>& args) {
			std::string usage_str = "Usage: " + name;
			if (args.size() != 0) {
				usage_str += " [Options]...";
			}
			if (info->values_num != 0) {
				if (info->values_num == 1) {
					usage_str += " <value>";
				}
				else {
					for (size_t i = 0; i != info->values_num; ++i) {
						usage_str += " <value" + std::to_string(i) + ">";
					}
				}
			}
			return usage_str;
		}

		// (default: value0=, value1=,)
		static std::string printDefault(const _info_sptr& arg) {
			std::string default_str = "";
			if (arg->default_values.has_value()) {
				const arg_value& deafults = arg->default_values.value();

				default_str += "(default: ";
				if (deafults.isScalar()) {
					default_str += "value=\"" + deafults.as<std::string>() + "\"";
				}
				else if (deafults.isSequence()) {
					size_t i = 0;
					for (const arg_value& value : arg->default_values.value()) {
						if (i != 0) default_str += ", ";
						default_str += "value" + std::to_string(i) + "=\"" + value.as<std::string>() + "\"";
						++i;
					}
				}
				default_str += ")";
			}
			else if (arg->is_default) {
				default_str += "(default)";
			}
			return default_str;
		}

		// description with default at the end
		static std::string printArgDescription(const _info_sptr& arg) {
			return arg->info.description + " " + printDefault(arg);
		}

		// standard description
		static std::string printDescription(const _info_sptr& info) {
			if (info->info.long_description.has_value()) {
				return info->info.long_description.value() + " " + printDefault(info);
			}
			else {
				return info->info.description + " " + printDefault(info);
			}
		}

		// -a, --arg <value0> <value1> ...
		static std::string printArgUsage(const _info_sptr& arg) {
			std::string arg_str;

			// add short name
			if (arg->short_name.has_value()) {
				arg_str += "-";
				arg_str += arg->short_name.value();
				arg_str += ", ";
			}
			else {
				arg_str += std::string(4, ' ');
			}

			// add name
			arg_str += arg->name;

			// add values
			if (arg->values_num > 0) {
				if (arg->values_num == 1) {
					arg_str += " <value>";
				}
				else {
					for (size_t i = 0; i != arg->values_num; ++i) {
						arg_str += " <value" + std::to_string(i) + ">";
					}
				}
			}
			return arg_str;
		}

		// join arg with description
		static std::string printArg(const _info_sptr& arg_info) {
			const std::string& arg = printArgUsage(arg_info);
			const std::string& description = printArgDescription(arg_info);

			int width, height;
			mstd::get_terminal_size(width, height);

			width -= _options_args_padding * 2; // sub padding
			int section_width = (49 * width) / 100; // 49/100 ... 49/100
			int gap_width = std::max(width - 2 * section_width, 0);
			size_t arg_i = 0;
			size_t desc_i = 0;

			std::string message;
			size_t break_i;
			while (arg_i != arg.size() || desc_i != description.size()) {
				// add new line
				if (arg_i != 0 || desc_i != 0) {
					message += '\n';
				}

				// add left padding
				message += std::string(_options_args_padding, ' ');

				// add arg part
				if (arg_i != arg.size()) {
					break_i = arg_i;
					for (size_t i = 0;; ++i) {
						if (arg_i + i == arg.size()) {
							break_i = arg.size();
							break;
						}

						if (i == section_width && break_i != arg_i) {
							break;
						}

						if (std::isspace(arg[arg_i + i])) {
							break_i = arg_i + i + 1;
						}
					}
					if (arg_i == break_i) break_i = arg.size();

					message += std::string(arg.begin() + arg_i, arg.begin() + break_i) + 
						std::string(section_width - (break_i - arg_i), ' ');

					arg_i = break_i;
				}
				else {
					message += std::string(section_width, ' ');
				}

				// add gap
				message += std::string(gap_width, ' ');

				// add description part
				if (desc_i != description.size()) {
					break_i = desc_i;
					for (size_t i = 0;; ++i) {
						if (desc_i + i == description.size()) {
							break_i = description.size();
							break;
						}

						if (i == section_width && break_i != desc_i) {
							break;
						}

						if (std::isspace(description[desc_i + i])) {
							break_i = desc_i + i + 1;
						}
					}
					if (break_i == desc_i) break_i = description.size();

					message += std::string(description.begin() + desc_i, description.begin() + break_i) + 
						std::string(section_width - (break_i - desc_i), ' ');

					desc_i = break_i;
				}
				else {
					message += std::string(section_width, ' ');
				}
			}
			return message;
		}

		// options
		static std::string printOptions(const std::vector<_info_sptr>& args) {
			std::string value;
			if (args.size() > 0) {
				value += "Options:\n";
				size_t i = 0;
				for (const _info_sptr& arg : args) {
					if (i != 0) value += "\n";
					value += printArg(arg);
					++i;
				}
			}
			return value;
		}

		static std::string getMessage(const std::string& name, const _info_sptr& info, 
			const std::vector<_info_sptr>& args) {
			return printUsage(name, info, args) + "\n" + printDescription(info) +
				"\n\n" + printOptions(args);
		}
	};
}