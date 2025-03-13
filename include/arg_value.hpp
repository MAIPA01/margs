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
#include "args_exception.hpp"

namespace margs {
	class arg_value {
	private:
		enum class _value_type : uint8_t {
			null = 0,
			scalar = 1,
			sequence = 2
		};

		using _string_ptr = std::shared_ptr<std::string>;
		using _vector_type = std::vector<arg_value>;
		using _vector_ptr = std::shared_ptr<_vector_type>;
		using _inizializer_type = std::initializer_list<arg_value>;

	public:
		using iterator = _vector_type::iterator;
		using const_iterator = _vector_type::const_iterator;
		using reverse_iterator = _vector_type::reverse_iterator;
		using const_reverse_iterator = _vector_type::const_reverse_iterator;

	private:

		_value_type _type;
		std::variant<_string_ptr, _vector_ptr> _value;

		// scalar
		constexpr void _init_scalar() {
			if (!isNull()) return;

			_type = _value_type::scalar;
			_value = std::make_shared<std::string>();
		}
		constexpr void _check_scalar() const {
			if (!isScalar()) {
				throw args_exception("This operation is only allowed when type is scalar");
			}
		}

		// sequence
		constexpr void _init_sequence() {
			if (!isNull()) return;

			_type = _value_type::sequence;
			_value = std::make_shared<_vector_type>();
		}
		constexpr void _check_sequence() const {
			if (!isSequence()) {
				throw args_exception("This operation is only allowed when type is sequence");
			}
		}

		// clone
		constexpr arg_value(const _value_type& type, const std::variant<_string_ptr, _vector_ptr>& value) 
			: _type(type) {
			if (isNull()) return;

			if (isScalar()) {
				_value = make_shared<std::string>(*get<_string_ptr>(value));
			}
			else if (isSequence()) {
				_value = std::make_shared<_vector_type>();
				for (const arg_value& arg : *get<_vector_ptr>(value)) {
					push_back(arg.clone());
				}
			}
		}
	public:
		constexpr arg_value() : _type(_value_type::null) {}
		constexpr arg_value(const arg_value& value) : _type(value._type), _value(value._value) {}
		constexpr arg_value(const std::string& value) : _type(_value_type::scalar), 
			_value(std::make_shared<std::string>(value)) {}
		arg_value(const _vector_type& values) : _type(_value_type::sequence) {
			_vector_ptr curr_value = std::make_shared<_vector_type>();

			for (const arg_value& value : values) {
				curr_value->push_back(value);
			}

			_value = curr_value;
		}
		arg_value(const _inizializer_type& values) {
			_vector_ptr curr_value = std::make_shared<_vector_type>();

			for (const arg_value& value : values) {
				curr_value->push_back(value);
			}

			_value = curr_value;
		}

		virtual ~arg_value() = default;

		constexpr bool isNull() const {
			return _type == _value_type::null;
		}
		constexpr bool isScalar() const {
			return _type == _value_type::scalar;
		}
		constexpr bool isSequence() const {
			return _type == _value_type::sequence;
		}

		constexpr iterator begin() {
			_init_sequence();
			_check_sequence();
			return get<_vector_ptr>(_value)->begin();
		}
		constexpr iterator end() {
			_init_sequence();
			_check_sequence();
			return get<_vector_ptr>(_value)->end();
		}
		constexpr const_iterator begin() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->cbegin();
		}
		constexpr const_iterator end() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->cend();
		}
		constexpr const_iterator cbegin() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->cbegin();
		}
		constexpr const_iterator cend() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->cend();
		}
		constexpr reverse_iterator rbegin() {
			_init_sequence();
			_check_sequence();
			return get<_vector_ptr>(_value)->rbegin();
		}
		constexpr reverse_iterator rend() {
			_init_sequence();
			_check_sequence();
			return get<_vector_ptr>(_value)->rend();
		}
		constexpr const_reverse_iterator rbegin() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->crbegin();
		}
		constexpr const_reverse_iterator rend() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->crend();
		}
		constexpr const_reverse_iterator crbegin() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->crbegin();
		}
		constexpr const_reverse_iterator crend() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->crend();
		}

		constexpr void push_back(const arg_value& value) {
			_init_sequence();
			_check_sequence();
			get<_vector_ptr>(_value)->push_back(value);
		}
		constexpr void push_back(const std::string& value) {
			push_back(arg_value(value));
		}

		template<class _Iter>
		constexpr void insert(const const_iterator& where, const _Iter& begin, const _Iter& end) {
			_init_sequence();
			_check_sequence();
			get<_vector_ptr>(_value)->insert(where, begin, end);
		}

		constexpr size_t size() {
			_init_sequence();
			_check_sequence();
			return get<_vector_ptr>(_value)->size();
		}
		constexpr size_t size() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->size();
		}

		arg_value clone() const {
			return arg_value(_type, _value);
		}

		template<class T>
		constexpr T as() const {
			T value = T();
			if (decoder<T>::decode(*this, value)) {
				return value;
			}
			throw args_exception("Can't decode to desire type");
		}
		template<>
		arg_value as<arg_value>() const {
			return *this;
		}
		template<>
		constexpr std::string as<std::string>() const {
			if (!isScalar()) {
				throw args_exception("Can't decode to desire type");
			}
			return *get<_string_ptr>(_value);
		}
		template<>
		constexpr _vector_type as<_vector_type>() const {
			if (!isSequence()) {
				throw args_exception("Can't decode to desire type");
			}
			return *get<_vector_ptr>(_value);
		}
		
		template<class T>
		constexpr operator T() {
			return as<T>();
		}

		constexpr arg_value& operator=(const arg_value& value) {
			if (!isNull() && _type != value._type) {
				throw args_exception("This operation is only allowed when type is null or types are equal");
			}

			if (isNull()) {
				_type = value._type;
			}

			_value = value._value;
			return *this;
		}
		constexpr arg_value& operator=(const std::string& value) {
			_init_scalar();
			_check_scalar();
			*get<_string_ptr>(_value) = value;
			return *this;
		}
		constexpr arg_value& operator=(const _vector_type& values) {
			_init_sequence();
			_check_sequence();
			*get<_vector_ptr>(_value) = values;
			return *this;
		}
		constexpr arg_value& operator=(const _inizializer_type& values) {
			_init_sequence();
			_check_sequence();
			*get<_vector_ptr>(_value) = values;
			return *this;
		}

		constexpr bool operator==(const arg_value& other) const {
			if (_type != other._type) return false;
			if (isNull()) {
				return true;
			}
			else if (isScalar()) {
				const _string_ptr& scalar_ptr = std::get<_string_ptr>(_value);
				const _string_ptr& other_scalar_ptr = std::get<_string_ptr>(other._value);
				return scalar_ptr == other_scalar_ptr || *scalar_ptr == *other_scalar_ptr;
			}
			else if (isSequence()) {
				const _vector_ptr& sequence_ptr = std::get<_vector_ptr>(_value);
				const _vector_ptr& other_sequence_ptr = std::get<_vector_ptr>(other._value);
				return sequence_ptr == other_sequence_ptr || *sequence_ptr == *other_sequence_ptr;
			}
			return false;
		}
		constexpr bool operator!=(const arg_value& other) const {
			return !(*this == other);
		}

		arg_value operator[](size_t idx) {
			_init_sequence();
			_check_sequence();
			return get<_vector_ptr>(_value)->at(idx);
		}
		arg_value operator[](size_t idx) const {
			_check_sequence();
			return get<_vector_ptr>(_value)->at(idx);
		}
	};
}

#include "decoder.hpp"