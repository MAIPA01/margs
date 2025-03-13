/*
 * margs - Maipa's Args Analyzer
 *
 * Licensed under the BSD 3-Clause License with Attribution Requirement.
 * See the LICENSE file for details: https://github.com/MAIPA01/margs/blob/main/LICENSE
 *
 * Copyright (c) 2025, Patryk Antosik (MAIPA01)
 */

#pragma once
#include "margs_libs.hpp"

// TODO: analizer zwraca mapê podzielon¹ na grupy, value i flagi (w³asny typ mapy args_map pozwala na getFlag("name") itp)

namespace margs {
	// EXCEPTIONS
	class args_exception;

	// ARG VALUE
	class arg_value;

	template<class T>
	struct decoder;

	// CMD ARG CONTENER
	enum class arg_type : uint8_t {
		value = 0,
		flag = 1,
		group = 2
	};

	template<class _help_data = void>
	class _basic_arg;
	template<class _help_data = void>
	using _basic_arg_sptr = std::shared_ptr<_basic_arg<_help_data>>;

	// HELP STRUCTS
	template<class _help_data = void>
	struct arg_info;
	template<class _help_data = void>
	using arg_info_sptr = std::shared_ptr<arg_info<_help_data>>;
	struct help_data;
	struct help_message;

	// ARGS MAP
	class args_map;

	// BUILDER
	template<class _help_data = void>
	class basic_args_builder;

	using args_builder = basic_args_builder<help_data>;

	// ANALIZER
	template<class _help_message = void>
	class basic_args_analizer;

	using args_analizer = basic_args_analizer<help_message>;

	// COMMON TYPES
	using _short_name_t = unsigned char;
	using _values_list_t = std::vector<arg_value>;
	template<class _help_data>
	using _args_list_t = std::vector<_basic_arg_sptr<_help_data>>;
	using arg_callback_t = std::function<void(const arg_value&)>;

	// CONSTEXPR BOOL
	template<class _help_data, class... Args>
	constexpr bool _is_all_args_v = ((sizeof...(Args) > 0) && (std::is_same_v<_basic_arg<_help_data>, Args> && ...));

	template<class T, class _help_data>
	constexpr bool _is_basic_v = (std::is_void_v<T> && std::is_same_v<T, _help_data>);
	template<class T, class _help_data>
	constexpr bool _is_help_data_v = (!std::is_void_v<T> && std::is_same_v<T, _help_data>);

	// ENABLE IF
	template<class _help_data, class... Args>
	using _enable_if_all_args_t = std::enable_if_t<_is_all_args_v<_help_data, Args...>, bool>;

	template<class T, class _help_data>
	using _enable_if_basic_t = std::enable_if_t<_is_basic_v<T, _help_data>, bool>;
	template<class T, class _help_data>
	using _enable_if_help_data_t = std::enable_if_t<_is_help_data_v<T, _help_data>, bool>;
}