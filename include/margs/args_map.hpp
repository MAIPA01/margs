/*
 * margs - Maipa's Args Analyzer
 *
 * Licensed under the BSD 3-Clause License with Attribution Requirement.
 * See the LICENSE file for details: https://github.com/MAIPA01/margs/blob/main/LICENSE
 *
 * Copyright (c) 2025, Patryk Antosik (MAIPA01)
 */

#pragma once
#include "margs_types.hpp"
#include "arg_value.hpp"

namespace margs {
	class args_map {
	private:
		std::optional<arg_value> _program = std::optional<arg_value>();
		std::unordered_set<std::string> _flags;
		std::unordered_map<std::string, arg_value> _values;
		std::unordered_map<std::string, std::optional<arg_value>> _groups;

		static std::string _get_arg_name(const std::string& name) {
			const std::string::const_iterator& arg_name_begin =
				std::find_if(name.rbegin(), name.rend(), 
					[](const char& c) -> bool { return c == ':'; }).base();
			return mstd::concat(std::string(name.begin(), arg_name_begin), "--",
				std::string(arg_name_begin, name.end()));
		}
	public:
		args_map() = default;
		virtual ~args_map() = default;

		void set_program(const arg_value& value) {
			_program = value;
		}

		void insert_flag(const std::string& flag_name) {
			_flags.insert(flag_name);
		}

		void insert_value(const std::string& value_name, const arg_value& value) {
			_values.insert({ value_name, value });
		}

		void insert_group(const std::string& group_name) {
			_groups.insert({ group_name, std::optional<arg_value>() });
		}

		void insert_group(const std::string& group_name, const arg_value& value) {
			_groups.insert({ group_name, value });
		}

		bool contains_flag(const std::string& flag_name) const {
			return _flags.contains(_get_arg_name(flag_name));
		}

		bool contains_value(const std::string& value_name) const {
			return _values.contains(_get_arg_name(value_name));
		}

		bool contains_group(const std::string& group_name) const {
			return _groups.contains(group_name);
		}

		bool program_has_value() const {
			return _program.has_value();
		}

		bool group_has_value(const std::string& group_name) const {
			if (!contains_group(group_name)) return false;
			return _groups.at(group_name).has_value();
		}

		arg_value get_program_value() const {
			if (!program_has_value()) return arg_value();
			return _program.value();
		}

		arg_value get_value(const std::string& value_name) const {
			if (!contains_value(value_name)) return arg_value();
			return _values.at(_get_arg_name(value_name));
		}

		arg_value get_group_value(const std::string& group_name) const {
			if (!group_has_value(group_name)) return arg_value();
			return _groups.at(group_name).value();
		}

		std::optional<arg_value> get_program() const {
			return _program;
		}

		std::unordered_set<std::string> get_flags() const {
			return _flags;
		}

		std::unordered_map<std::string, arg_value> get_values() const {
			return _values;
		}

		std::unordered_map<std::string, std::optional<arg_value>> get_groups() const {
			return _groups;
		}
	};
}