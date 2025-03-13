#pragma once
#include "margs_types.hpp"
#include "basic_arg.hpp"

namespace margs {
	template<class _help_data>
	class basic_args_builder {
	private:
#pragma region TYPES
		using _arg_t = _basic_arg<_help_data>;
		using _arg_sptr_t = _basic_arg_sptr<_help_data>;
		using _args_list_t = _args_list_t<_help_data>;
#pragma endregion // TYPES

#pragma region CALLBACKS
		using _flag_callback_t = std::function<void(void)>;

		template<class... Ts>
		using _values_callback_t = std::function<void(const Ts&...)>;
#pragma endregion // CALLBACKS

#pragma region CONSTEXPR_BOOL
		template<class... Ts>
		static constexpr bool _is_any_void_v = (std::is_void_v<Ts> || ...);
		template<class... Ts>
		static constexpr bool _is_valid_values_v = (sizeof...(Ts) > 0 && !_is_any_void_v<Ts...>);

		template<class F, class T>
		struct _is_valid_values_callback_impl;
		template<class F, class... Args>
		struct _is_valid_values_callback_impl<F, std::tuple<Args...>> : std::conjunction<
			std::bool_constant<_is_valid_values_v<Args...>>,
			mstd::is_same_function<F, _values_callback_t<Args...>>
		> {
		};
		template<class F, class = void>
		struct _is_valid_values_callback : std::false_type {};
		template<class F>
		struct _is_valid_values_callback<F, std::void_t<mstd::function_args_t<F>>>
			: _is_valid_values_callback_impl<F, mstd::function_args_t<F>> {
		};
		template<class F>
		static constexpr bool _is_valid_values_callback_v = _is_valid_values_callback<F>::value;

		template<class F>
		static constexpr bool _is_valid_flag_callback_v = (mstd::is_same_function_v<F, _flag_callback_t> || 
			mstd::is_same_function_v<F, arg_callback_t>);

#pragma endregion // CONSTEXPR_BOOL

#pragma region ENABLE_IF
		template<class... Ts>
		using _enable_if_valid_values_t = std::enable_if_t<_is_valid_values_v<Ts...>, bool>;
		template<class F>
		using _enable_if_valid_values_callback_t = std::enable_if_t<_is_valid_values_callback_v<F>, bool>;
		template<class F>
		using _enable_if_valid_flag_callback_t = std::enable_if_t<_is_valid_flag_callback_v<F>, bool>;
#pragma endregion // ENABLE_IF

#pragma region CALLBACK_LAMBDA
		template<class... Ts, size_t... Is>
		static constexpr void _call_values_callback(const _values_callback_t<Ts...>& callback, 
			const _values_list_t& values, const std::index_sequence<Is...>&) {
			static_assert(sizeof...(Ts) == values.size(), "Values size should be equal Types number");
			callback(values.at(Is).as<Ts>()...);
		}
		
		static constexpr arg_callback_t _make_lambda(const _flag_callback_t& callback) {
			return [callback](const arg_value&) -> void { callback(); };
		}
		template<class T, class... Ts>
		static constexpr arg_callback_t _make_lambda(const _values_callback_t<T, Ts...>& callback) {
			if constexpr (mstd::is_same_function_v<_values_callback_t<T, Ts...>, arg_callback_t>) {
				return callback;
			}
			else {
				if constexpr (sizeof...(Ts) == 0) {
					return [callback](const arg_value& value) -> void {
						callback(value.as<T>());
					};
				}
				else {
					return [callback](const arg_value& value) -> void {
						_call_values_callback<T, Ts...>(callback, 
							value.as<_values_list_t>(), std::make_index_sequence<sizeof...(Ts) + 1>());
					};
				}
			}
		}
#pragma endregion // CALLBACK_LAMBDA

		basic_args_builder() = default;
	public:
		using help_data_type = _help_data;

		virtual ~basic_args_builder() = default;

#pragma region BASIC_MAKERS
		// BASIC FLAG
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeFlag(const std::string& name, const bool& is_default = false) {
			return _arg_t::create(name, is_default);
		}
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeFlag(const std::string& name, const _short_name_t& short_name, 
			const bool& is_default = false) {
			return _arg_t::create(name, short_name, is_default);
		}

