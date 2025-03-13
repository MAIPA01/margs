#pragma once
#include "margs_types.hpp"
#include "arg_value.hpp"

namespace margs {
	template<bool test, class _help_data>
	struct _arg_info {};

	template<class _help_data>
	struct _arg_info<true, _help_data> {
		_arg_info(const _help_data& info) : _info(info) {}
		const _help_data _info; // used to create help message
	};

	template<class _help_data>
	class _basic_arg : public std::enable_shared_from_this<_basic_arg<_help_data>>, 
		private _arg_info<!std::is_void_v<_help_data>, _help_data> {
	private:
		// flag: (--_name | -_short_name)
		// value: (--_name | -_short_name) <value>+
		// group: _name <arg>* <value>*

		using _self_t = _basic_arg<_help_data>;
		using _self_sptr_t = _basic_arg_sptr<_help_data>;
		using _arg_info_base_t = _arg_info<!std::is_void_v<_help_data>, _help_data>;
		using _values_union_t = std::variant<size_t, arg_value>;
		using _opt_short_name_t = std::optional<_short_name_t>;
		using _args_list_t = _args_list_t<_help_data>;
		using _opt_args_list_t = std::optional<_args_list_t>;
		using _opt_callback_t = std::optional<arg_callback_t>;

		template<class V>
		static constexpr bool _is_values_num_v = (std::is_arithmetic_v<V> || std::is_convertible_v<V, size_t>);
		template<class V>
		static constexpr bool _is_default_values_v = (std::is_same_v<V, _values_list_t> || 
			std::is_convertible_v<V, _values_list_t>);

		template<class V>
		using _enable_if_values_t = std::enable_if_t<(_is_values_num_v<V> || _is_default_values_v<V>), bool>;

		const std::string _name;					// for flag, value and group (mandatory)
		const _opt_short_name_t _short_name;		// for flag and value (no for group)
		const bool _is_default = false;				// for flag (no for value and no for group)
		const _values_union_t _values = 0ULL;		// for value (mandatory) and group (optional) (no for flag)
		const bool _has_default_values = false;		// for value (optional) and group (optional) (no for flag)
		const _opt_args_list_t _args;				// for group (optional) (no for flag and value)
		const _opt_callback_t _callback;			// for flag, value and group (optional)

		template<class V, _enable_if_values_t<V> = true>
		static constexpr auto _init_values(const V& values) {
			if constexpr (_is_values_num_v<V>) {
				return size_t(values);
			}
			else {
				_values_list_t values_list = _values_list_t(values);
				if (values_list.size() == 0) {
					return arg_value();
				}
				
				if (values_list.size() == 1) {
					return arg_value(values_list.at(0));
				}

				return arg_value(values);
			}
		}
	public:
		using help_data_type = _help_data;

		// for flag:
		// name, is_default, callback?								<- make flag
		// name, short, is_default, callback?						<- make flag

		// for value:
		// name, (values_num | default_values), callback?			<- if values_num == 0 then make flag else value
		// name, short, (values_num | default_values), callback?	<- if values_num == 0 then make flag else value

		// for group:
		// name, callback?											<- use (name, args, callback?) to make group with args.size() == 0
		// name, (values_num | default_values), callback?			<- use (name, args, (values_num | default_values), callback?) to make group with args.size() == 0
		// name, args, callback?									<- make group
		// name, args, (values_num | default_values), callback?		<- make group

		// unique constructors:
		// name, callback?
		// name, short, callback?
		// 
		// name, (values_num | default_values), callback?
		// name, short, (values_num | default_values), callback?
		// 
		// name, args, callback?
		// name, args, (values_num | default_values), callback?

#pragma region BASIC
		// flag
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		_basic_arg(const std::string& name, const bool& is_default, 
			const _opt_callback_t& callback = _opt_callback_t())
			: _name(name), _is_default(is_default), _callback(callback) {}
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		_basic_arg(const std::string& name, const bool& is_default, const _short_name_t& short_name,
			const _opt_callback_t& callback = _opt_callback_t())
			: _name(name), _short_name(short_name), _is_default(is_default), _callback(callback) {}

		// value or flag if values_num == 0
		template<class V, class H = _help_data, _enable_if_values_t<V> = true,
			_enable_if_basic_t<H, _help_data> = true>
		_basic_arg(const std::string& name, const V& values,
			const _opt_callback_t& callback = _opt_callback_t())
			: _name(name), _values(_init_values(values)),
			_has_default_values(!_is_values_num_v<V>), _callback(callback) {
		}
		template<class V, class H = _help_data, _enable_if_values_t<V> = true,
			_enable_if_basic_t<H, _help_data> = true>
		_basic_arg(const std::string& name, const _short_name_t& short_name, const V& values,
			const _opt_callback_t& callback = _opt_callback_t())
			: _name(name), _short_name(short_name), _values(_init_values(values)),
			_has_default_values(!_is_values_num_v<V>), _callback(callback) {
		}

		// group
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		_basic_arg(const std::string& name, const _args_list_t& args,
			const _opt_callback_t& callback = _opt_callback_t())
			: _name(name), _args(args), _callback(callback) {}
		template<class V, class H = _help_data, _enable_if_values_t<V> = true,
			_enable_if_basic_t<H, _help_data> = true>
		_basic_arg(const std::string& name, const _args_list_t& args, const V& values,
			const _opt_callback_t& callback = _opt_callback_t())
			: _name(name), _values(_init_values(values)),
			_has_default_values(!_is_values_num_v<V>), _callback(callback) {
		}

		// flag
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _self_sptr_t create(const std::string& name, const bool& is_default,
			const _opt_callback_t& callback = _opt_callback_t()) {
			return std::make_shared<_self_t>(name, is_default, callback);
		}
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _self_sptr_t create(const std::string& name, const _short_name_t& short_name, 
			const bool& is_default, const _opt_callback_t& callback = _opt_callback_t()) {
			return std::make_shared<_self_t>(name, short_name, is_default, callback);
		}

		// value or flag if values_num == 0
		template<class V, class H = _help_data, _enable_if_values_t<V> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _self_sptr_t create(const std::string& name, const V& values,
			const _opt_callback_t& callback = _opt_callback_t()) {
			return std::make_shared<_self_t>(name, values, callback);
		}
		template<class V, class H = _help_data, _enable_if_values_t<V> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _self_sptr_t create(const std::string& name, const _short_name_t& short_name, const V& values,
			const _opt_callback_t& callback = _opt_callback_t()) {
			return std::make_shared<_self_t>(name, short_name, values, callback);
		}

		// group
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _self_sptr_t create(const std::string& name, const _args_list_t& args,
			const _opt_callback_t& callback = _opt_callback_t()) {
			return std::make_shared<_self_t>(name, args, callback);
		}
		template<class V, class H = _help_data, _enable_if_values_t<V> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _self_sptr_t create(const std::string& name, const _args_list_t& args, const V& values,
			const _opt_callback_t& callback = _opt_callback_t()) {
			return std::make_shared<_self_t>(name, args, values, callback);
		}
#pragma endregion // BASIC

#pragma region HELP_DATA
		// flag
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		_basic_arg(const std::string& name, const bool& is_default, const H& info,
			const _opt_callback_t& callback = _opt_callback_t()) 
			: _arg_info_base_t(info), _name(name), _is_default(_is_default), _callback(callback) {}
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		_basic_arg(const std::string& name, const _short_name_t& short_name, const bool& is_default, 
			const H& info, const _opt_callback_t& callback = _opt_callback_t()) 
			: _arg_info_base_t(info), _name(name), _short_name(short_name), _is_default(is_default), 
			_callback(callback) {}

		// value or flag if values_num == 0
		template<class V, class H = _help_data, _enable_if_values_t<V> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		_basic_arg(const std::string& name, const V& values, const H& info,
			const _opt_callback_t& callback = _opt_callback_t())
			: _name(name), _values(_init_values(values)),
			_has_default_values(!_is_values_num_v<V>), _callback(callback) {
			_arg_info_base_t::_info = info;
		}
		template<class V, class H = _help_data, _enable_if_values_t<V> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		_basic_arg(const std::string& name, const _short_name_t& short_name, const V& values,
			const H& info, const _opt_callback_t& callback = _opt_callback_t())
			: _arg_info_base_t(info), _name(name), _short_name(short_name), _values(_init_values(values)),
			_has_default_values(!_is_values_num_v<V>), _callback(callback) {
		}

		// group
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		_basic_arg(const std::string& name, const _args_list_t& args, const H& info,
			const _opt_callback_t& callback = _opt_callback_t()) 
			: _arg_info_base_t(info), _name(name), _args(args), _callback(callback) {}
		template<class V, class H = _help_data, _enable_if_values_t<V> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		_basic_arg(const std::string& name, const _args_list_t& args, const V& values,
			const H& info, const _opt_callback_t& callback = _opt_callback_t())
			: _arg_info_base_t(info), _name(name), _values(_init_values(values)), _has_default_values(!_is_values_num_v<V>),
			_callback(callback) {}

		// flag
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _self_sptr_t create(const std::string& name, const H& info,
			const _opt_callback_t& callback = _opt_callback_t()) {
			return std::make_shared<_self_t>(name, info, callback);
		}
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _self_sptr_t create(const std::string& name, const _short_name_t& short_name,
			const H& info, const _opt_callback_t& callback = _opt_callback_t()) {
			return std::make_shared<_self_t>(name, short_name, info, callback);
		}

		// value or flag if values_num == 0
		template<class V, class H = _help_data, _enable_if_values_t<V> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _self_sptr_t create(const std::string& name, const V& values, const H& info,
			const _opt_callback_t& callback = _opt_callback_t()) {
			return std::make_shared<_self_t>(name, values, info, callback);
		}
		template<class V, class H = _help_data, _enable_if_values_t<V> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _self_sptr_t create(const std::string& name, const _short_name_t& short_name, const V& values,
			const H& info, const _opt_callback_t& callback = _opt_callback_t()) {
			return std::make_shared<_self_t>(name, short_name, values, info, callback);
		}

		// group
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _self_sptr_t create(const std::string& name, const _args_list_t& args,
			const H& info, const _opt_callback_t& callback = _opt_callback_t()) {
			return std::make_shared<_self_t>(name, args, info, callback);
		}
		template<class V, class H = _help_data, _enable_if_values_t<V> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _self_sptr_t create(const std::string& name, const _args_list_t& args, const V& values,
			const H& info, const _opt_callback_t& callback = _opt_callback_t()) {
			return std::make_shared<_self_t>(name, args, values, info, callback);
		}

		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		H getHelpInfo() const { return _arg_info_base_t::_info; }
#pragma endregion // HELP_DATA

		virtual ~_basic_arg() = default;

		std::string getName() const { return _name; }
		bool hasShortName() const { return _short_name.has_value(); }
		char getShortName() const { return _short_name.value_or('\0'); }
		bool isDefault() const { return _is_default; }
		bool hasCallback() const { return _callback.has_value(); }
		arg_callback_t getCallback() const { return _callback.value(); }
		bool hasValues() const {
			if (_has_default_values) return true;
			return get<size_t>(_values) != 0;
		}
		size_t getValuesNum() const {
			if (_has_default_values) {
				const arg_value& value = get<arg_value>(_values);
				if (value.isScalar() || value.isNull()) return 1;
				else if (value.isSequence()) return value.size();
			}
			return get<size_t>(_values);
		}
		bool hasDefaultValues() const { return _has_default_values; }
		arg_value getDefaultValues() const {
			if (_has_default_values) return get<arg_value>(_values);
			return arg_value();
		}
		bool hasArgs() const { return _args.has_value(); }
		_args_list_t getArgs() const { return _args.value_or(_args_list_t()); }
		size_t getArgsNum() const {
			if (_args.has_value()) return _args.value();
			else return 0;
		}

		arg_type getType() const {
			if (_args.has_value()) {
				return arg_type::group;
			}

			if (hasValues()) {
				return arg_type::value;
			}

			return arg_type::flag;
		}
	};
}