#include "ECS.h"

namespace ECS {
	ComponentId Get_component_id() {
		static ComponentId id = 0;
		return id++;
	}

	Component::~Component() {}

	Entity& Component::entity() const { return *_entity; }

	void Component::action() {}

	void Component::update() {}

	Entity::Entity() : _components(0) {}

	Entity::Entity(const Entity& other)
	{
		_components.reserve(other._components.size());
		for (size_t i = 0; i < _components.size(); ++i) {
			if (static_cast<bool>(other._components[i])) {
				_components.emplace_back(other._components[i]->copy());
				_components[i]->_entity = this;
			}
			else {
				_components.emplace_back();
			}
		}
	}

	Entity::Entity(Entity&& other) noexcept
	{
		_components.reserve(other._components.size());
		for (size_t i = 0; i < _components.size(); ++i) {
			if (static_cast<bool>(other._components[i])) {
				_components.emplace_back(std::move(other._components[i]));
				_components[i]->_entity = this;
			}
			else {
				_components.emplace_back();
			}
		}
	}

	Entity& Entity::operator=(const Entity& other)
	{
		if (this == &other) {
			return *this;
		}

		_components.clear();
		_components.reserve(other._components.size());
		for (size_t i = 0; i < _components.size(); ++i) {
			if (static_cast<bool>(other._components[i])) {
				_components.emplace_back(other._components[i]->copy());
				_components[i]->_entity = this;
			}
			else {
				_components.emplace_back();
			}
		}

		return *this;
	}

	Entity& Entity::operator=(Entity&& other) noexcept
	{
		if (this == &other) {
			return *this;
		}

		_components.clear();
		_components.reserve(other._components.size());
		for (size_t i = 0; i < _components.size(); ++i) {
			if (static_cast<bool>(other._components[i])) {
				_components.emplace_back(std::move(other._components[i]));
				_components[i]->_entity = this;
			}
			else {
				_components.emplace_back();
			}
		}

		return *this;
	}

	void Entity::action()
	{
		for (auto&& component : _components) {
			component->action();
		}
	}

	void Entity::update()
	{
		for (auto&& component : _components) {
			component->update();
		}
	}
}