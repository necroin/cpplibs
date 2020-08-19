#pragma once
#ifndef _ANY_H
#define _ANY_H
#include <exception>
#include <typeinfo>
#include <type_traits>

namespace any {
	class Any;

	template<class _Type>
	_Type any_cast(Any& any);

	class bad_any_cast : public std::exception {
	public:
		bad_any_cast() : exception() {}
		bad_any_cast(const char* message) : exception(message) {}
	};
};

namespace any {
	namespace meta_functions {

		template<class _Type>
		struct type_qualifier {
			static _Type& execute(size_t type_id, void* object) {
				if (type_id == typeid(_Type).hash_code()) {
					return *static_cast<_Type*>(object);
				}
				else {
					throw bad_any_cast("bad any_cast");
				}
			}
		};

		template<class _Type>
		struct type_qualifier<_Type&> {
			static _Type& execute(size_t type_id, void* object) {
				if (type_id == typeid(_Type).hash_code()) {
					return *static_cast<_Type*>(object);
				}
				else {
					throw bad_any_cast("bad any_cast");
				}
			}
		};

		template<class _Type>
		struct type_qualifier<_Type*> {
			static _Type* execute(size_t type_id, void* object) {
				if (type_id == typeid(_Type).hash_code()) {
					return static_cast<_Type*>(object);
				}
				if (type_id == typeid(_Type*).hash_code()) {
					return *static_cast<_Type**>(object);
				}
				else {
					return nullptr;
				}
			}

		};
	}
}

namespace any {
	class AbstractTypeFuctions {
	public:
		virtual void destroy(void* object) const = 0;
		virtual void* copy(void* object) const = 0;
		virtual AbstractTypeFuctions* self_copy() const = 0;
		virtual ~AbstractTypeFuctions() {}
	};

	template<class _Type>
	class TypeFuctions : public AbstractTypeFuctions
	{
	public:
		virtual void destroy(void* object) const override {
			delete static_cast<_Type*>(object);
		}
		virtual void* copy(void* object) const override {
			return static_cast<void*>(new _Type(*static_cast<_Type*>(object)));
		}
		virtual AbstractTypeFuctions* self_copy() const override {
			return new TypeFuctions<_Type>(*this);
		}
		virtual ~TypeFuctions() override {}
	};

	class Any {
	private:
		void* _object = nullptr;
		AbstractTypeFuctions* _type_functions = nullptr;
		bool   _has_value = false;
		size_t _type_id = 0;
	private:
		template<class _Type> friend _Type any_cast(Any& any);
	public:
		Any() {}

		template<class _Type>
		Any(_Type&& object) {
			_object = static_cast<void*>(new std::remove_reference_t<_Type>(std::forward<_Type>(object)));
			_type_functions = new TypeFuctions<std::remove_reference_t<_Type>>;
			_type_id = typeid(_Type).hash_code();
		}


		Any(const Any& other) {
			if (other._object) {
				_object = other._type_functions->copy(other._object);
				_type_functions = other._type_functions->self_copy();
				_type_id = other._type_id;
			}
		}

		Any(Any&& other) noexcept :
			_object(other._object),
			_type_functions(other._type_functions),
			_type_id(other._type_id)
		{
			other._object = nullptr;
			other._type_functions = nullptr;
			other._type_id = 0;
		}

		template<class _Type>
		Any& operator=(_Type&& object) {
			this->reset();

			_object = static_cast<void*>(new std::remove_reference_t<_Type>(std::forward<_Type>(object)));
			_type_functions = new TypeFuctions<std::remove_reference_t<_Type>>;
			_type_id = typeid(_Type).hash_code();
			_has_value = true;

			return *this;
		}

		Any& operator=(const Any& other) {
			if (this == &other) {
				return *this;
			}

			this->reset();

			if (other._object) {
				_object = other._type_functions->copy(other._object);
				_type_functions = other._type_functions->self_copy();
				_type_id = other._type_id;
			}

			return *this;
		}

		Any& operator=(Any&& other) noexcept {
			if (this == &other) {
				return *this;
			}

			this->reset();

			_object = other._object;
			_type_functions = other._type_functions;
			_type_id = other._type_id;

			other._object = nullptr;
			other._type_functions = nullptr;
			other._type_id = 0;

			return *this;
		}

		~Any() {
			this->reset();
		}

		void reset() {
			if (_object) {
				_type_functions->destroy(_object);
				delete _type_functions;
				_object = nullptr;
			}
		}

		bool has_value() const noexcept { return static_cast<bool>(_object); }

		template<class _Type>
		_Type as() {
			return any_cast<_Type>(*this);
		}
	};

	template<class _Type>
	_Type any_cast(Any& any) {
		if (any.has_value()) {
			return meta_functions::type_qualifier<_Type>::execute(any._type_id, any._object);
		}
		throw bad_any_cast("bad any cast");
	}
};
#endif