#pragma once
#ifndef _EVENTSYSTEM_H_
#define _EVENTSYSTEM_H_
#include <list>
#include <memory>
#include <functional>

namespace EventSystem {
	namespace detail {
		template<class... _Args>
		struct pack {};

		template<class... _Args>
		struct get_function_args;

		template<class... _Args>
		struct get_function_args<void(_Args...)> {
			using args_pack = pack<_Args...>;
		};

		template<class... _Args>
		struct get_function_args<std::function<void(_Args...)>> {
			using args_pack = pack<_Args...>;
		};

		template<class... Args>
		auto to_function(std::function<void(Args...)>)->std::function<void(Args...)>;
	}

	template<class... _Args>
	class AbstractEventHandler {
	public:
		virtual void call(_Args... args) = 0;
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

	namespace detail {
		template<class... _Args>
		using AbstractEventHandlerPointer = std::shared_ptr<AbstractEventHandler<_Args...>>;


		template<class... Unused>
		class FunctorEventHandler;

		template<class _FunctorHandler, class... _Args>
		class FunctorEventHandler<_FunctorHandler, detail::pack<_Args...>> :
			public AbstractEventHandler<_Args...>
		{
		private:
			_FunctorHandler _functor_handler;
		protected:
			virtual bool is_equals(const AbstractEventHandler<_Args...>& other) const override {
				return this == &other;
			}
		public:
			FunctorEventHandler(const _FunctorHandler& functor_handler) : _functor_handler(functor_handler) {}
			FunctorEventHandler(_FunctorHandler&& functor_handler) : _functor_handler(functor_handler) {}
		public:
			virtual void call(_Args... args) override {
				_handle_function(args...);
			}
		};
	}


	template<class _FunctorHandler>
	decltype(auto) createFunctorEventHandler(_FunctorHandler&& functor_handler) {
		return std::make_shared<detail::FunctorEventHandler<_FunctorHandler, typename detail::get_function_args<decltype(detail::to_function(std::function(functor_handler)))>::args_pack>>(std::forward<_FunctorHandler>(functor_handler));
	}


	template<class... _Args>
	class IEvent {
	private:
		using EventHandler = AbstractEventHandler<_Args...>;
		using EventHandlerPointer = std::shared_ptr<EventHandler>;
	protected:
		IEvent() {}
		virtual bool add_handler(EventHandlerPointer& event_handler_pointer) = 0;
		virtual bool remove_handler(EventHandlerPointer& event_handler) = 0;
	public:
		bool operator+=(EventHandlerPointer& event_handler_pointer) {
			return add_handler(event_handler_pointer);
		}
		bool operator-=(EventHandlerPointer& event_handler) {
			return remove_handler(event_handler);
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
		virtual bool remove_handler(EventHandlerPointer& event_handler) override {
			decltype(auto) handler_it = find_handler(*event_handler);
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