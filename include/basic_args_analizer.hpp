#pragma once
#include "margs_types.hpp"
#include "basic_arg.hpp"
#include "arg_value.hpp"
#include "args_exception.hpp"
#include "help_message.hpp"

namespace margs {	
	template<class _help_message>
	struct _get_help_data {
		using type = _help_message::data_type;
	};

	template<>
	struct _get_help_data<void> {
		using type = void;
	};

	template<class _help_message>
	using _get_help_data_t = _get_help_data<_help_message>::type;

	template<bool test, class _help_message>
	struct _analizer_info_args {};

	template<class _help_message>
	struct _analizer_info_args<true, _help_message> {
		using _help_data_t = _get_help_data_t<_help_message>;
		using _arg_info_sptr_t = arg_info_sptr<_help_data_t>;
		using _args_infos_list_t = std::vector<_arg_info_sptr_t>;
		using _args_infos_list_iterator_t = _args_infos_list_t::iterator;

		std::string _program_name;
		_arg_info_sptr_t _program_info;
		std::unordered_map<std::string, _args_infos_list_t> _args_info_per_group;
		std::unordered_map<std::string, _args_infos_list_iterator_t> _group_args_info;
	};

	template<class _help_message>
	class basic_args_analizer : private _analizer_info_args<!std::is_void_v<_help_message>, _help_message> {
	private:
		using _self_t = basic_args_analizer<_help_message>;
		using _self_ptr_t = basic_args_analizer_sptr<_help_message>;
		using _help_data_t = _get_help_data_t<_help_message>;
		using _arg_info_t = arg_info<_help_data_t>;
		using _arg_info_sptr_t = arg_info_sptr<_help_data_t>;
		using _analizer_info_t = _analizer_info_args<!std::is_void_v<_help_message>, _help_message>;

		static constexpr bool _is_help_data_v = !std::is_void_v<_help_data_t>;
		static constexpr bool _is_help_message_v = !std::is_void_v<_help_message>;
		template<class T>
		using _enable_if_help_message_t = std::enable_if_t<(_is_help_message_v && 
			std::is_same_v<T, _help_message>), bool>;
		template<class R>
		using _enable_help_message_parameter_t = std::enable_if_t<_is_help_message_v, R>;

		using _arg_sptr_t = _basic_arg_sptr<_help_data_t>;

		struct _analize_data {
			std::string name_space;
			std::unordered_map<std::string, arg_value> values;
		};

		struct _arg_data {
			size_t values_num = 0;
			std::optional<arg_callback_t> callback = std::optional<arg_callback_t>();

			constexpr bool has_callback() const {
				return callback.has_value();
			}

			constexpr bool has_values() const {
				return values_num > 0;
			}
		};

		std::unordered_map<std::string, _arg_data> _args_data;
		std::unordered_map<std::string, std::string> _short_name_to_arg;
		std::unordered_set<std::string> _default_flags;
		std::unordered_map<std::string, arg_value> _default_values;

		std::string _make_arg_name(const std::string& arg_name) {
			std::string good_arg_name = "";
			for (const unsigned char& c : arg_name) {
				if (std::isspace(c)) {
					good_arg_name += '-';
				}
				else {
					good_arg_name += c;
				}
			}
			return good_arg_name;
		}

		template<class T = _help_message, _enable_if_help_message_t<T> = true>
		void _get_program_name(char* path) {
			uint32_t c = 0;
			std::string full_path = _get_string(path, c, '\0');

			size_t begin_i = 0;
			size_t end_i = 0;
			for (size_t i = 0; i != full_path.size(); ++i) {
				if (full_path[i] == '\\' || full_path[i] == '/') {
					begin_i = i + 1;
					end_i = begin_i;
				}
				else if (full_path[i] == '.') {
					end_i = i;
				}
			}
			if (end_i == begin_i) end_i = full_path.size();

			_analizer_info_t::_program_name = std::string(full_path.begin() + begin_i, full_path.begin() + end_i);
		}

