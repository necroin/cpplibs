#pragma once
#ifndef _FINITE_STATE_MACHINE_
#define _FINITE_STATE_MACHINE_
#include <list>
#include <memory>
#include <functional>
#include <iostream>
#include <type_traits>

namespace FSM {
	namespace detail {
		template<class... _Args>
		struct pack {};

		template<class... _Args>
		struct get_functor_args;

		template<class _Object, class _Return, class... _Args>
		struct get_functor_args<_Return(_Object::*)(_Args...) const> { using args_pack = pack<_Args...>; };

		template<class... _Args>
		struct args_count {
			static constexpr size_t value = sizeof...(_Args);
		};

		template<class... _Args>
		struct args_count<pack<_Args...>>
		{
			static constexpr size_t value = sizeof...(_Args);
		};

		template<class... _Args>
		inline constexpr size_t args_count_v = args_count<_Args...>::value;
	}
}

namespace FSM {
	template<class... _Input>
	class State;

	namespace detail {
		template<class... _Input>
		class AbstractTransition {
		public:
			virtual State<_Input...>* handle(_Input&... input) = 0;
		};

		template<class _FunctorHandler, class... _Input>
		class Transition :
			public AbstractTransition<_Input...>
		{
			_FunctorHandler _handler;
		public:
			Transition(const _FunctorHandler& handler) : _handler(handler) {
				static_assert(args_count_v<_Input...> == args_count_v<typename get_functor_args<decltype(&_FunctorHandler::operator())>::args_pack>,
					"The number of arguments is not equal");
			}
			Transition(_FunctorHandler&& handler) : _handler(handler) {
				static_assert(args_count_v<_Input...> == args_count_v<typename get_functor_args<decltype(&_FunctorHandler::operator())>::args_pack>,
					"The number of arguments is not equal");
			}
			State<_Input...>* handle(_Input&... input) override {
				return _handler(input...);
			}
		};

		class AbstractAction {
		public:
			virtual void execute() = 0;
		};

		template<class _Functor>
		class Action : public AbstractAction {
		private:
			_Functor _functor;
		public:
			Action(_Functor& functor) : _functor(functor) {}
			Action(_Functor&& functor) : _functor(functor) {}
			virtual void execute() override {
				_functor();
			}
		};
	}

	using StateFunctorExecuter = std::function<void()>;

	template<class... _Input>
	class State {
	private:
		StateFunctorExecuter _executer;
		std::unique_ptr<detail::AbstractAction> _entry_action;
		std::unique_ptr<detail::AbstractAction> _exit_action;
		std::list<std::unique_ptr<detail::AbstractTransition<_Input...>>> _transitions;
	public:
		State(const StateFunctorExecuter& executer) : _executer(executer) {}
		State(const StateFunctorExecuter&& executer) : _executer(executer) {}

		State* handle(_Input&... input) {
			for (auto&& transition : _transitions) {
				State* new_state = transition->handle(input...);
				if (new_state) {
					this->exit();
					new_state->entry();
					return new_state;
				}
			}
			return this;
		}

		void execute() {
			_executer();
		}

		template<class FunctorHandler>
		void add_transition(FunctorHandler&& handler){
			_transitions.emplace_back(std::move(std::make_unique<detail::Transition<FunctorHandler, _Input...>>(std::forward<FunctorHandler>(handler))));
		}

		template<class Functor>
		void set_entry(Functor&& functor) {
			_entry_action = std::make_unique<detail::Action<Functor>>(functor);
		}

		template<class Functor>
		void set_exit(Functor&& functor) {
			_exit_action = std::make_unique<detail::Action<Functor>>(functor);
		}

		void entry() {
			if(_entry_action) _entry_action->execute();
		}

		void exit() {
			if(_exit_action) _exit_action->execute();
		}
	};

	template<class... _Input>
	class FSM {
	private:
		using __State = State<_Input...>;
		std::list<__State> _states;
		__State* _current_state = nullptr;
	public:
		__State& add_state(const StateFunctorExecuter& executer) {
			_states.emplace_back(executer);
			return _states.back();
		}

		__State& add_state(const StateFunctorExecuter&& executer) {
			_states.emplace_back(executer);
			return _states.back();
		}

		template<class... Input>
		void handle(Input&&... input) {
			static_assert(detail::args_count_v<_Input...> == detail::args_count_v<Input...>, "The number of arguments is not equal");
			if (_current_state) _current_state = _current_state->handle(input...);
			else { throw std::exception("Undefined current state"); }
		}

		void execute() {
			if (_current_state) _current_state->execute();
			else { throw std::exception("Undefined current state"); }
		}

		void set_current_state(__State& new_current_state) {
			_current_state = &new_current_state;
		}
	};
}
#endif
