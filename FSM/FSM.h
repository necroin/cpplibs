#pragma once
#ifndef _FINITE_STATE_MACHINE_
#define _FINITE_STATE_MACHINE_
#include <list>
#include <functional>
#include <memory>
#include <type_traits>

namespace FSM::detail {
	template<class... _Args>
	struct pack {};

	template<class... _Args>
	struct get_functor_args;

	template<class _Return,class... _Args>
	struct get_functor_args<std::function<_Return(_Args...)>> {
		using args_pack = pack<_Args...>;
		using return_type = _Return;
	};

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

namespace FSM {
	template<class... _Input>
	class State;

	template<class... _Input>
	class AbstractTransition {
	public:
		virtual State<_Input...>* handle(_Input&... input) = 0;
	};


	template<class _FunctorHandler, class... _Input>
	class Transition :
		public AbstractTransition<_Input...>
	{
	private:
		_FunctorHandler _handler;
	public:
		Transition(const _FunctorHandler& handler) : _handler(handler) {
			static_assert(detail::args_count_v<_Input...> == detail::args_count_v<typename detail::get_functor_args<decltype(std::function(handler))>::args_pack>,
			"The number of arguments in transition is not equal to the number of arguments of the state machine");
		}
		Transition(_FunctorHandler&& handler) : _handler(handler) {
			static_assert(detail::args_count_v<_Input...> == detail::args_count_v<typename detail::get_functor_args<decltype(std::function(handler))>::args_pack>,
			"The number of arguments in transition is not equal to the number of arguments of the state machine");
		}
		State<_Input...>* handle(_Input&... input) override {
			return _handler(input...);
		}
	};

	using StateFunctorExecuter = std::function<void()>;

	template<class... _Input>
	class State {
	private:
		StateFunctorExecuter _executer;
		std::list<std::unique_ptr<AbstractTransition<_Input...>>> _transitions;
	public:
		State(const StateFunctorExecuter& executer) : _executer(executer) {}
		State(StateFunctorExecuter&& executer) : _executer(executer) {}

		State* handle(_Input&... input) {
			for (auto&& transition : _transitions) {
				State* new_state = transition->handle(input...);
				if (new_state) return new_state;
			}
			return this;
		}

		void execute() {
			_executer();
		}

		template<class FunctorHandler>
		void add_transition(FunctorHandler&& handler){
			_transitions.emplace_back(std::move(std::make_unique<Transition<std::remove_reference_t<FunctorHandler>, _Input...>>(std::forward<FunctorHandler>(handler))));
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
			static_assert(detail::args_count_v<_Input...> == detail::args_count_v<Input...>,
			"The number of arguments submitted to the handling function is not equal to the number of arguments of the state machine");
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
