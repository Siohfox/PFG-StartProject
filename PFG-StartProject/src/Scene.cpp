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
	
	// Set a camera
	_camera = new Camera();

	// Don't start simulation yet
	_simulation_start = false;

	// Position of the light, in world-space
	_lightPosition = glm::vec3(10, 10, 0);

	// Create the material for the planes
	Material* modelMaterial = new Material();
	modelMaterial->LoadShaders("assets/shaders/VertShader.txt", "assets/shaders/FragShader.txt");
	modelMaterial->SetDiffuseColour(glm::vec3(0.8, 0.8, 0.8));
	modelMaterial->SetTexture("assets/textures/diffuse.bmp");
	modelMaterial->SetLightPosition(_lightPosition);

	// Load Mesh of planes
	Mesh* groundMesh = new Mesh();
	groundMesh->LoadOBJ("assets/models/woodfloor.obj");

	// Create the material for the spheres
	Material* objectMaterial = new Material();
	objectMaterial->LoadShaders("assets/shaders/VertShader.txt", "assets/shaders/FragShader.txt");
	objectMaterial->SetDiffuseColour(glm::vec3(0.8, 0.1, 0.1));
	objectMaterial->SetTexture("assets/textures/default.bmp");
	objectMaterial->SetLightPosition(_lightPosition);

	// Load Mesh of spheres
	Mesh* modelMesh = new Mesh();
	modelMesh->LoadOBJ("assets/models/sphere.obj");

	// Initialise the amount of objects that will be loaded (read in via file)
	int spheres = std::stoi(_fileCode.at(0));
	int planes = 1;

	for (int i = 0; i < spheres; i++)
	{
		DynamicObject* newObj = CreateSphere(1, objectMaterial, modelMesh, glm::vec3(0.0f + i, 20.0f, 0.0f), glm::vec3(std::stof(_fileCode.at(2)), std::stof(_fileCode.at(2)), std::stof(_fileCode.at(2))), std::stof(_fileCode.at(1)) , std::stof(_fileCode.at(2)));

		_sceneDynamicObjects.push_back(newObj);
	}

	for (int i = 0; i < planes; i++)
	{
		GameObject* newObj = CreatePlane(0, modelMaterial, groundMesh, glm::vec3(0.0f + i * 10, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 2.0f, 2.0f));

		_sceneGameObjects.push_back(newObj);
	}

	// test object to spawn above the others
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
			_sceneDynamicObjects.at(j)->Update(_sceneGameObjects.at(i), deltaTs /  _sceneGameObjects.size());
		}

		for (size_t k = 0; k < _sceneDynamicObjects.size(); k++)
		{
			if (k == j)
			{
				continue;
			}
			else
			{
				_sceneDynamicObjects.at(j)->Update(_sceneDynamicObjects.at(k), deltaTs / (_sceneDynamicObjects.size() * 2));
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