#include "Scene.h"


/*! \brief Brief description.
*  Scene class is a container for loading all the game objects in your simulation or your game.
*
*/
Scene::Scene()
{
	getFileCode("Input.txt");

	for (size_t i = 0; i < _fileCode.size(); i++)
	{
		std::cout << "File content includes: " << _fileCode.at(i) << "\n";
	}
	

	// Set up your scene here......
	// Set a camera
	_camera = new Camera();
	// Don't start simulation yet
	_simulation_start = false;

	// Position of the light, in world-space
	_lightPosition = glm::vec3(10, 10, 0);

	glm::vec3 _v_i = glm::vec3(10.0f, 10.5f, 0.0f);
	// Create a game level object

	// Create the material for the game object- level
	Material* modelMaterial = new Material();
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


	// The mesh is the geometry for the object
	Mesh* groundMesh = new Mesh();
	// Load from OBJ file. This must have triangulated geometry
	groundMesh->LoadOBJ("assets/models/woodfloor.obj");
	// Tell the game object to use this mesh



	// Create the material for the game object- level
	Material* objectMaterial = new Material();
	// Shaders are now in files
	objectMaterial->LoadShaders("assets/shaders/VertShader.txt", "assets/shaders/FragShader.txt");
	// You can set some simple material properties, these values are passed to the shader
	// This colour modulates the texture colour
	objectMaterial->SetDiffuseColour(glm::vec3(0.8, 0.1, 0.1));
	// The material currently supports one texture
	// This is multiplied by all the light components (ambient, diffuse, specular)
	// Note that the diffuse colour set with the line above will be multiplied by the texture colour
	// If you want just the texture colour, use modelMaterial->SetDiffuseColour( glm::vec3(1,1,1) );
	objectMaterial->SetTexture("assets/textures/default.bmp");
	// Need to tell the material the light's position
	// If you change the light's position you need to call this again
	objectMaterial->SetLightPosition(_lightPosition);
	// Tell the level object to use this material


	// Set the geometry for the object
	Mesh* modelMesh = new Mesh();
	// Load from OBJ file. This must have triangulated geometry
	modelMesh->LoadOBJ("assets/models/sphere.obj");
	// Tell the game object to use this mesh

	int spheres = 3;
	int planes = 1;

	for (int i = 0; i < spheres; i++)
	{
		DynamicObject* newObj = CreateSphere(1, objectMaterial, modelMesh, glm::vec3(0.0f + i, 20.0f, 0.0f), glm::vec3(std::stof(_fileCode.at(1)), std::stof(_fileCode.at(1)), std::stof(_fileCode.at(1))), std::stof(_fileCode.at(0)) , std::stof(_fileCode.at(1)));

		_sceneDynamicObjects.push_back(newObj);
	}

	for (int i = 0; i < planes; i++)
	{
		GameObject* newObj = CreatePlane(0, modelMaterial, groundMesh, glm::vec3(0.0f + i * 10, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f));

		_sceneGameObjects.push_back(newObj);
	}

	// test object
	DynamicObject* newObj = CreateSphere(1, objectMaterial, modelMesh, glm::vec3(0.2f, 25.0f, 0.0f), glm::vec3(0.3f, 0.3f, 0.3f), 2.0f, 0.3f);
	_sceneDynamicObjects.push_back(newObj);
}

Scene::~Scene()
{
	// You should neatly clean everything up here
	delete _camera;

	for (size_t i = 0; i < _sceneDynamicObjects.size(); i++)
	{
		delete _sceneDynamicObjects.at(i);
	}

	for (size_t i = 0; i < _sceneGameObjects.size(); i++)
	{
		delete _sceneGameObjects.at(i);
	}
}

void Scene::Update(float deltaTs, Input* input)
{


	// Update the game object (this is currently hard-coded motion)
	if (input->cmd_x)
	{
		_simulation_start = true;
	}
	if (_simulation_start == true)
	{
		for (int i = 0; i < _sceneDynamicObjects.size(); i++)
		{
			_sceneDynamicObjects.at(i)->StartSimulation(_simulation_start);
		}
	}

	for (size_t i = 0; i < _sceneGameObjects.size(); i++)
	{
		_sceneGameObjects.at(i)->Update(deltaTs);
	}

	for (int j = 0; j < _sceneDynamicObjects.size(); j++)
	{
		// For each game object that exists, pass it into 
		for (size_t i = 0; i < _sceneGameObjects.size(); i++)
		{
			_sceneDynamicObjects.at(j)->Update(_sceneGameObjects.at(i), deltaTs /  3);
			//_sceneDynamicObjects.at(j)->CollisionCheck(_sceneGameObjects.at(i));
		}

		for (size_t k = 0; k < _sceneDynamicObjects.size(); k++)
		{
			if (k == j)
			{
				continue;
			}
			else
			{
				_sceneDynamicObjects.at(j)->Update(_sceneDynamicObjects.at(k), deltaTs / 7);
			}
		}
	}


	

	_camera->Update(input);

	_viewMatrix = _camera->GetView();
	_projMatrix = _camera->GetProj();
														
}

void Scene::Draw()
{
	// Draw objects, giving the camera's position and projection

	for (int i = 0; i < _sceneDynamicObjects.size(); i++)
	{
		_sceneDynamicObjects.at(i)->Draw(_viewMatrix, _projMatrix);
	}

	for each (GameObject* obj in _sceneGameObjects)
	{
		obj->Draw(_viewMatrix, _projMatrix);
	}

}


DynamicObject* Scene::CreateSphere(int objectType, Material* material, Mesh* modelMesh, glm::vec3 position, glm::vec3 scale, float mass, float boundingRad)
{
	DynamicObject* object = new DynamicObject();
	object->SetMaterial(material);
	object->SetMesh(modelMesh);
	object->SetPosition(position);
	object->SetScale(scale);
	object->SetMass(mass);
	object->SetBoundingRadius(boundingRad);
	object->SetType(objectType);

	return object;
}

GameObject* Scene::CreatePlane(int objectType, Material* material, Mesh* modelMesh, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
{
	GameObject* object = new GameObject();
	object->SetMaterial(material);
	object->SetMesh(modelMesh);
	object->SetPosition(position);
	object->SetRotation(rotation.x, rotation.y, rotation.z);
	object->SetType(objectType);
	object->SetScale(scale.x, scale.y, scale.z);

	return object;
}

void Scene::getFileCode(std::string fileName)
{
	std::string line;
	std::ifstream myfile(fileName);
	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			std::cout << line << '\n';

			_fileCode.push_back(line);
		}

		myfile.close();
	}
}