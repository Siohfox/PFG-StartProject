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

	_torque = glm::vec3(0.0f, 0.0f, 0.0f);
	_angular_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	_angular_momentum = glm::vec3(0.0f, 0.0f, 0.0f);

	_rotationMatrix = glm::mat3(1.0f);

	_rotQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	_scale = glm::vec3(1.0f, 1.0f, 1.0f);
	_start = false;
	_stopped = false;
}

DynamicObject::~DynamicObject()
{

}

void DynamicObject::StartSimulation(bool start)
{
	_start = start;

	_rotQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

	glm::mat3 body_inertia;

	body_inertia = glm::mat3
	{
		(2.0f / 5.0f) * _mass * std::pow(_bRadius, 2),0,0,
		0, (2.0f / 5.0f) * _mass * std::pow(_bRadius, 2),0,
		0,0, (2.0f / 5.0f) * _mass * std::pow(_bRadius, 2)
	};

	_body_inertia_tensor_inverse = glm::inverse(body_inertia);

	ComputeInverseInertiaTensor();

	_angular_velocity = _inertia_tensor_inverse * _angular_momentum;
}

void DynamicObject::ComputeInverseInertiaTensor()
{
	_inertia_tensor_inverse = _rotationMatrix * _body_inertia_tensor_inverse * glm::transpose(_rotationMatrix);
}

void DynamicObject::Update(GameObject* otherObject, float deltaTs)
{

	if (_start)
	{
		// STEP 1: clear forces
		ClearForces();
		ClearTorque();

		// STEP 2: Compute forces
		glm::vec3 gravityForce = glm::vec3(0.0f, -9.8 * _mass * 0.1f, 0.0f);
		AddForce(gravityForce);

		// STEP 3: Find collision response
		CollisionResponse(otherObject, deltaTs);

		// STEP 4: Calculate next position
		RungeKutta4(deltaTs);
	}

	UpdateModelMatrix();

}