		// -h | --help
		template<class T = _help_message, _enable_if_help_message_t<T> = true>
		void _make_help(const std::string& name_space = "") {
			const std::string& arg_name = (name_space == "" ? "--" : (name_space + "::--")) + "help";

			const std::string& short_name = (name_space == "" ? "-" : (name_space + "::-")) + "h";
			_short_name_to_arg.insert({ short_name, arg_name });

			_arg_data data = { .values_num = 0 };
			if (name_space == "") {
				data.callback = [&, name_space](const arg_value&) -> void {
					// print help
					std::cout << 
						_help_message::getMessage(_analizer_info_t::_program_name,
							_analizer_info_t::_program_info,
							_analizer_info_t::_args_info_per_group[name_space])
						<< std::endl;
				};
			}
			else {
				std::string local_program_name = _analizer_info_t::_program_name;
				uint32_t c = 0;
				while (c != name_space.size()) {
					local_program_name += " " + _get_string(name_space.c_str(), c, '\0', ':');
					if (name_space[c] != '\0') c += 2;
				}

				data.callback = [&, name_space, local_program_name](const arg_value&) -> void {
					// print help
					std::cout << 
						_help_message::getMessage(local_program_name, 
							*_analizer_info_t::_group_args_info[name_space],
							_analizer_info_t::_args_info_per_group[name_space]) 
						<< std::endl;
				};
			}
			_args_data.insert({ arg_name, data });
		}

		template<class... Args>
		void _add_args(const _arg_sptr_t& arg, const Args&... args) {
			_add_arg(arg);

			if constexpr (sizeof...(args) > 0) {
				_add_args(args...);
			}
		}

		void _add_arg(const _arg_sptr_t& arg, const std::string& name_space = "") {
			_arg_info_sptr_t info;
			std::string arg_name;

			if constexpr (_is_help_message_v) {
				info = std::make_shared<_arg_info_t>();

				// Get Local Name
				info->name = (arg->getType() == arg_type::group ? "" : "--") +
					_make_arg_name(arg->getName());

				// Get Name
				arg_name = (name_space == "" ? "" : (name_space + "::")) + info->name;
			}
			else {
				// Get Name
				arg_name = (name_space == "" ? "" : (name_space + "::")) + 
					(arg->getType() == arg_type::group ? "" : "--") + _make_arg_name(arg->getName());
			}

			// Set Short Name
			if (arg->hasShortName()) {
				std::string short_name;

				if constexpr (_is_help_message_v) {
					info->short_name = arg->getShortName();
					short_name = (name_space == "" ? "-" : (name_space + "::-")) + 
						std::string(1, info->short_name.value());
				}
				else {
					short_name = (name_space == "" ? "-" : (name_space + "::-")) + 
						std::string(1, arg->getShortName());
				}
				_short_name_to_arg.insert({ short_name, arg_name });
			}

			// Add Arg Data
			_arg_data data = {};
			if constexpr (_is_help_message_v) {
				info->values_num = arg->getValuesNum();
				data.values_num = info->values_num;
				info->is_default = arg->isDefault();
				if (info->is_default) {
					_default_flags.insert(arg_name);
				}
			}
			else {
				data.values_num = arg->getValuesNum();
				if (arg->isDefault()) {
					_default_flags.insert(arg_name);
				}
			}
			if (arg->hasCallback()) {
				data.callback = arg->getCallback();
			}
			_args_data.insert({ arg_name, data });

			// Add Default Values
			if (arg->hasDefaultValues()) {
				if constexpr (_is_help_message_v) {
					info->default_values = arg->getDefaultValues();
					_default_values.insert({ arg_name, info->default_values.value() });
				}
				else {
					_default_values.insert({ arg_name, arg->getDefaultValues() });
				}
			}

			if constexpr (_is_help_message_v) {
				// Add Arg Info Help Data
				if constexpr (_is_help_data_v) {
					info->info = arg->getHelpInfo();
				}

				// Add Arg Info
				if (!_analizer_info_t::_args_info_per_group.contains(name_space)) {
					_analizer_info_t::_args_info_per_group[name_space] = {};
				}
				_analizer_info_t::_args_info_per_group[name_space].push_back(info);
			}

			// Add Group Args
			if (arg->hasArgs()) {
				if constexpr (_is_help_message_v) {
					// add group info
					_analizer_info_t::_group_args_info[arg_name] = 
						_analizer_info_t::_args_info_per_group[name_space].end() - 1;

					// add help callback
					_make_help(arg_name);
				}

				for (const _arg_sptr_t& sub_arg : arg->getArgs()) {
					_add_arg(sub_arg, arg_name);
				}
			}
		}

		// --long-name | -n | group
		void _read_arg(uint32_t& v, const int& argc, char** argv, _analize_data& data) const {
			if (argv[v][0] == '-') {
				if (argv[v][1] == '-') {
					return _read_long_arg(v, argc, argv, data);
				}
				else {
					return _read_short_arg(v, argc, argv, data);
				}
			}
			else if (v == 0) {
				return _read_group(v, argc, argv, data);
			}
			throw args_exception(std::string("Not recognized argument '") + argv[v] + "'");
		}