		// CALLBACK FLAG
		template<class F, class H = _help_data, _enable_if_valid_flag_callback_t<F> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeFlag(const std::string& name, const F& callback, const bool& is_default = false) {
			return _arg_t::create(name, is_default, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_flag_callback_t<F> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeFlag(const std::string& name, const _short_name_t& short_name, 
			const F& callback, const bool& is_default = false) {
			return _arg_t::create(name, short_name, is_default, _make_lambda(as_function_t<F>(callback)));
		}

		// BASIC VALUE
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const size_t& values_num) {
			return _arg_t::create(name, values_num);
		}
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const _values_list_t& values) {
			return _arg_t::create(name, values);
		}
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const _short_name_t& short_name,
			const size_t& values_num) {
			return _arg_t::create(name, short_name, values_num);
		}
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const _short_name_t& short_name,
			const _values_list_t& values) {
			return _arg_t::create(name, short_name, values);
		}

		// CALLBACK VALUE
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const size_t& values_num,
			const F& callback) {
			return _arg_t::create(name, values_num, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const _values_list_t& values,
			const F& callback) {
			return _arg_t::create(name, values, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const _short_name_t& short_name,
			const size_t& values_num, const F& callback) {
			return _arg_t::create(name, short_name, values_num, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const _short_name_t& short_name,
			const _values_list_t& values, const F& callback) {
			return _arg_t::create(name, short_name, values, _make_lambda(as_function_t<F>(callback)));
		}

		// BASIC GROUP
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name) {
			return _arg_t::create(name, _args_list_t());
		}
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const size_t& values_num) {
			return _arg_t::create(name, _args_list_t(), values_num);
		}
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _values_list_t& values) {
			return _arg_t::create(name, _args_list_t(), values);
		}
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _args_list_t& args) {
			return _arg_t::create(name, args);
		}
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _args_list_t& args,
			const size_t& values_num) {
			return _arg_t::create(name, args, values_num);
		}
		template<class H = _help_data, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _args_list_t& args,
			const _values_list_t& values) {
			return _arg_t::create(name, args, values);
		}

		// CALLBACK GROUP
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const F& callback) {
			return _arg_t::create(name, _args_list_t(), _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const size_t& values_num,
			const F& callback) {
			return _arg_t::create(name, _args_list_t(), values_num, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _values_list_t& values,
			const F& callback) {
			return _arg_t::create(name, _args_list_t(), values, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _args_list_t& args,
			const F& callback) {
			return _arg_t::create(name, args, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _args_list_t& args,
			const size_t& values_num, const F& callback) {
			return _arg_t::create(name, args, values_num, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _args_list_t& args,
			const _values_list_t& values, const F& callback) {
			return _arg_t::create(name, args, values, _make_lambda(as_function_t<F>(callback)));
		}

		// ARGS GROUP
		template<class... Args, class H = _help_data, _enable_if_all_args_t<_help_data, Args...> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const std::shared_ptr<Args>&... args) {
			return _arg_t::create(name, { args... });
		}
		template<class... Args, class H = _help_data, _enable_if_all_args_t<_help_data, Args...> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const std::shared_ptr<Args>&... args,
			const size_t& values_num) {
			return _arg_t::create(name, { args... }, values_num);
		}
		template<class... Args, class H = _help_data, _enable_if_all_args_t<_help_data, Args...> = true,
			_enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const std::shared_ptr<Args>&... args,
			const _values_list_t& values) {
			return _arg_t::create(name, { args... }, values);
		}

		// CALLBACK ARGS GROUP
		template<class F, class... Args, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_all_args_t<_help_data, Args...> = true, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const std::shared_ptr<Args>&... args,
			const F& callback) {
			return _arg_t::create(name, { args... }, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class... Args, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_all_args_t<_help_data, Args...> = true, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const std::shared_ptr<Args>&... args,
			const size_t& values_num, const F& callback) {
			return _arg_t::create(name, { args... }, values_num, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class... Args, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_all_args_t<_help_data, Args...> = true, _enable_if_basic_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const std::shared_ptr<Args>&... args,
			const _values_list_t& values, const F& callback) {
			return _arg_t::create(name, { args... }, values, _make_lambda(as_function_t<F>(callback)));
		}
#pragma endregion // BASIC_MAKERS

#pragma region HELP_DATA_MAKERS
		// BASIC FLAG
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeFlag(const std::string& name, const H& info, const bool& is_default = false) {
			return _arg_t::create(name, is_default, info);
		}
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeFlag(const std::string& name, const _short_name_t& short_name, const H& info, 
			const bool& is_default = false) {
			return _arg_t::create(name, short_name, is_default, info);
		}

		// CALLBACK FLAG
		template<class F, class H = _help_data, _enable_if_valid_flag_callback_t<F> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeFlag(const std::string& name, const H& info, const F& callback, 
			const bool& is_default = false) {
			return _arg_t::create(name, is_default, info, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_flag_callback_t<F> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeFlag(const std::string& name, const _short_name_t& short_name, const H& info, 
			const F& callback, const bool& is_default = false) {
			return _arg_t::create(name, short_name, is_default, info, _make_lambda(as_function_t<F>(callback)));
		}

		// BASIC VALUE
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const size_t& values_num, const H& info) {
			return _arg_t::create(name, values_num, info);
		}
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const _values_list_t& values, const H& info) {
			return _arg_t::create(name, values, info);
		}
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const _short_name_t& short_name,
			const size_t& values_num, const H& info) {
			return _arg_t::create(name, short_name, values_num, info);
		}
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const _short_name_t& short_name,
			const _values_list_t& values, const H& info) {
			return _arg_t::create(name, short_name, values, info);
		}

		// CALLBACK VALUE
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const size_t& values_num, const H& info,
			const F& callback) {
			return _arg_t::create(name, values_num, info, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const _values_list_t& values, const H& info,
			const F& callback) {
			return _arg_t::create(name, values, info, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const _short_name_t& short_name,
			const size_t& values_num, const H& info, const F& callback) {
			return _arg_t::create(name, short_name, values_num, info, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeValue(const std::string& name, const _short_name_t& short_name,
			const _values_list_t& values, const H& info, const F& callback) {
			return _arg_t::create(name, short_name, values, info, _make_lambda(as_function_t<F>(callback)));
		}

		// BASIC GROUP
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const H& info) {
			return _arg_t::create(name, _args_list_t(), info);
		}
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const size_t& values_num, const H& info) {
			return _arg_t::create(name, _args_list_t(), values_num, info);
		}
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _values_list_t& values, const H& info) {
			return _arg_t::create(name, _args_list_t(), values, info);
		}
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _args_list_t& args, const H& info) {
			return _arg_t::create(name, args, info);
		}
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _args_list_t& args,
			const size_t& values_num, const H& info) {
			return _arg_t::create(name, args, values_num, info);
		}
		template<class H = _help_data, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _args_list_t& args,
			const _values_list_t& values, const H& info) {
			return _arg_t::create(name, args, values, info);
		}

		// CALLBACK GROUP
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const H& info, const F& callback) {
			return _arg_t::create(name, _args_list_t(), info, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const size_t& values_num, const H& info,
			const F& callback) {
			return _arg_t::create(name, _args_list_t(), values_num, info, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _values_list_t& values, const H& info,
			const F& callback) {
			return _arg_t::create(name, _args_list_t(), values, info, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _args_list_t& args, const H& info,
			const F& callback) {
			return _arg_t::create(name, args, info, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _args_list_t& args,
			const size_t& values_num, const H& info, const F& callback) {
			return _arg_t::create(name, args, values_num, info, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class H = _help_data, _enable_if_valid_values_callback_t<F> = true,
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const _args_list_t& args,
			const _values_list_t& values, const H& info, const F& callback) {
			return _arg_t::create(name, args, values, info, _make_lambda(as_function_t<F>(callback)));
		}

		// ARGS GROUP
		template<class... Args, class H = _help_data, _enable_if_all_args_t<_help_data, Args...> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const std::shared_ptr<Args>&... args, 
			const H& info) {
			return _arg_t::create(name, { args... }, info);
		}
		template<class... Args, class H = _help_data, _enable_if_all_args_t<_help_data, Args...> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const std::shared_ptr<Args>&... args,
			const size_t& values_num, const H& info) {
			return _arg_t::create(name, { args... }, values_num, info);
		}
		template<class... Args, class H = _help_data, _enable_if_all_args_t<_help_data, Args...> = true, 
			_enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const std::shared_ptr<Args>&... args,
			const _values_list_t& values, const H& info) {
			return _arg_t::create(name, { args... }, values, info);
		}

		// CALLBACK ARGS GROUP
		template<class F, class... Args, class H = _help_data, _enable_if_valid_values_callback_t<F> = true, 
			_enable_if_all_args_t<_help_data, Args...> = true, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const std::shared_ptr<Args>&... args, 
			const H& info, const F& callback) {
			return _arg_t::create(name, { args... }, info, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class... Args, class H = _help_data, _enable_if_valid_values_callback_t<F> = true, 
			_enable_if_all_args_t<_help_data, Args...> = true, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const std::shared_ptr<Args>&... args,
			const size_t& values_num, const H& info, const F& callback) {
			return _arg_t::create(name, { args... }, values_num, info, _make_lambda(as_function_t<F>(callback)));
		}
		template<class F, class... Args, class H = _help_data, _enable_if_valid_values_callback_t<F> = true, 
			_enable_if_all_args_t<_help_data, Args...> = true, _enable_if_help_data_t<H, _help_data> = true>
		static _arg_sptr_t makeGroup(const std::string& name, const std::shared_ptr<Args>&... args,
			const _values_list_t& values, const H& info, const F& callback) {
			return _arg_t::create(name, { args... }, values, info, _make_lambda(as_function_t<F>(callback)));
		}
#pragma endregion // HELP_DATA_MAKERS
	};
}