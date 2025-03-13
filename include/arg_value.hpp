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
		void _init_scalar() {
			if (!isNull()) return;

			_type = _value_type::scalar;
			_value = std::make_shared<std::string>();
		}
		void _check_scalar() const {
			if (!isScalar()) {
				throw args_exception("This operation is only allowed when type is scalar");
			}
		}

		// sequence
		void _init_sequence() {
			if (!isNull()) return;

			_type = _value_type::sequence;
			_value = std::make_shared<_vector_type>();
		}
		void _check_sequence() const {
			if (!isSequence()) {
				throw args_exception("This operation is only allowed when type is sequence");
			}
		}

		// clone
		arg_value(const _value_type& type, const std::variant<_string_ptr, _vector_ptr>& value) : _type(type) {
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
		arg_value() : _type(_value_type::null) {}
		arg_value(const arg_value& value) : _type(value._type), _value(value._value) {}
		arg_value(const std::string& value) : _type(_value_type::scalar), _value(make_shared<std::string>(value)) {}
		arg_value(const char* v) : arg_value(std::string(v)) {};
		template<size_t N>
		arg_value(const char(&v)[N]) : arg_value(std::string(v)) {};
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
		template<class T>
		arg_value(const T& value) : arg_value(converter<T>::encode(value)) {}

		virtual ~arg_value() = default;

		bool isNull() const {
			return _type == _value_type::null;
		}
		bool isScalar() const {
			return _type == _value_type::scalar;
		}
		bool isSequence() const {
			return _type == _value_type::sequence;
		}

		iterator begin() {
			_init_sequence();
			_check_sequence();
			return get<_vector_ptr>(_value)->begin();
		}
		iterator end() {
			_init_sequence();
			_check_sequence();
			return get<_vector_ptr>(_value)->end();
		}

		const_iterator begin() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->cbegin();
		}
		const_iterator end() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->cend();
		}

		const_iterator cbegin() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->cbegin();
		}
		const_iterator cend() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->cend();
		}

		reverse_iterator rbegin() {
			_init_sequence();
			_check_sequence();
			return get<_vector_ptr>(_value)->rbegin();
		}
		reverse_iterator rend() {
			_init_sequence();
			_check_sequence();
			return get<_vector_ptr>(_value)->rend();
		}

		const_reverse_iterator rbegin() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->crbegin();
		}
		const_reverse_iterator rend() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->crend();
		}

		const_reverse_iterator crbegin() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->crbegin();
		}
		const_reverse_iterator crend() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->crend();
		}

		void push_back(const std::string& value) {
			push_back(arg_value(value));
		}
		void push_back(const arg_value& value) {
			_init_sequence();
			_check_sequence();
			get<_vector_ptr>(_value)->push_back(value);
		}
		template<class T>
		void push_back(const T& value) {
			push_back(converter<T>::encode(value));
		}

		void insert(const const_iterator& where, const const_iterator& begin, const const_iterator& end) {
			_init_sequence();
			_check_sequence();
			get<_vector_ptr>(_value)->insert(where, begin, end);
		}

		size_t size() {
			_init_sequence();
			_check_sequence();
			return get<_vector_ptr>(_value)->size();
		}
		size_t size() const {
			_check_sequence();
			return get<_vector_ptr>(_value)->size();
		}

		arg_value clone() const {
			return arg_value(_type, _value);
		}

		template<class T>
		T as() const {
			T value;
			if (converter<T>::decode(*this, value)) {
				return value;
			}
			throw args_exception("Can't decode to desire type");
		}
		template<>
		arg_value as<arg_value>() const {
			return *this;
		}
		template<>
		std::string as<std::string>() const {
			if (!isScalar()) {
				throw args_exception("This operation is only allowed when type is scalar");
			}
			return *get<_string_ptr>(_value);
		}
		template<>
		_vector_type as<_vector_type>() const {
			if (!isSequence()) {
				throw args_exception("This operation is only allowed when type is sequence");
			}
			return *get<_vector_ptr>(_value);
		}
		template<class T>
		operator T() {
			return as<T>();
		}

		arg_value& operator=(const arg_value& value) {
			if (!isNull() && _type != value._type) {
				throw args_exception("This operation is only allowed when type is null or types are equal");
			}

			if (isNull()) {
				_type = value._type;
			}

			_value = value._value;
			return *this;
		}
		arg_value& operator=(const std::string& value) {
			_init_scalar();
			_check_scalar();
			*get<_string_ptr>(_value) = value;
			return *this;
		}
		arg_value& operator=(const _vector_type& values) {
			_init_sequence();
			_check_sequence();
			*get<_vector_ptr>(_value) = values;
			return *this;
		}
		arg_value& operator=(const _inizializer_type& values) {
			_init_sequence();
			_check_sequence();
			*get<_vector_ptr>(_value) = values;
			return *this;
		}
		template<class T>
		arg_value& operator=(const T& value) {
			return this->operator=(converter<T>::encode(value));
		}

		bool operator==(const arg_value& other) const {
			if (_type != other._type) return false;
			if (isNull()) {
				return true;
			}
			else if (isScalar()) {
				const _string_ptr& scalar_ptr = std::get<_string_ptr>(_value);
				const _string_ptr& other_scalar_ptr = std::get<_string_ptr>(other._value);
				if (scalar_ptr == other_scalar_ptr || *scalar_ptr == *other_scalar_ptr) return true;
			}
			else if (isSequence()) {
				const _vector_ptr& sequence_ptr = std::get<_vector_ptr>(_value);
				const _vector_ptr& other_sequence_ptr = std::get<_vector_ptr>(other._value);
				if (sequence_ptr == other_sequence_ptr) return true;
				else {
					const std::vector<arg_value>& this_values = *sequence_ptr;
					const std::vector<arg_value>& other_values = *other_sequence_ptr;
					if (this_values.size() != other_values.size()) return false;

					for (size_t i = 0; i != this_values.size(); ++i) {
						const arg_value& this_value = this_values.at(i);
						const arg_value& other_value = other_values.at(i);
						if (this_value != other_value) return false;
					}
					return true;
				}
			}
			return false;
		}
		template<class T>
		bool operator==(const T& other) const {
			return operator==(converter<T>::encode(other));
		}
		bool operator!=(const arg_value& other) const {
			return !(*this == other);
		}
		template<class T>
		bool operator!=(const T& other) const {
			return !operator==(other);
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

#include "converter.hpp"