		// --long-name | --long-name <value> | --long-name=<value>
		void _read_long_arg(uint32_t& v, const int& argc, char** argv, _analize_data& data) const {
			uint32_t c = 0; // begins with --

			// Get Long Name
			const std::string& arg_name = (data.name_space == "" ? "" : (data.name_space + "::")) 
											+ _get_string(argv[v], c, '\0', '=');

			// Get Arg Data
			const _arg_data& arg_info = _get_arg_data(arg_name);

			// Get Value
			arg_value value = arg_value();
			if (arg_info.has_values()) {
				if (arg_info.values_num == 1) {
					if (argv[v][c] == '=') {
						++c;
						value = _read_value(argv[v], c);
					}
					else {
						_increment_v(v, argc);
						value = _read_value(argv[v]);
					}
				}
				else {
					for (size_t i = 0; i != arg_info.values_num; ++i) {
						_increment_v(v, argc);
						value.push_back(_read_value(argv[v]));
					}
				}
			}

			if (arg_info.has_callback()) {
				// Execute Action
				arg_info.callback.value()(value);
			}

			data.values[arg_name] = value;
		}

		// -n | -n <value> | -na <value_n> <value_a>
		void _read_short_arg(uint32_t& v, const int& argc, char** argv, _analize_data& data) const {
			uint32_t c = 1; // begins with -
			const uint32_t arg_v = v;

			while (argv[arg_v][c] != '\0') {
				// Get Short Name
				const std::string& short_name = (data.name_space == "" ? "-" : (data.name_space + "::-")) + 
					argv[arg_v][c];
				if (!_short_name_to_arg.contains(short_name)) {
					throw args_exception(std::string("Not recognized argument '") + argv[arg_v][c] + "'");
				}
				++c;
				const std::string& arg_name = _short_name_to_arg.at(short_name);

				// Get Arg Data
				const _arg_data& arg_info = _get_arg_data(arg_name);

				// Get Value
				arg_value value = arg_value();
				if (arg_info.has_values()) {
					if (arg_info.values_num == 1) {
						_increment_v(v, argc);
						value = _read_value(argv[v]);
					}
					else {
						for (size_t i = 0; i != arg_info.values_num; ++i) {
							_increment_v(v, argc);
							value.push_back(_read_value(argv[v]));
						}
					}
				}

				if (arg_info.has_callback()) {
					// Execute Action
					arg_info.callback.value()(value);
				}

				data.values[arg_name] = value;
			}
		}

		// group
		void _read_group(uint32_t& v, const int& argc, char** argv, _analize_data& data) const {
			uint32_t c = 0;

			// Get Group Name
			data.name_space = (data.name_space == "" ? "" : (data.name_space + "::")) + 
				_get_string(argv[v], c, '\0');

			// Get Arg Data
			const _arg_data& arg_info = _get_arg_data(data.name_space);

			// Read Group Args
			uint32_t gargc = argc - v - 1;
			if (arg_info.has_values()) {
				if (mstd::sub_overflow(gargc, arg_info.values_num, gargc)) {
					throw args_exception("Expected values at the end of group");
				}
			}
			char** gargv = argv + v + 1;
			for (uint32_t gv = 0; gv < gargc; ++gv, ++v) {
				_read_arg(gv, gargc, gargv, data);
			}

			// Get Value
			arg_value value = arg_value();
			if (arg_info.has_values()) {
				if (arg_info.values_num == 1) {
					_increment_v(v, argc);
					value = _read_value(argv[v]);
				}
				else {
					for (size_t i = 0; i != arg_info.values_num; ++i) {
						_increment_v(v, argc);
						value.push_back(_read_value(argv[v]));
					}
				}
			}

			if (arg_info.has_callback()) {
				// Get Action
				arg_info.callback.value()(value);
			}

			data.values[data.name_space] = value;
		}

		// value
		std::string _read_value(char* value_arg, uint32_t c = 0) const {
			return _get_string(value_arg, c, '\0');
		}

		template<class T, class... Ts>
		static constexpr bool _is_any_v = (std::is_same_v<T, Ts> || ...);

		template<class... _Chars, 
			std::enable_if_t<(_is_any_v<_Chars, char, unsigned char, signed char> && ...), bool> = true>
		bool _is_string_end(const char& curr_char, const char& c, const _Chars&... chars) const {
			if (curr_char == c) return true;

			if constexpr (sizeof...(chars) > 0) {
				return _is_string_end(curr_char, chars...);
			}
			else {
				return false;
			}
		}

