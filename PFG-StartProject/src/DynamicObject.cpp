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
		Verlet(deltaTs);
	}


	UpdateModelMatrix();

}

void DynamicObject::Euler(float deltaTs)
{
	float oneOverMass = 1 / _mass;

	_velocity += (_force * oneOverMass) * deltaTs;

	_position += _velocity * deltaTs;

}


void DynamicObject::Verlet(float deltaTs)
{
	glm::vec3 acceleration;

	acceleration = _force / _mass;

	_previousPosition = _position - _velocity * deltaTs + 0.5f * acceleration * deltaTs * deltaTs;

	_position = -_previousPosition + 2.0f * _position + acceleration * deltaTs * deltaTs;

	_velocity = (_position - _previousPosition) / (2.0f * deltaTs);

	_velocity += acceleration * deltaTs;
}



void DynamicObject::RungeKutta4(float deltaTs)
{
	glm::vec3 force;
	glm::vec3 acceleration;
	glm::vec3 k0;
	glm::vec3 k1;
	glm::vec3 k2;
	glm::vec3 k3;

	//Evaluate once at t0
	force = _force;
	acceleration = force / _mass;
	k0 = deltaTs * acceleration;

	//Evaluate twice at t0 + deltaT/2.0 using half of k0 and half of k1
	force = _force + k0 / 2.0f;
	acceleration = force / _mass;
	k1 = deltaTs * acceleration;

	force = _force + k1 / 2.0f;
	acceleration = force / _mass;
	k2 = deltaTs * acceleration;

	//Evaluate once at t0 + deltaT using k2
	force = _force + k2;
	acceleration = force / _mass;
	k3 = deltaTs * acceleration;

	//Evaluate at t0 + deltaT using weighted sum of k0, k1, k2 and k3
	_velocity += (k0 + 2.0f * k1 + 2.0f * k2 + k3) / 6.0f;
	//Update position 
	_position += _velocity * deltaTs;
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
	if (type == 0)
	{

		// Call moving sphere collision detection
		glm::vec3 n = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 c0 = _position;
		glm::vec3 c1 = _position + _velocity * deltaTs;
		glm::vec3 q = otherObject->GetPosition(); // This was getvelocity not getinitialvelocity
		glm::vec3 ci;

		// Using DistancetoPlane to detect collision
		bool collision = PFG::MovingSphereToPlaneCollision(n, c0, c1, q, r, ci);

		// Response to collision if there is one
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
	}

	// Sphere to sphere
	if (type == 1)
	{
		DynamicObject* otherDynamObj = dynamic_cast<DynamicObject*>(otherObject);

		glm::vec3 centre0 = otherDynamObj->GetPosition();
		glm::vec3 centre1 = _position;
		float radius1 = GetBoundingRadius();
		float radius2 = otherDynamObj->GetBoundingRadius();
		glm::vec3 collisionPoint;

		bool collision = PFG::SphereToSphereCollision(centre0, centre1, radius1, radius2, collisionPoint);

		if (collision)
		{
			std::cout << "A SPHERE HATH COLLIDETH WITH ANOTHER SPHERE";

			glm::vec3 ColliderVel = otherDynamObj->GetVelocity();
			glm::vec3 relativeVel = _velocity - ColliderVel;
			glm::vec3 normal = glm::normalize(centre0 - centre1);
			

			glm::vec3 contactPosition = radius1 * normal;
			float eCof = -(1.0f + elasticity) * glm::dot(relativeVel, normal);
			float invMass = 1 / GetMass();
			float invColliderMass = 1 / otherDynamObj->GetMass();
			float jLin = eCof / (invMass + invColliderMass);

			glm::vec3 collision_impulse_force = jLin * normal / deltaTs;

			glm::vec3 contact_force = glm::vec3(0.0f, 9.81f * _mass, 0.0f);
			glm::vec3 total_force = contact_force + collision_impulse_force;

			AddForce(total_force);
		}
	}
}


void DynamicObject::UpdateModelMatrix()
{
	_modelMatrix = glm::translate(glm::mat4(1), _position);
	_modelMatrix = glm::scale(_modelMatrix, _scale);
	_invModelMatrix = glm::inverse(_modelMatrix);
}