#pragma once
#ifndef _ECS_H_
#define _ECS_H_
#include <vector>
#include <memory>
#include <type_traits>

namespace ECS {
	class Entity;
	class Component;

	using ComponentId = std::size_t;

	ComponentId Get_component_id();

	template<class _Ty>
	ComponentId Get_component_id() {
		static ComponentId id = Get_component_id();
		return id;
	}

	using ComponentsArray = std::vector<std::unique_ptr<Component>>;

	class Component {
	private:
		friend class Entity;
	protected:
		Entity* _entity;
	public:
		virtual ~Component();

		Entity& entity() const;

		virtual void action();
		virtual void update();

		virtual Component* copy() const = 0;
	};

	class Entity {
	protected:
		ComponentsArray _components;
	public:
		Entity() = default;
		Entity(const Entity& other);
		Entity(Entity&& other) noexcept;

		Entity& operator=(const Entity& other);
		Entity& operator=(Entity&& other) noexcept;
	public:
		template<class _Component, class... Args>
		_Component& add_component(Args&&...args);

		template<class _Component>
		bool has_component() const noexcept;

		template<class _Component>
		_Component& get_component() const;

		virtual void action();
		virtual void update();
	};
#endif

	template<class _Component, class ...Args>
	inline _Component& Entity::add_component(Args&& ...args)
	{
		ComponentId component_id = Get_component_id<_Component>();
		if (component_id > _components.size() - 1) {
			_components.resize(component_id + 1);
		}
		decltype(auto) component = std::make_unique<_Component>(std::forward<Args>(args)...);
		component->_entity = this;
		_components[component_id] = std::move(component);
		return get_component<_Component>();
	}

	template<class _Component>
	inline bool Entity::has_component() const noexcept
	{
		ComponentId component_id = Get_component_id<_Component>();
		if (component_id > _components.size() - 1) {
			return false;
		}
		return static_cast<bool>(_components[component_id]);
	}

	template<class _Component>
	inline _Component& Entity::get_component() const
	{
		ComponentId component_id = Get_component_id<_Component>();
		if (component_id > _components.size() - 1) {
			throw std::exception("non-contained component");
		}
		return static_cast<_Component&>(*_components[component_id]);
	}
}