void DynamicObject::Euler(float deltaTs)
{
	float oneOverMass = 1 / _mass;

	_velocity += (_force * oneOverMass) * deltaTs; // Velocity is calced based on previous velocity

	_position += _velocity * deltaTs; // Current position is calced based on previous position

	if (_stopped == true)
	{
		_velocity.x = 0.0f;
		_velocity.z = 0.0f;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ROTATION PHYSICS
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// STEP 1: Compute current angular momentum
	_angular_momentum += _torque * deltaTs;

	// STEP 2: Computer inverse inertia tensor
	ComputeInverseInertiaTensor();

	// STEP 3: Update angular velocity
	_angular_velocity = _inertia_tensor_inverse * _angular_momentum;

	// STEP 4: Compute skew matrix omega star
	glm::mat3 omega_star = glm::mat3(0.0f, -_angular_velocity.z, _angular_velocity.y,
									_angular_velocity.z, 0.0f, -_angular_velocity.x,
									-_angular_velocity.y, _angular_velocity.x, 0.0f);

	// Update rotation matrix
	_rotationMatrix += omega_star * _rotationMatrix * deltaTs;
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

void DynamicObject::RungeKutta2(float deltaTs)
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

	// Evaluate once at t0 + deltaT using k1
	_velocity += k1;
	_position += _velocity * deltaTs;
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

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ROTATION PHYSICS
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// STEP 1: Compute current angular momentum
	glm::vec3 tempTorque;

	tempTorque = _torque;
	k0 = deltaTs * tempTorque;

	tempTorque = _torque + k0 / 2.0f;
	k1 = deltaTs * tempTorque;

	tempTorque = _torque + k1 / 2.0f;
	k2 = deltaTs * tempTorque;

	tempTorque = _torque + k2;
	k3 = deltaTs * tempTorque;

	_angular_momentum += (k0 + 2.0f * k1 + 2.0f, + k2 + k3) / 6.0f;

	if (_stopped == true)
	{
		_angular_momentum = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	// STEP 2: Computer inverse inertia tensor
	ComputeInverseInertiaTensor();

	// STEP 3: Update angular velocity
	_angular_velocity = _inertia_tensor_inverse * _angular_momentum;

	// STEP 4: Compute skew matrix omega star
	glm::mat3 omega_star = glm::mat3(0.0f, -_angular_velocity.z, _angular_velocity.y,
		_angular_velocity.z, 0.0f, -_angular_velocity.x,
		-_angular_velocity.y, _angular_velocity.x, 0.0f);

	// Update rotation matrix
	_rotationMatrix += omega_star * _rotationMatrix * deltaTs;

}

void DynamicObject::CollisionResponse(GameObject* otherObject, float deltaTs)
{
	// New stuff
	const float r = GetBoundingRadius();
	const float elasticity = 0.5;
	int type = otherObject->GetType();
	_stopped = false;

	// Sphere to plane
	if (type == 0)
	{
		// Call moving sphere collision detection
		glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 centre0 = _position;
		glm::vec3 centre1 = _position + _velocity * deltaTs;
		glm::vec3 q = otherObject->GetPosition();
		glm::vec3 contactPoint;

		// Using DistancetoPlane to detect collision
		bool collision = PFG::MovingSphereToPlaneCollision(normal, centre0, centre1, q, r, contactPoint);

		// Response to collision if there is one
		if (collision)
		{
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// COLLISION RESPONSE
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			glm::vec3 ColliderVel = otherObject->GetInitialVelocity();
			glm::vec3 relativeVel = _velocity - ColliderVel;
			glm::vec3 n = glm::vec3(0.0f, 1.0f, 0.0f); // floor normal up
			float invColliderMass = 0.0f; // floor doesn't move

			_position = contactPoint;

			// Zero out the 
			float Jlinear = 0.0f;
			float Jangular = 0.0f;
			float elasticity = 0.5f;
			glm::vec3 r1 = _bRadius * glm::vec3(0.0f, 1.0f, 0.0f); // Lever between the COM and point of contact

			float oneOverMass1 = 1.0f / _mass; // 1/m of object 1
			float oneOverMass2 = 0.0f;		   // 1/m of object 2
			glm::vec3 vA = _velocity;		   // Velocity of object 1
			glm::vec3 vB = glm::vec3(0.0f, 0.0f, 0.0f);
			glm::vec3 relativeVelocity = vA - vB;
			glm::vec3 contactNormal = glm::vec3(0.0f, 1.0f, 0.0f);

			float eCof = -(1.0f + elasticity) * glm::dot(relativeVel, contactNormal);
			// Jlin = (-(1 + e)*va dot cN) / (1 / m1)+ (1 / m2)
			Jlinear = eCof / oneOverMass1 + oneOverMass2;
			// Jang = (-(1 + e)*va dot cN) / (1 / m1) + (1 / m2) + (I * r1 * cN) dot cN
			Jangular = eCof / (oneOverMass1 + oneOverMass2 + glm::dot(_inertia_tensor_inverse * (r1 * contactNormal), contactNormal));

			glm::vec3 impulseForce = (Jangular + Jlinear) * contactNormal; // Fi = (Jang + Jlin) * cN
			glm::vec3 contactForce = -(_force) * contactNormal;

			AddForce(impulseForce + contactForce);
			_velocity += (impulseForce / _mass); // Adding the impulse onto the velocity

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// FRICTION
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			glm::vec3 forwardRelativeVelocity = relativeVelocity - glm::dot(relativeVelocity, contactNormal) * contactNormal; // Finding relative velocity perpendicular to the contact normal

			glm::vec3 forwardRelativeDirection = glm::vec3(0.0f, 0.0f, 0.0f);
			if (forwardRelativeVelocity != glm::vec3(0.0f, 0.0f, 0.0f))
			{
				forwardRelativeDirection = glm::normalize(forwardRelativeVelocity); // gets a normalized vector of the direction travelled perpendicular to the contact normal
			}

			float mu = 0.5f;
			glm::vec3 frictionDirection = forwardRelativeDirection * -1.0f; // friction direction acts in opposite direction of direction travel
			glm::vec3 frictonForce = frictionDirection * mu * glm::length(contactForce);

			if (glm::length(forwardRelativeVelocity) - ((glm::length(frictonForce) / _mass) * deltaTs) > 0.0f) // Checks to see if friction force would reverse the direction of travel
			{
				AddForce(frictonForce); // Add friction
			}
			else
			{
				frictonForce = forwardRelativeVelocity * -1.0f; // Adds enough friction to stop the object
				AddForce(frictonForce);
				_stopped = true;
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// TORQUE
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			glm::vec3 tempTorque = (glm::cross(r1, contactForce)) + (glm::cross(r1, frictonForce)); // Computes torque

			tempTorque.x -= _angular_momentum.x * 20.0f;
			tempTorque -= _angular_momentum.z * 20.0f; // A damper to slow rotation over time

			AddTorque(tempTorque);
		}
	}

	// Sphere to sphere
	if (type == 1)
	{
		// Cast gameobject into dynamic so we can use sphere to sphere functions
		DynamicObject* otherDynamObj = dynamic_cast<DynamicObject*>(otherObject);

		glm::vec3 centre0 = otherDynamObj->GetPosition();
		glm::vec3 centre1 = _position;
		float radius1 = GetBoundingRadius();
		float radius2 = otherDynamObj->GetBoundingRadius();
		glm::vec3 collisionPoint;

		bool collision = PFG::SphereToSphereCollision(centre0, centre1, radius1, radius2, collisionPoint);

		if (collision)
		{
			//std::cout << "A SPHERE HATH COLLIDETH WITH ANOTHER SPHERE";

			glm::vec3 ColliderVel = otherDynamObj->GetVelocity();
			glm::vec3 relativeVel = _velocity - ColliderVel;
			glm::vec3 normal = glm::normalize(centre0 - centre1);
			

			glm::vec3 contactPosition = radius1 * normal;
			float eCof = -(1.0f + elasticity) * glm::dot(relativeVel, normal);
			float invMass = 1 / GetMass();
			float invColliderMass = 1 / otherDynamObj->GetMass();
			float jLin = eCof / (invMass + invColliderMass);

			glm::vec3 collision_impulse_force = jLin * normal / deltaTs;

			glm::vec3 acceleration = _force / _mass + otherDynamObj->GetMass();

			glm::vec3 contact_force = _force - _mass * acceleration;
			glm::vec3 total_force = contact_force + collision_impulse_force;

			AddForce(total_force);
		}
	}
}


void DynamicObject::UpdateModelMatrix()
{
	glm::mat4 modelRotation = glm::mat4(_rotationMatrix);

	glm::quat rotation = glm::normalize(glm::quat_cast(modelRotation));

	_rotationMatrix = glm::mat3_cast(rotation);

	_modelMatrix = glm::translate(glm::mat4(1), _position);
	_modelMatrix = glm::scale(_modelMatrix, _scale);
	_modelMatrix = _modelMatrix * glm::mat4_cast(rotation);
	_invModelMatrix = glm::inverse(_modelMatrix);

}