		template<class... _Chars, 
			std::enable_if_t<(_is_any_v<_Chars, char, unsigned char, signed char> && ...), bool> = true>
		std::string _get_string(const char* table, uint32_t& c, const _Chars&... chars) const {
			std::string value = "";
			while (!_is_string_end(table[c], chars...)) {
				value += table[c];
				++c;
			}
			return value;
		}

		const _arg_data& _get_arg_data(const std::string& arg_name) const {
			if (!_args_data.contains(arg_name)) {
				throw args_exception(std::string("Not recognized argument '") + arg_name + "'");
			}
			return _args_data.at(arg_name);
		}

		void _increment_v(uint32_t& v, const uint32_t& argc) const {
			++v;
			if (v >= argc) {
				throw args_exception("Expected some value");
			}
		}

	public:
		template<class... Args, class H = _help_data_t, _enable_if_all_args_t<_help_data_t, Args...> = true,
			_enable_if_basic_t<H, _help_data_t> = true>
		basic_args_analizer(char** argv, const std::shared_ptr<Args>&... args) {
			// Add program data
			_args_data.insert({ "", _arg_data{} });

			if constexpr (_is_help_message_v) {
				_analizer_info_t::_program_info = std::make_shared<_arg_info_t>();

				_get_program_name(argv[0]);
				_make_help();
			}
			_add_args(args...);
		}
		template<class... Args, class H = _help_data_t, _enable_if_all_args_t<_help_data_t, Args...> = true,
			_enable_if_basic_t<H, _help_data_t> = true>
		basic_args_analizer(char** argv, const size_t& values_num, const std::shared_ptr<Args>&... args) {
			// Add program data
			_args_data.insert({ "", _arg_data{ .values_num = values_num } });

			if constexpr (_is_help_message_v) {
				_analizer_info_t::_program_info = std::make_shared<_arg_info_t>();
				_analizer_info_t::_program_info->values_num = values_num;

				_get_program_name(argv[0]);
				_make_help();
			}
			_add_args(args...);
		}
		template<class... Args, class H = _help_data_t, _enable_if_all_args_t<_help_data_t, Args...> = true,
			_enable_if_basic_t<H, _help_data_t> = true>
		basic_args_analizer(char** argv, const _values_list_t& values, const std::shared_ptr<Args>&... args) {
			// Add program data
			_args_data.insert({ "", _arg_data{ .values_num = values.size() }});
			_default_values[""] = arg_value(values);

			if constexpr (_is_help_message_v) {
				_analizer_info_t::_program_info = std::make_shared<_arg_info_t>();
				_analizer_info_t::_program_info->default_values = _default_values[""];
				_analizer_info_t::_program_info->values_num = values.size();

				_get_program_name(argv[0]);
				_make_help();
			}
			_add_args(args...);
		}

		template<class... Args, class H = _help_data_t, _enable_if_all_args_t<_help_data_t, Args...> = true, 
			_enable_if_help_data_t<H, _help_data_t> = true>
		basic_args_analizer(char** argv, const H& info, const std::shared_ptr<Args>&... args) {
			// Add program data
			_args_data.insert({ "", _arg_data{} });

			if constexpr (_is_help_message_v) {
				_analizer_info_t::_program_info = std::make_shared<_arg_info_t>();
				_analizer_info_t::_program_info->info = info;

				_get_program_name(argv[0]);
				_make_help();
			}
			_add_args(args...);
		}
		template<class... Args, class H = _help_data_t, _enable_if_all_args_t<_help_data_t, Args...> = true,
			_enable_if_help_data_t<H, _help_data_t> = true>
		basic_args_analizer(char** argv, const size_t& values_num, const H& info, 
			const std::shared_ptr<Args>&... args) {
			// Add program data
			_args_data.insert({ "", _arg_data{ .values_num = values_num } });

			if constexpr (_is_help_message_v) {
				_analizer_info_t::_program_info = std::make_shared<_arg_info_t>();
				_analizer_info_t::_program_info->values_num = values_num;
				_analizer_info_t::_program_info->info = info;

				_get_program_name(argv[0]);
				_make_help();
			}
			_add_args(args...);
		}
		template<class... Args, class H = _help_data_t, _enable_if_all_args_t<_help_data_t, Args...> = true,
			_enable_if_help_data_t<H, _help_data_t> = true>
		basic_args_analizer(char** argv, const _values_list_t& values, const H& info, 
			const std::shared_ptr<Args>&... args) {
			// Add program data
			_args_data.insert({ "", _arg_data{ .values_num = values.size() }});
			_default_values[""] = arg_value(values);

			if constexpr (_is_help_message_v) {
				_analizer_info_t::_program_info = std::make_shared<_arg_info_t>();
				_analizer_info_t::_program_info->values_num = values.size();
				_analizer_info_t::_program_info->default_values = _default_values[""];
				_analizer_info_t::_program_info->info = info;

				_get_program_name(argv[0]);
				_make_help();
			}
			_add_args(args...);
		}

