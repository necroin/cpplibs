#pragma once
#ifndef _EVENTSYSTEM_H_
#define _EVENTSYSTEM_H_
#include <list>
#include <functional>
#include <memory>
#include <type_traits>

namespace EventSystem {
	namespace detail {
		namespace strip {
			template<class _Functor>
			struct __strip_signature;

			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...)> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) const> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) volatile> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) const volatile> { using type = _Result(_Object::*)(_Args...); };

			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...)&> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) const&> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) volatile&> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) const volatile&> { using type = _Result(_Object::*)(_Args...); };

			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...)&&> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) const&&> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) volatile&&> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) const volatile&&> { using type = _Result(_Object::*)(_Args...); };

#if _HAS_CXX17
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) noexcept> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) const noexcept> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) volatile noexcept> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) const volatile noexcept> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) & noexcept> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) const& noexcept> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) volatile& noexcept> { using type = _Result(_Object::*)(_Args...); };
			template<class _Result, class _Object, class ..._Args>
			struct __strip_signature<_Result(_Object::*) (_Args...) const volatile& noexcept> { using type = _Result(_Object::*)(_Args...); };
#endif

			template<class _Functor>
			using __strip_signature_t = typename __strip_signature<_Functor>::type;
		}
	}
}

namespace EventSystem {
	namespace detail {
		template<class... _Args>
		struct pack {};

		template<class... _Args>
		struct get_function_args;

		template<class _Return, class _Object, class... _Args>
		struct get_function_args<_Return(_Object::*)(_Args...)> {
			using args_pack = pack<_Args...>;
			using return_type = _Return;
		};

		template<class _Return, class... _Args>
		struct get_function_args<std::function<_Return(_Args...)>> {
			using args_pack = pack<_Args...>;
			using return_type = _Return;
		};
	}

	template<class... _Args>
	class AbstractEventHandler {
	public:
		virtual void call(_Args&... args) = 0;
		virtual ~AbstractEventHandler() {}
		bool operator==(const AbstractEventHandler& other) const {
			return is_equals(other);
		}
		bool operator!=(const AbstractEventHandler& other) const {
			return !is_equals(other);
		}
	protected:
		AbstractEventHandler() {}
		virtual bool is_equals(const AbstractEventHandler& other) const = 0;
	};

	namespace handlers {
		template<class... _Unused>
		class FunctorEventHandler;

		template<class _FunctorHandler, class... _Args>
		class FunctorEventHandler<_FunctorHandler, detail::pack<_Args...>> :
			public AbstractEventHandler<_Args...>
		{
		private:
			_FunctorHandler _functor_handler;
		protected:
			virtual bool is_equals(const AbstractEventHandler<_Args...>& other) const override final {
				return this == &other;
			}
		public:
			FunctorEventHandler(const _FunctorHandler& functor_handler) : _functor_handler(functor_handler) {}
			FunctorEventHandler(_FunctorHandler&& functor_handler) : _functor_handler(functor_handler) {}
		public:
			virtual void call(_Args&... args) override final {
				_functor_handler(args...);
			}
		};

		template<class... _Unused>
		class MethodEventHandler;

		template<class _Method, class _Object, class _Return, class... _Args>
		class MethodEventHandler<_Method, _Return(_Object::*)(_Args...)> :
			public AbstractEventHandler<_Args...>
		{
		private:
			_Object& _object;
			_Method _method;
		protected:
			virtual bool is_equals(const AbstractEventHandler<_Args...>& other) const override final {
				decltype(this) other_ptr = dynamic_cast<decltype(this)>(&other);
				return  other_ptr != nullptr && &_object == &other_ptr->_object && _method == other_ptr->_method;
			}
		public:
			MethodEventHandler(_Object& object, _Method method) : _object(object), _method(method) {
				if (_method == nullptr) throw std::exception("Undefined method");
			}

			virtual void call(_Args&... args) override final {
				(_object.*_method)(args...);
			}
		};
	}

	template<class _FunctorHandler>
	decltype(auto) createFunctorEventHandler(_FunctorHandler&& functor_handler) {
#if _HAS_CXX17
		return std::make_shared<handlers::FunctorEventHandler<std::remove_reference_t<_FunctorHandler>, typename detail::get_function_args<decltype(std::function(functor_handler))>::args_pack>>(std::forward<_FunctorHandler>(functor_handler));
#else
		return std::make_shared<handlers::FunctorEventHandler<std::remove_reference_t<_FunctorHandler>, typename detail::get_function_args<detail::strip::__strip_signature_t<decltype(&_FunctorHandler::operator())>>::args_pack>>(std::forward<_FunctorHandler>(functor_handler));
#endif
	}

	template<class _Object, class _Method>
	decltype(auto) createMethodEventHandler(_Object& object ,_Method&& method) {
		return std::make_shared<handlers::MethodEventHandler<_Method, detail::strip::__strip_signature_t<_Method>>>(object, method);
	}

	template<class... _Args>
	class IEvent {
	private:
		using EventHandler = AbstractEventHandler<_Args...>;
		using EventHandlerPointer = std::shared_ptr<EventHandler>;
	protected:
		IEvent() {}
		virtual bool add_handler(EventHandlerPointer& event_handler_pointer) = 0;
		virtual bool remove_handler(EventHandlerPointer& event_handler_pointer) = 0;
	public:
		bool operator+=(EventHandlerPointer event_handler_pointer) {
			return add_handler(event_handler_pointer);
		}
		bool operator-=(EventHandlerPointer event_handler_pointer) {
			return remove_handler(event_handler_pointer);
		}
	};

	template <class... _Args>
	class Event :
		public IEvent<_Args...>
	{
	private:
		using EventHandler = AbstractEventHandler<_Args...>;
		using EventHandlerPointer = std::shared_ptr<EventHandler>;
	private:
		std::list<EventHandlerPointer> _handlers;
	private:
		decltype(auto) find_handler(EventHandler& event_handler) {
			return std::find_if(_handlers.cbegin(), _handlers.cend(), [&event_handler](const EventHandlerPointer& oneHandler)
				{
					return (*oneHandler == event_handler);
				});
		}
	protected:
		virtual bool add_handler(EventHandlerPointer& event_handler_pointer) override {
			if (find_handler(*event_handler_pointer) == _handlers.end()) {
				_handlers.emplace_back(event_handler_pointer);
				return true;
			}
			return false;
		}
		virtual bool remove_handler(EventHandlerPointer& event_handler_pointer) override {
			decltype(auto) handler_it = find_handler(*event_handler_pointer);
			if (handler_it != _handlers.end()) {
				_handlers.erase(handler_it);
				return true;
			}
			return false;
		}
	public:
		void operator()(_Args... args) {
			for (auto&& handler : _handlers) {
				handler->call(args...);
			}
		}
	};
}
#endif