#include "Scene.h"


/*! \brief Brief description.
*  Scene class is a container for loading all the game objects in your simulation or your game.
*
*/
Scene::Scene()
{
	// Set up your scene here......
	// Set a camera
	_camera = new Camera();
	// Don't start simulation yet
	_simulation_start = false;

	// Position of the light, in world-space
	_lightPosition = glm::vec3(10, 10, 0);

	// Create a game object
	_physics_object = new GameObject();
	_physics_object2 = new GameObject();
	// Create a game level object
	_level = new GameObject();

	// Create the material for the game object- level
	Material *modelMaterial = new Material();
	// Shaders are now in files
	modelMaterial->LoadShaders("assets/shaders/VertShader.txt", "assets/shaders/FragShader.txt");
	// You can set some simple material properties, these values are passed to the shader
	// This colour modulates the texture colour
	modelMaterial->SetDiffuseColour(glm::vec3(0.8, 0.8, 0.8));
	// The material currently supports one texture
	// This is multiplied by all the light components (ambient, diffuse, specular)
	// Note that the diffuse colour set with the line above will be multiplied by the texture colour
	// If you want just the texture colour, use modelMaterial->SetDiffuseColour( glm::vec3(1,1,1) );
	modelMaterial->SetTexture("assets/textures/diffuse.bmp");
	// Need to tell the material the light's position
	// If you change the light's position you need to call this again
	modelMaterial->SetLightPosition(_lightPosition);
	// Tell the level object to use this material
	_level->SetMaterial(modelMaterial);

	// The mesh is the geometry for the object
	Mesh *groundMesh = new Mesh();
	// Load from OBJ file. This must have triangulated geometry
	groundMesh->LoadOBJ("assets/models/woodfloor.obj");
	// Tell the game object to use this mesh
	_level->SetMesh(groundMesh);
	_level->SetPosition(0.0f, 0.0f, 0.0f);
	_level->SetRotation(3.141590f, 0.0f, 0.0f);


	// Create the material for the game object- level
	Material *objectMaterial = new Material();
	Material* objectMaterial2 = new Material();
	// Shaders are now in files
	objectMaterial->LoadShaders("assets/shaders/VertShader.txt", "assets/shaders/FragShader.txt");
	objectMaterial2->LoadShaders("assets/shaders/VertShader.txt", "assets/shaders/FragShader.txt");
	// You can set some simple material properties, these values are passed to the shader
	// This colour modulates the texture colour
	objectMaterial->SetDiffuseColour(glm::vec3(0.8, 0.1, 0.1));
	objectMaterial2->SetDiffuseColour(glm::vec3(0.1, 0.8, 0.1));
	// The material currently supports one texture
	// This is multiplied by all the light components (ambient, diffuse, specular)
	// Note that the diffuse colour set with the line above will be multiplied by the texture colour
	// If you want just the texture colour, use modelMaterial->SetDiffuseColour( glm::vec3(1,1,1) );
	objectMaterial->SetTexture("assets/textures/default.bmp");
	objectMaterial2->SetTexture("assets/textures/default.bmp");
	// Need to tell the material the light's position
	// If you change the light's position you need to call this again
	objectMaterial->SetLightPosition(_lightPosition);
	objectMaterial2->SetLightPosition(_lightPosition);
	// Tell the level object to use this material
	_physics_object->SetMaterial(objectMaterial);
	_physics_object2->SetMaterial(objectMaterial2);

	// Set the geometry for the object
	Mesh *modelMesh = new Mesh();
	// Load from OBJ file. This must have triangulated geometry
	modelMesh->LoadOBJ("assets/models/sphere.obj");
	// Tell the game object to use this mesh
	_physics_object->SetMesh(modelMesh);
	_physics_object->SetPosition(0.0f, 100.0f, 0.0f);
	_physics_object->SetScale(0.3f, 0.3f, 0.3f);

	_physics_object2->SetMesh(modelMesh);
	_physics_object2->SetPosition(1.0f, 6.0f, 0.0f);
	_physics_object2->SetScale(0.3f, 0.3f, 0.3f);

	
}

Scene::~Scene()
{
	// You should neatly clean everything up here
	delete _physics_object;
	delete _physics_object2;
	delete _level;
	delete _camera;
}

float timeStep = 0;

void Scene::Update(float deltaTs, Input* input)
{

	
	// Update the game object (this is currently hard-coded motion)
	if (input->cmd_x)
	{
		_simulation_start = true;
	}
	if (_simulation_start == true)
	{

		glm::vec3 pos = _physics_object->GetPosition();
		glm::vec3 pos2 = _physics_object2->GetPosition();


		pos.y = (-9.8 * (deltaTs*deltaTs)) / 2;
		pos2.y = (-9.8 * (deltaTs * deltaTs)) / 2;

		// Collision detection 
		if (pos.y <= _level->GetPosition().y + 0.3f)
		{
			pos = glm::vec3(pos.x, _level->GetPosition().y + 0.3f, pos.z);
		}

		if (pos2.y <= _level->GetPosition().y + 0.3f)
		{
			pos2 = glm::vec3(pos2.x, _level->GetPosition().y + 0.3f, pos2.z);
		}

		_physics_object->SetPosition(pos);
		_physics_object2->SetPosition(pos2);
	}
	_physics_object->Update(deltaTs);
	_physics_object2->Update(deltaTs);
	_level->Update(deltaTs);
	_camera->Update(input);

	_viewMatrix = _camera->GetView();
	_projMatrix = _camera->GetProj();
														
}

void Scene::Draw()
{
	// Draw objects, giving the camera's position and projection
	_physics_object->Draw(_viewMatrix, _projMatrix);
	_physics_object2->Draw(_viewMatrix, _projMatrix);
	_level->Draw(_viewMatrix, _projMatrix);

}


