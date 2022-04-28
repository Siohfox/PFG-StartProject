#include "DynamicObject.h"
#include <GLM/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>


DynamicObject::DynamicObject()
{
	_force = glm::vec3(0.0f, 0.0f, 0.0f);
	_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	_bRadius = 0.0f;
	_mass = 1.0f;

	_scale = glm::vec3(1.0f, 1.0f, 1.0f);
	_start = false;
}

DynamicObject::~DynamicObject()
{

}

void DynamicObject::Update(float deltaTs)
{
	float collision_impulse;
	glm::vec3 floor_normal(0.0f, 1.0f, 0.0f);
	float elasticity = 0.5;
	glm::vec3 impulse_force;
	glm::vec3 contact_force(0.0f, _mass * 9.8f * 0.1f, 0.0f);
	float r = GetBoundingRadius();

	if (_start == true)
	{

		// Step 1 clear forces
		ClearForces();

		// Step 2 Compute forces
		glm::vec3 gravityForce = glm::vec3(0.0f, -9.8 * _mass * 0.1f, 0.0f);
		AddForce(gravityForce);

		// Step 3 Compute collisions
		if (_position.y <= r)
		{
			_position.y = _bRadius;

			AddForce(contact_force);
			collision_impulse = (-(1 + elasticity) * glm::dot(_velocity, floor_normal)) / (1 / _mass);
			impulse_force = (collision_impulse * floor_normal) / deltaTs;

			AddForce(impulse_force);
		}

		// Step 4 Euler integration
		Euler(deltaTs);
	}


	UpdateModelMatrix();

}

void DynamicObject::Euler(float deltaTs)
{
	float oneOverMass = 1 / _mass;

	_velocity += (_force * oneOverMass) * deltaTs;

	_position += _velocity * deltaTs;

}

void DynamicObject::UpdateModelMatrix()
{
	_modelMatrix = glm::translate(glm::mat4(1), _position);
	_modelMatrix = glm::scale(_modelMatrix, _scale);
	_invModelMatrix = glm::inverse(_modelMatrix);
}