		template<class... Args, class H = _help_data_t, _enable_if_all_args_t<_help_data_t, Args...> = true,
			_enable_if_basic_t<H, _help_data_t> = true>
		static _self_ptr_t create(char** argv, const std::shared_ptr<Args>&... args) {
			return std::make_shared<_self_t>(argv, args...);
		}
		template<class... Args, class H = _help_data_t, _enable_if_all_args_t<_help_data_t, Args...> = true,
			_enable_if_basic_t<H, _help_data_t> = true>
		static _self_ptr_t create(char** argv, const size_t& values_num,
			const std::shared_ptr<Args>&... args) {
			return std::make_shared<_self_t>(argv, values_num, args...);
		}
		template<class... Args, class H = _help_data_t, _enable_if_all_args_t<_help_data_t, Args...> = true,
			_enable_if_basic_t<H, _help_data_t> = true>
		static _self_ptr_t create(char** argv, const _values_list_t& values,
			const std::shared_ptr<Args>&... args) {
			return std::make_shared<_self_t>(argv, values, args...);
		}

		template<class... Args, class H = _help_data_t, _enable_if_all_args_t<_help_data_t, Args...> = true,
			_enable_if_help_data_t<H, _help_data_t> = true>
		static _self_ptr_t create(char** argv, const H& info, const std::shared_ptr<Args>&... args) {
			return std::make_shared<_self_t>(argv, info, args...);
		}
		template<class... Args, class H = _help_data_t, _enable_if_all_args_t<_help_data_t, Args...> = true,
			_enable_if_help_data_t<H, _help_data_t> = true>
		static _self_ptr_t create(char** argv, const size_t& values_num, const H& info,
			const std::shared_ptr<Args>&... args) {
			return std::make_shared<_self_t>(argv, values_num, info, args...);
		}
		template<class... Args, class H = _help_data_t, _enable_if_all_args_t<_help_data_t, Args...> = true,
			_enable_if_help_data_t<H, _help_data_t> = true>
		static _self_ptr_t create(char** argv, const _values_list_t& values, const H& info,
			const std::shared_ptr<Args>&... args) {
			return std::make_shared<_self_t>(argv, values, info, args...);
		}

		std::unordered_map<std::string, arg_value> analize(const int& argc, char** argv) {
			if (argc == 1) {
				return std::unordered_map<std::string, arg_value>();
			}

			_analize_data data = _analize_data();

			// Get Program Data
			const _arg_data& arg_info = _get_arg_data("");

			// Read Group Args
			uint32_t pargc = argc - 1;
			if (arg_info.has_values()) {
				if (mstd::sub_overflow(pargc, arg_info.values_num, pargc)) {
					throw args_exception("Expected values at the end of program call");
				}
			}
			char** pargv = argv + 1;
			uint32_t v = 0;
			for (; v < pargc; ++v) {
				_read_arg(v, pargc, pargv, data);
			}

			// Get Value
			arg_value program_value = arg_value();
			if (arg_info.has_values()) {
				if (arg_info.values_num == 1) {
					_increment_v(v, argc);
					program_value = _read_value(argv[v]);
				}
				else {
					for (size_t i = 0; i != arg_info.values_num; ++i) {
						_increment_v(v, argc);
						program_value.push_back(_read_value(argv[v]));
					}
				}
			}

			data.values[""] = program_value;

			// enter default values (and do callbacks)
			for (const std::pair<std::string, arg_value>& value : _default_values) {
				if (data.values.contains(value.first)) {
					continue;
				}

				data.values.insert(value);

				const _arg_data& arg_info = _args_data.at(value.first);
				if (arg_info.has_callback()) {
					arg_info.callback.value()(value.second);
				}
			}

			// enter default flags (and do callbacks)
			for (const std::string& flag : _default_flags) {
				if (data.values.contains(flag)) {
					continue;
				}

				data.values.insert({ flag, arg_value() });

				const _arg_data& arg_info = _args_data.at(flag);
				if (arg_info.has_callback()) {
					arg_info.callback.value()(arg_value());
				}
			}

			return data.values;
		}
	};
}