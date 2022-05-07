#include "DynamicObject.h"
#include <GLM/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>
#include "Utility.h"

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

void DynamicObject::Update(GameObject* otherObject, float deltaTs)
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

		// Step 3
		CollisionResponse(otherObject, deltaTs);

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

void DynamicObject::Midpoint(float deltaTs)
{
	glm::vec3 force;
	glm::vec3 accel;
	glm::vec3 k0;
	glm::vec3 k1;

	// Evaluate once at t0
	force = _force;
	accel = force / _mass;
	k0 = deltaTs * accel;

	// Evaluate once at t0 + deltaT/2 using half of k0
	force = _force + k0 / 2.0f;
	accel = force / _mass;
	k1 = deltaTs * accel;

	// Eva;uate once at t0 + deltaT using k1
	_velocity += k1;
	_position += _velocity * deltaTs;
}


void DynamicObject::Verlet(float deltaTs)
{


	//_position + deltaTs += -_position * deltaTs - deltaTs + 2 * _position * deltaTs + _force/_mass * deltaTs;
}

void DynamicObject::CollisionResponse(GameObject* otherObject, float deltaTs)
{
	// Stuff I had before
	float collision_impulse;
	glm::vec3 floor_normal(0.0f, 1.0f, 0.0f);	
	glm::vec3 impulse_force;
	glm::vec3 contact_force(0.0f, _mass * 9.8f * 0.1f, 0.0f);

	// New stuff
	const float r = GetBoundingRadius();
	float elasticity = 0.8;
	int type = otherObject->GetType();



	// Sphere to plane
	if (type == 1)
	{

		// Call moving sphere collision detection
		glm::vec3 n = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 c0 = _position;
		glm::vec3 c1 = _position + _velocity * deltaTs;
		glm::vec3 q = otherObject->GetInitialVelocity(); // This was getvelocity not getinitialvelocity
		glm::vec3 ci;

		// Using DistancetoPlane to detect collision
		bool collision = PFG::MovingSphereToPlaneCollision(n, c0, c1, q, r, ci);

		if (collision)
		{
			glm::vec3 ColliderVel = otherObject->GetInitialVelocity(); // This was getvelocity not getinitialvelocity
			glm::vec3 relativeVel = _velocity - ColliderVel;
			glm::vec3 n = glm::vec3(0.0f, 1.0f, 0.0f); // floor normal up
			float invColliderMass = 0.0f; // floor doesn't move

			glm::vec3 contactPosition = ci;
			float eCof = -(1.0f + elasticity) * glm::dot(relativeVel, n);
			float invMass = 1 / GetMass();
			float jLin = eCof / (invMass + invColliderMass);

			glm::vec3 collision_impulse_force = jLin * n / deltaTs;

			glm::vec3 contact_force = glm::vec3(0.0f, 9.81f * _mass, 0.0f);
			glm::vec3 total_force = contact_force + collision_impulse_force;

			AddForce(total_force);
		}

		/*if (_position.y <= r)
		{
			_position.y = _bRadius;

			AddForce(contact_force);
			collision_impulse = (-(1 + elasticity) * glm::dot(_velocity, floor_normal)) / (1 / _mass);
			impulse_force = (collision_impulse * floor_normal) / deltaTs;

			AddForce(impulse_force);
		}*/
	}

	// Sphere to sphere
	else if (otherObject->GetType() == 0)
	{

		glm::vec3 n = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 c0 = _position;
		glm::vec3 c1 = _position + _velocity * deltaTs;
		glm::vec3 q = otherObject->GetInitialVelocity(); // This was getvelocity not getinitialvelocity
		glm::vec3 ci;

		//if (PFG::SphereToSphereCollision(c0, c1, q, r, ci))
		//{
		//	
		//}
		

		// sphere to sphere
	}

	
}


void DynamicObject::UpdateModelMatrix()
{
	_modelMatrix = glm::translate(glm::mat4(1), _position);
	_modelMatrix = glm::scale(_modelMatrix, _scale);
	_invModelMatrix = glm::inverse(_modelMatrix);
}