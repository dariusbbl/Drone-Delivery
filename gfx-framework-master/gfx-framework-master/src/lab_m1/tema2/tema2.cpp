#include "lab_m1/Tema2/tema2.h"

#include <vector>
#include <string>
#include <iostream>


using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema2::Tema2()
{
}


Tema2::~Tema2()
{
	delete drone;
	delete camera;
	for (auto shader : shaders) {
		delete shader.second;
	}
}

// matrici de transformare
// Translate matrix
inline glm::mat4 Translate(float translateX, float translateY, float translateZ)
{
	return glm::transpose(
		glm::mat4(1, 0, 0, translateX,
			0, 1, 0, translateY,
			0, 0, 1, translateZ,
			0, 0, 0, 1)
	);
}

// Scale matrix
inline glm::mat4 Scale(float scaleX, float scaleY, float scaleZ)
{
	return glm::transpose(
		glm::mat4(scaleX, 0, 0, 0,
			0, scaleY, 0, 0,
			0, 0, scaleZ, 0,
			0, 0, 0, 1)
	);
}

// Rotate matrix relative to the OZ axis
inline glm::mat4 RotateOZ(float radians)
{
	return glm::transpose(
		glm::mat4(cos(radians), -sin(radians), 0, 0,
			sin(radians), cos(radians), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1)
	);

}

// Rotate matrix relative to the OY axis
inline glm::mat4 RotateOY(float radians)
{
	return glm::transpose(
		glm::mat4(cos(radians), 0, sin(radians), 0,
			0, 1, 0, 0,
			-sin(radians), 0, cos(radians), 0,
			0, 0, 0, 1)
	);

}

// Rotate matrix relative to the OX axis
inline glm::mat4 RotateOX(float radians)
{
	return glm::transpose(
		glm::mat4(1, 0, 0, 0,
			0, cos(radians), -sin(radians), 0,
			0, sin(radians), cos(radians), 0,
			0, 0, 0, 1)
	);

}

void Tema2::Init()
{
	//polygonMode = GL_FILL;

	SimpleScene::Init();

	// shader pentru aspectul terenului
	Shader* terrainShader = new Shader("TerrainShader");
	terrainShader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
	terrainShader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
	terrainShader->CreateAndLink();
	shaders[terrainShader->GetName()] = terrainShader;

	// trimit datele de rezolutie la shader
	terrainShader->Use();

	// Initialize tx, ty and tz (the translation steps)
	translateX = 0;
	translateY = 0;
	translateZ = 0;

	// Initialize angular steps
	angularStepOX = 0;
	angularStepOY = 0;
	angularStepOZ = 0;

	// Sets the resolution of the small viewport
	glm::ivec2 resolution = window->GetResolution();
	miniViewportArea = ViewportArea(50, 50, resolution.x / 5.f, resolution.y / 5.f);

	drone = new Drone();
	drone->position = glm::vec3(rand() % 85 - 45, 2, rand() % 85 - 45);
	drone->rotation = glm::vec3(0);
	drone->scale = glm::vec3(1);
	drone->bladeSpeed = 10.0f;
	drone->bladeAngle = 0;

	// camera
	camera = new implemented::Camera();
	camera->Set(glm::vec3(0, 3, 5), drone->position, glm::vec3(0, 1, 0));
	camera->distanceToTarget = 5.0f;
	camera->SetFirstPerson(true);
	projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);

	// mesh pentru corpul dronei
	{
		glm::vec3 grayColor = glm::vec3(0.5f, 0.5f, 0.5f);
		std::vector<VertexFormat> vertices
		{
			VertexFormat(glm::vec3(-0.5f, -0.5f, -0.5f), grayColor),
			VertexFormat(glm::vec3(0.5f, -0.5f, -0.5f), grayColor),
			VertexFormat(glm::vec3(-0.5f, 0.5f, -0.5f), grayColor),
			VertexFormat(glm::vec3(0.5f, 0.5f, -0.5f), grayColor),
			VertexFormat(glm::vec3(-0.5f, -0.5f, 0.5f), grayColor),
			VertexFormat(glm::vec3(0.5f, -0.5f, 0.5f), grayColor),
			VertexFormat(glm::vec3(-0.5f, 0.5f, 0.5f), grayColor),
			VertexFormat(glm::vec3(0.5f, 0.5f, 0.5f), grayColor)
		};

		std::vector<unsigned int> indices = {
			0, 1, 2,    1, 3, 2,
			2, 3, 7,    2, 7, 6,
			1, 7, 3,    1, 5, 7,
			6, 7, 5,    6, 5, 4,
			0, 4, 1,    1, 4, 5,
			0, 2, 6,    0, 6, 4,
		};

		Mesh* bodyMesh = new Mesh("drone_body");
		bodyMesh->InitFromData(vertices, indices);
		meshes[bodyMesh->GetMeshID()] = bodyMesh;
	}

	// mesh pentru elice
	{
		glm::vec3 blackColor = glm::vec3(0.1f, 0.1f, 0.1f);
		std::vector<VertexFormat> vertices
		{
			VertexFormat(glm::vec3(-0.5f, -0.5f, -0.5f), blackColor),
			VertexFormat(glm::vec3(0.5f, -0.5f, -0.5f), blackColor),
			VertexFormat(glm::vec3(-0.5f, 0.5f, -0.5f), blackColor),
			VertexFormat(glm::vec3(0.5f, 0.5f, -0.5f), blackColor),
			VertexFormat(glm::vec3(-0.5f, -0.5f, 0.5f), blackColor),
			VertexFormat(glm::vec3(0.5f, -0.5f, 0.5f), blackColor),
			VertexFormat(glm::vec3(-0.5f, 0.5f, 0.5f), blackColor),
			VertexFormat(glm::vec3(0.5f, 0.5f, 0.5f), blackColor)
		};

		std::vector<unsigned int> indices = {
			0, 1, 2,    1, 3, 2,
			2, 3, 7,    2, 7, 6,
			1, 7, 3,    1, 5, 7,
			6, 7, 5,    6, 5, 4,
			0, 4, 1,    1, 4, 5,
			0, 2, 6,    0, 6, 4,
		};

		Mesh* propMesh = new Mesh("drone_blade");
		propMesh->InitFromData(vertices, indices);
		meshes[propMesh->GetMeshID()] = propMesh;
	}

	// mesh pentru teren
	meshes["terrain"] = CreateTerrain("terrain", 100);
	// mesh pentru copaci
	meshes["tree"] = CreateTree("tree");
	// pozitii random ale copacilor
	for (int i = 0; i < 40; i++) {
		bool validPosition = false;
		glm::vec3 newPos;

		while (!validPosition) {
			float x = rand() % 85 - 45;
			float z = rand() % 85 - 45;
			newPos = glm::vec3(x, 0, z);
			validPosition = true;

			// verific distanta fata de drona
			if (glm::distance(glm::vec2(newPos.x, newPos.z),
				glm::vec2(drone->position.x, drone->position.z)) < 5.0f) {
				validPosition = false;
				continue;
			}

			// verific distanta fata de alti copaci
			for (const auto& pos : treePositions) {
				if (glm::distance(glm::vec2(newPos.x, newPos.z), glm::vec2(pos.x, pos.z)) < 5.0f) {
					validPosition = false;
					break;
				}
			}
		}
		treePositions.push_back(newPos);
	}
	// scale aleator pentru copaci
	float treeScale = 5.0f + static_cast<float>(rand()) / RAND_MAX;
	treeSizes.push_back(treeScale);
	// mesh pentru cladiri
	meshes["building"] = CreateBuilding("building");
	// pozitii random ale cladirilor
	for (int i = 0; i < 8; i++) {
		bool validPosition = false;
		glm::vec3 newPos;

		while (!validPosition) {
			float x = rand() % 85 - 45;
			float z = rand() % 85 - 45;
			newPos = glm::vec3(x, 0, z);
			validPosition = true;

			// verific distanta fata de drona
			if (glm::distance(glm::vec2(newPos.x, newPos.z),
				glm::vec2(drone->position.x, drone->position.z)) < 6.0f) {
				validPosition = false;
				continue;
			}

			// verific distanta fata de alte cladiri
			for (const auto& pos : buildingPositions) {
				if (glm::distance(glm::vec2(newPos.x, newPos.z), glm::vec2(pos.x, pos.z)) < 6.0f) {
					validPosition = false;
					break;
				}
			}

			// verific distanta fata de copaci
			if (validPosition) {
				for (const auto& pos : treePositions) {
					if (glm::distance(glm::vec2(newPos.x, newPos.z), glm::vec2(pos.x, pos.z)) < 4.0f) {
						validPosition = false;
						break;
					}
				}
			}
		}

		buildingPositions.push_back(newPos);
		float width = 5.0f + static_cast<float>(rand()) / RAND_MAX;
		float height = 5.5f + static_cast<float>(rand()) / RAND_MAX * 2.0f;
		float depth = 5.0f + static_cast<float>(rand()) / RAND_MAX;
		buildingSizes.push_back(glm::vec3(width, height, depth));

	}

	// mesh pentru pachet (cutie)
	{
		std::vector<VertexFormat> vertices = {
			VertexFormat(glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(0.74f, 0.71f, 0.41f)), // culoare khaki
			VertexFormat(glm::vec3(0.5f, 0.0f, -0.5f), glm::vec3(0.74f, 0.71f, 0.41f)),
			VertexFormat(glm::vec3(-0.5f, 1.0f, -0.5f), glm::vec3(0.74f, 0.71f, 0.41f)),
			VertexFormat(glm::vec3(0.5f, 1.0f, -0.5f), glm::vec3(0.74f, 0.71f, 0.41f)),
			VertexFormat(glm::vec3(-0.5f, 0.0f, 0.5f), glm::vec3(0.74f, 0.71f, 0.41f)),
			VertexFormat(glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(0.74f, 0.71f, 0.41f)),
			VertexFormat(glm::vec3(-0.5f, 1.0f, 0.5f), glm::vec3(0.74f, 0.71f, 0.41f)),
			VertexFormat(glm::vec3(0.5f, 1.0f, 0.5f), glm::vec3(0.74f, 0.71f, 0.41f))
		};

		std::vector<unsigned int> indices = {
			0, 1, 2,    1, 3, 2,
			2, 3, 7,    2, 7, 6,
			1, 7, 3,    1, 5, 7,
			6, 7, 5,    6, 5, 4,
			0, 4, 1,    1, 4, 5,
			0, 2, 6,    0, 6, 4,
		};

		meshes["box"] = new Mesh("box");
		meshes["box"]->InitFromData(vertices, indices);
	}

	// mesh pentru destinatie (cerc)
	{
		std::vector<VertexFormat> vertices;
		std::vector<unsigned int> indices;

		// centrul cercului
		vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), glm::vec3(1, 0.84f, 0)));

		// puncte de circumferinta
		int numSegments = 32;
		for (int i = 0; i <= numSegments; i++) {
			float angle = 2.0f * M_PI * i / numSegments;
			float x = cos(angle);
			float z = sin(angle);
			vertices.push_back(VertexFormat(glm::vec3(x, 0, z), glm::vec3(1, 0.84f, 0)));
		}

		// indici pentru triunghiuri
		for (int i = 1; i <= numSegments; i++) {
			indices.push_back(0);          // centru
			indices.push_back(i);          // punct curent
			indices.push_back((i % numSegments) + 1);      // punctul circular
		}

		meshes["circle"] = new Mesh("circle");
		meshes["circle"]->InitFromData(vertices, indices);
		meshes["circle"]->SetDrawMode(GL_TRIANGLES);
	}

	// initializare pachet
	currentPackage = new Package();
	glm::vec3 packagePos = GenerateValidPosition();
	packagePos.y = 0.5f;
	currentPackage->position = packagePos;
	currentPackage->isPickedUp = false;
	currentPackage->isVisible = true;
	hasPickedUpPackage = false;
	// initializare destinatie
	currentDestination = new Destination();
	currentDestination->position = GenerateValidPosition();
	currentDestination->isVisible = false;
	currentDestination->radius = 3.0f;

	// contor de pachete colectate
	collectedPackages = 0;

	// mesh pentru sageata
	glm::vec3 arrowColor(1.0f, 0.0f, 0.0f); // culoare rosie
	std::vector<VertexFormat> vertices = {
		VertexFormat(glm::vec3(0, 0, -0.5), arrowColor),
		VertexFormat(glm::vec3(0.5, 0, 0.5), arrowColor),
		VertexFormat(glm::vec3(-0.5, 0, 0.5), arrowColor),
	};
	std::vector<unsigned int> indices = { 0, 1, 2 };
	Mesh* arrow = new Mesh("arrow");
	arrow->InitFromData(vertices, indices);
	meshes["arrow"] = arrow;

	// mesh pentru nori
	meshes["cloud"] = CreateCloud("cloud");

	// initializare nori la pozitii random
	for (int i = 0; i < NUM_CLOUDS; i++) {
		Cloud cloud;
		cloud.position = glm::vec3(
			(float)(rand() % (int)CLOUD_AREA) - CLOUD_AREA / 2,
			CLOUD_HEIGHT,
			(float)(rand() % (int)CLOUD_AREA) - CLOUD_AREA / 2
		);

		// scale random pentru varietate
		float baseScale = 2.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f;
		cloud.scale = glm::vec3(
			baseScale * (0.8f + static_cast<float>(rand()) / RAND_MAX * 0.4f),
			baseScale * 0.5f,
			baseScale * (0.8f + static_cast<float>(rand()) / RAND_MAX * 0.4f)
		);

		// viteza random
		cloud.speed = 0.5f + static_cast<float>(rand()) / RAND_MAX * 2.0f;

		// unghi random de miscare
		cloud.rotationAngle = static_cast<float>(rand()) / RAND_MAX * 2 * M_PI;

		clouds.push_back(cloud);
	}

}
void Tema2::FrameStart()
{
	// Clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0.52f, 0.8f, 0.98f, 0); // culoarea light sky blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Tema2::RenderDrone() {
	modelMatrix = glm::mat4(1);
	modelMatrix *= Translate(drone->position.x, drone->position.y, drone->position.z);
	modelMatrix *= RotateOY(drone->rotation.y);

	float armLength = 0.95f;
	float armWidth = 0.1f;
	float supportSize = 0.1f;

	// bratelei dronei
	{
		// brat 1
		glm::mat4 arm1Matrix = modelMatrix;
		arm1Matrix *= RotateOY(glm::radians(45.0f));
		arm1Matrix *= Scale(armLength, armWidth, armWidth);
		this->RenderMesh(meshes["drone_body"], shaders["VertexColor"], arm1Matrix);

		// brat 2 - face un X cu bratul 1
		glm::mat4 arm2Matrix = modelMatrix;
		arm2Matrix *= RotateOY(glm::radians(-45.0f));
		arm2Matrix *= Scale(armLength, armWidth, armWidth);
		this->RenderMesh(meshes["drone_body"], shaders["VertexColor"], arm2Matrix);
	}

	float bladePositions[4][2] = {
		{ 0.3f,  0.3f},
		{ 0.3f, -0.3f},
		{-0.3f,  0.3f},
		{-0.3f, -0.3f}
	};

	for (int i = 0; i < 4; i++) {
		float x = bladePositions[i][0];
		float z = bladePositions[i][1];

		// suportul elicei
		{
			glm::mat4 supportMatrix = modelMatrix;
			supportMatrix *= Translate(x, 0.05f, z);
			supportMatrix *= RotateOY(glm::radians(45.0f));
			supportMatrix *= Scale(supportSize, supportSize, supportSize);
			RenderMesh(meshes["drone_body"], shaders["VertexColor"], supportMatrix);
			this->RenderMesh(meshes["drone_body"], shaders["VertexColor"], supportMatrix);

		}

		// elicele
		{
			glm::mat4 blade1Matrix = modelMatrix;
			blade1Matrix *= Translate(x, 0.1f, z);
			blade1Matrix *= RotateOY(drone->bladeAngle);
			blade1Matrix *= Scale(0.4f, 0.02f, 0.02f);
			RenderMesh(meshes["drone_blade"], shaders["VertexColor"], blade1Matrix);
			this->RenderMesh(meshes["drone_blade"], shaders["VertexColor"], blade1Matrix);


			glm::mat4 blade2Matrix = modelMatrix;
			blade2Matrix *= Translate(x, supportSize, z);
			blade2Matrix *= RotateOY(drone->bladeAngle + glm::radians(90.0f));
			blade2Matrix *= Scale(0.4f, 0.02f, 0.02f);
			RenderMesh(meshes["drone_blade"], shaders["VertexColor"], blade2Matrix);
			this->RenderMesh(meshes["drone_blade"], shaders["VertexColor"], blade2Matrix);

		}
	}
}

// functie de creare a hartii
Mesh* Tema2::CreateTerrain(const std::string& name, int gridSize) {
	glm::vec3 color = glm::vec3(0.74f, 0.71f, 0.41f); // aspect de nisip pentru teren
	std::vector<VertexFormat> vertices;
	std::vector<unsigned int> indices;

	float cellSize = 1.0f;
	float halfSize = (gridSize * cellSize) / 2.0f;

	// generare vertices intr-un vector
	for (int i = 0; i <= gridSize; i++) {
		for (int j = 0; j <= gridSize; j++) {
			float x = (j * cellSize) - halfSize;
			float y = 0; // va fi modificat in shader pentru relief
			float z = (i * cellSize) - halfSize;
			vertices.push_back(VertexFormat(glm::vec3(x, y, z), color));
		}
	}

	// Generare indici pentru triunghiuri
	for (int i = 0; i < gridSize; i++) {
		for (int j = 0; j < gridSize; j++) {
			int topLeft = i * (gridSize + 1) + j;
			int topRight = topLeft + 1;
			int bottomLeft = (i + 1) * (gridSize + 1) + j;
			int bottomRight = bottomLeft + 1;

			// Primul triunghi
			indices.push_back(topLeft);
			indices.push_back(bottomLeft);
			indices.push_back(topRight);

			// Al doilea triunghi
			indices.push_back(topRight);
			indices.push_back(bottomLeft);
			indices.push_back(bottomRight);
		}
	}

	Mesh* terrain = new Mesh(name);
	terrain->InitFromData(vertices, indices);
	return terrain;
}

Mesh* Tema2::CreateTree(const std::string& name) {
	glm::vec3 trunkColor(0.5f, 0.35f, 0.05f);
	glm::vec3 leavesColor(0.0f, 0.8f, 0.0f);
	std::vector<VertexFormat> vertices;
	std::vector<unsigned int> indices;

	// trunchiul copacului - cilindru
	const int segments = 12;
	const float radius = 0.1f;

	for (int i = 0; i <= segments; i++) {
		float angle = 2.0f * M_PI * i / segments;
		float x = radius * cos(angle);
		float z = radius * sin(angle);

		vertices.push_back(VertexFormat(glm::vec3(x, 0, z), trunkColor));
		vertices.push_back(VertexFormat(glm::vec3(x, 0.5f, z), trunkColor));
	}

	for (int i = 0; i < segments; i++) {
		int base = i * 2;
		indices.push_back(base);
		indices.push_back(base + 2);
		indices.push_back(base + 1);

		indices.push_back(base + 1);
		indices.push_back(base + 2);
		indices.push_back(base + 3);
	}

	// primul con (mai mare) - de jos
	int baseIndex = vertices.size();
	vertices.push_back(VertexFormat(glm::vec3(0, 1.0f, 0), leavesColor));  // varf

	float baseRadius1 = 0.5f;
	float baseHeight1 = 0.4f;
	for (int i = 0; i <= segments; i++) {
		float angle = 2.0f * M_PI * i / segments;
		float x = baseRadius1 * cos(angle);
		float z = baseRadius1 * sin(angle);
		vertices.push_back(VertexFormat(glm::vec3(x, baseHeight1, z), leavesColor));
	}

	for (int i = 0; i < segments; i++) {
		indices.push_back(baseIndex);
		indices.push_back(baseIndex + 1 + i);
		indices.push_back(baseIndex + 2 + i);
	}

	// al doilea con (mai mic) - de sus
	baseIndex = vertices.size();
	vertices.push_back(VertexFormat(glm::vec3(0, 1.4f, 0), leavesColor));  // varf

	float baseRadius2 = 0.35f;
	float baseHeight2 = 0.8f;
	for (int i = 0; i <= segments; i++) {
		float angle = 2.0f * M_PI * i / segments;
		float x = baseRadius2 * cos(angle);
		float z = baseRadius2 * sin(angle);
		vertices.push_back(VertexFormat(glm::vec3(x, baseHeight2, z), leavesColor));
	}

	for (int i = 0; i < segments; i++) {
		indices.push_back(baseIndex);
		indices.push_back(baseIndex + 1 + i);
		indices.push_back(baseIndex + 2 + i);
	}

	Mesh* tree = new Mesh(name);
	tree->InitFromData(vertices, indices);
	return tree;
}

// functie de randare a cladirilor
Mesh* Tema2::CreateBuilding(const std::string& name) {
	glm::vec3 buildingColor(0.7f, 0.7f, 0.7f); // gri
	glm::vec3 windowColor(0.3f, 0.5f, 1.0f); // albastrul geamurilor
	glm::vec3 doorColor(0.45f, 0.25f, 0.15f); // maroul usii

	std::vector<VertexFormat> vertices;
	std::vector<unsigned int> indices;

	// corp principal
	vertices = {
		// fata
		VertexFormat(glm::vec3(-0.5f, 0, 0.5f), buildingColor),
		VertexFormat(glm::vec3(0.5f, 0, 0.5f), buildingColor),
		VertexFormat(glm::vec3(-0.5f, 1.0f, 0.5f), buildingColor),
		VertexFormat(glm::vec3(0.5f, 1.0f, 0.5f), buildingColor),
		// spate
		VertexFormat(glm::vec3(-0.5f, 0, -0.5f), buildingColor),
		VertexFormat(glm::vec3(0.5f, 0, -0.5f), buildingColor),
		VertexFormat(glm::vec3(-0.5f, 1.0f, -0.5f), buildingColor),
		VertexFormat(glm::vec3(0.5f, 1.0f, -0.5f), buildingColor)
	};

	indices = {
		0, 1, 2,    1, 3, 2,
		4, 5, 6,    5, 7, 6,
		2, 3, 6,    3, 7, 6,
		0, 1, 4,    1, 5, 4,
		0, 2, 4,    2, 6, 4,
		1, 3, 5,    3, 7, 5
	};

	// usa cladirii
	int doorBase = vertices.size();
	vertices.push_back(VertexFormat(glm::vec3(-0.15f, 0, 0.501f), doorColor));
	vertices.push_back(VertexFormat(glm::vec3(0.15f, 0, 0.501f), doorColor));
	vertices.push_back(VertexFormat(glm::vec3(-0.15f, 0.3f, 0.501f), doorColor));
	vertices.push_back(VertexFormat(glm::vec3(0.15f, 0.3f, 0.501f), doorColor));

	indices.push_back(doorBase);
	indices.push_back(doorBase + 1);
	indices.push_back(doorBase + 2);
	indices.push_back(doorBase + 1);
	indices.push_back(doorBase + 3);
	indices.push_back(doorBase + 2);

	// ferestrele cladirii
	float heights[] = { 0.4f, 0.6f, 0.8f };
	for (float h : heights) {
		int windowBase = vertices.size();
		vertices.push_back(VertexFormat(glm::vec3(-0.4f, h, 0.501f), windowColor));
		vertices.push_back(VertexFormat(glm::vec3(-0.2f, h, 0.501f), windowColor));
		vertices.push_back(VertexFormat(glm::vec3(-0.4f, h + 0.1f, 0.501f), windowColor));
		vertices.push_back(VertexFormat(glm::vec3(-0.2f, h + 0.1f, 0.501f), windowColor));

		indices.push_back(windowBase);
		indices.push_back(windowBase + 1);
		indices.push_back(windowBase + 2);
		indices.push_back(windowBase + 1);
		indices.push_back(windowBase + 3);
		indices.push_back(windowBase + 2);

		// o alta fereastra pe acelasi rand
		windowBase = vertices.size();
		vertices.push_back(VertexFormat(glm::vec3(0.2f, h, 0.501f), windowColor));
		vertices.push_back(VertexFormat(glm::vec3(0.4f, h, 0.501f), windowColor));
		vertices.push_back(VertexFormat(glm::vec3(0.2f, h + 0.1f, 0.501f), windowColor));
		vertices.push_back(VertexFormat(glm::vec3(0.4f, h + 0.1f, 0.501f), windowColor));

		indices.push_back(windowBase);
		indices.push_back(windowBase + 1);
		indices.push_back(windowBase + 2);
		indices.push_back(windowBase + 1);
		indices.push_back(windowBase + 3);
		indices.push_back(windowBase + 2);
	}

	Mesh* building = new Mesh(name);
	building->InitFromData(vertices, indices);
	return building;
}

// functie pentru coliziunea dintre drona si pamant
void Tema2::CheckDroneGroundCollision() {
	// daca drona a coborat pana la valoarea 0 pe axa oy, o opresc => coliziune cu pamantul
	if (drone->position.y < 0.5f) {
		drone->position.y = 0.5f;
	}
}

// functie pentru verificarea coliziunii cu copacii
bool Tema2::CollisionWithTree(const glm::vec3& center1, float radius1, const glm::vec3& center2, float radius2) {
	// sa nu se blocheze drona daca este deasupra copacului
	float treeHeight = treeSizes[0] * 1.4f;
	if (center1.y > treeHeight) {
		return false;
	}

	// calculez raza la inaltimea dronei
	float baseRadius = 0.5f * treeSizes[0]; // raza la baza copacului
	float height = center1.y;

	// calculez raza efectiva in functie de inaltime (difera din cauza formelor de con)
	float effectiveRadius;

	if (height <= 0.5f * treeSizes[0]) { // pentru trunchi si baza primului con
		effectiveRadius = baseRadius * 0.8f;
	}
	else if (height <= treeSizes[0]) { // pentru primul con
		float t = (height - 0.5f * treeSizes[0]) / (0.5f * treeSizes[0]);
		effectiveRadius = baseRadius * (0.8f - 0.4f * t);
	}
	else { // pentru al doilea con
		float t = (height - treeSizes[0]) / (0.4f * treeSizes[0]);
		effectiveRadius = baseRadius * (0.4f - 0.3f * t);
	}

	// marim raza de coliziune cu un offset
	effectiveRadius *= 1.2f;

	// distanta pe planul XZ
	float distance = glm::length(glm::vec2(center1.x - center2.x, center1.z - center2.z));

	// returnez true daca exisra coliziune
	return distance < (radius1 + effectiveRadius);
}


bool Tema2::CollisionWithBuilding(const glm::vec3& sphereCenter, float radius, const glm::vec3& boxMin, const glm::vec3& boxMax) {
	// verific indexul cladirii pentru scalare
	size_t buildingIndex = 0;
	for (size_t i = 0; i < buildingPositions.size(); i++) {
		if (glm::distance(buildingPositions[i], (boxMin + boxMax) * 0.5f) < 0.1f) {
			buildingIndex = i;
			break;
		}
	}

	// scale-ul fiecarei cladiri
	glm::vec3 buildingScale = buildingSizes[buildingIndex];
	glm::vec3 buildingPos = buildingPositions[buildingIndex];

	// box-ul real al cladirii dupa scalare
	glm::vec3 scaledHalfSize = buildingScale * 0.5f;
	glm::vec3 scaledBoxMin = buildingPos - glm::vec3(scaledHalfSize.x, 0, scaledHalfSize.z);
	glm::vec3 scaledBoxMax = buildingPos + glm::vec3(scaledHalfSize.x, buildingScale.y, scaledHalfSize.z);

	// verificarea coliziunii pe fiecare axa
	bool collisionX = sphereCenter.x + radius > scaledBoxMin.x && sphereCenter.x - radius < scaledBoxMax.x;
	bool collisionY = sphereCenter.y + radius > scaledBoxMin.y && sphereCenter.y - radius < scaledBoxMax.y;
	bool collisionZ = sphereCenter.z + radius > scaledBoxMin.z && sphereCenter.z - radius < scaledBoxMax.z;

	// returnez true daca exista coliziune
	return collisionX && collisionY && collisionZ;
}

void Tema2::UpdatePackageSystem(float deltaTimeSeconds) {
	// verific coliziunea cu pachetul
	if (!currentPackage->isPickedUp && currentPackage->isVisible) {
		float distance = glm::length(drone->position - currentPackage->position);
		if (distance < 2.0f) {
			currentPackage->isPickedUp = true;
			currentDestination->isVisible = true;
		}
	}

	// actualizez pozitia pachetului cand e preluat
	if (currentPackage->isPickedUp) {
		currentPackage->position = drone->position - glm::vec3(0, 1.0f, 0);

		// verific livrarea la destinatie
		float distToDestination = glm::length(
			glm::vec2(drone->position.x - currentDestination->position.x,
				drone->position.z - currentDestination->position.z));
		if (!hasPickedUpPackage) {
			std::cout << endl;
			std::cout << "Picked up the package!" << std::endl;
			hasPickedUpPackage = true; // mesaj afisat
		}
		if (distToDestination < currentDestination->radius) {
			// generez pachet si destinatie noi, random
			collectedPackages++;

			// afisez in consola numarul de pachete colectate
			std::cout << "Successfully delivered the package!" << std::endl;
			std::cout << "Delivered packages: " << collectedPackages << std::endl;

			currentPackage->position = GenerateValidPosition();
			currentPackage->isPickedUp = false;
			hasPickedUpPackage = false;

			currentDestination->position = GenerateValidPosition();
			currentDestination->isVisible = false;
		}
	}
}

// functie pentru randarea pachetului si destinatiei
void Tema2::RenderPackageSystem() {
	if (currentPackage->isVisible) {
		// randez cutia pachetului
		glm::mat4 packageModel = glm::mat4(1);
		packageModel = glm::translate(packageModel, currentPackage->position);
		packageModel = glm::scale(packageModel, glm::vec3(0.7f));
		RenderMesh(meshes["box"], shaders["VertexColor"], packageModel);
		if (!currentPackage->isPickedUp) {
			// randez cerc pentru a fi vizibil usor
			glm::mat4 circleModel = glm::mat4(1);
			circleModel = glm::translate(circleModel, currentPackage->position);
			circleModel = glm::scale(circleModel, glm::vec3(3.0f)); // Ajustăm dimensiunea cercului
			glUniform3f(glGetUniformLocation(shaders["VertexColor"]->program, "overrideColor"), 1.0f, 0.0f, 1.0f); // Mov
			RenderMesh(meshes["circle"], shaders["VertexColor"], circleModel);
			glUniform3f(glGetUniformLocation(shaders["VertexColor"]->program, "overrideColor"), -1.0f, -1.0f, -1.0f); // Resetăm culoarea
		}
	}

	if (currentDestination->isVisible) {
		glm::mat4 destModel = glm::mat4(1);
		destModel = glm::translate(destModel, currentDestination->position);
		destModel = glm::scale(destModel, glm::vec3(currentDestination->radius));
		RenderMesh(meshes["circle"], shaders["VertexColor"], destModel);
	}
}

// functie pentru verificarea generarii pachetelor si destinatiilor in afara razei copacilor/cladirilor
bool Tema2::IsValidLocation(const glm::vec3& position, float radius) {
	// verific sa nu initializez in raza dronei
	float droneRadius = 2.0f; // raza din jurul dronei
	if (glm::distance(glm::vec2(position.x, position.z),
		glm::vec2(drone->position.x, drone->position.z)) < (radius + droneRadius)) {
		return false;
	}

	// verific intersectia cu copacii
	for (const auto& treePos : treePositions) {
		float treeRadius = 0.5f * treeSizes[0];
		if (glm::distance(glm::vec2(position.x, position.z),
			glm::vec2(treePos.x, treePos.z)) < (radius + treeRadius)) {
			return false;
		}
	}

	// verific intersectia cu cladirile
	for (size_t i = 0; i < buildingPositions.size(); i++) {
		glm::vec3 halfSize = buildingSizes[i] * 0.5f;
		glm::vec3 boxMin = buildingPositions[i] - halfSize;
		glm::vec3 boxMax = buildingPositions[i] + halfSize;

		if (position.x >= boxMin.x - radius && position.x <= boxMax.x + radius &&
			position.z >= boxMin.z - radius && position.z <= boxMax.z + radius) {
			return false;
		}
	}

	return true;
}

// functie pentru impiedicarea generarii in zone nevalide
glm::vec3 Tema2::GenerateValidPosition() {
	glm::vec3 position;
	do {
		position = glm::vec3(rand() % 85 - 45, 0.5f, rand() % 85 - 45);
	} while (!IsValidLocation(position, 2.0f));
	return position;
}

// functie pentru randarea sagetii de directie
void Tema2::RenderDirectionArrow() {
	glm::vec3 targetPos;

	// determin directia sagetii (catre pachet sau destinatie)
	if (!currentPackage->isPickedUp) {
		targetPos = currentPackage->position;
	}
	else {
		targetPos = currentDestination->position;
	}

	// calculez distanta catre tinta
	glm::vec3 directionToTarget = glm::normalize(glm::vec3(targetPos.x - drone->position.x, 0, targetPos.z - drone->position.z));
	float angleToTarget = atan2(directionToTarget.x, directionToTarget.z);
	angleToTarget += glm::pi<float>();

	// pozitia sagetii fata de drona
	glm::vec3 arrowOffset = camera->forward * 3.8f;
	glm::vec3 arrowPosition = drone->position + arrowOffset;
	arrowPosition.y = drone->position.y - 0.5f;

	// pozitia sagetii in ecran
	glm::vec4 projectedPos = projectionMatrix * camera->GetViewMatrix() * glm::vec4(arrowPosition, 1.0f);

	// sa nu poata iesi sageata din ecran
	if (projectedPos.y / projectedPos.w < -0.9f) {
		arrowPosition.y = drone->position.y - 0.3f;
	}

	// matricea de modelare a sagetii
	glm::mat4 arrowModel = glm::mat4(1);
	arrowModel = glm::translate(arrowModel, arrowPosition);
	arrowModel = glm::rotate(arrowModel, angleToTarget, glm::vec3(0, 1, 0));
	arrowModel = glm::scale(arrowModel, glm::vec3(1.0f, 1.0f, 3.0f));

	// randez sageata
	RenderMesh(meshes["arrow"], shaders["VertexColor"], arrowModel);
}

// functie e creare a norilor
Mesh* Tema2::CreateCloud(const std::string& name) {
	std::vector<VertexFormat> vertices;
	std::vector<unsigned int> indices;
	glm::vec3 cloudColor = glm::vec3(0.9f, 0.9f, 0.9f);  // alb cu putin gri

	// Parametri pentru generarea sferelor
	const int stacks = 10;
	const int sectors = 12;
	const float radius = 1.0f;

	// Functie helper pentru adăugarea unei sfere la pozitia specificata
	auto addSphere = [&](float xOffset, float yOffset, float zOffset, float scale) {
		int baseIndex = vertices.size();

		// Generare vertices pentru sferă
		for (int i = 0; i <= stacks; ++i) {
			float stackAngle = M_PI / 2 - i * M_PI / stacks;
			float xy = radius * cosf(stackAngle);
			float z = radius * sinf(stackAngle);

			for (int j = 0; j <= sectors; ++j) {
				float sectorAngle = j * 2 * M_PI / sectors;
				float x = xy * cosf(sectorAngle);
				float y = xy * sinf(sectorAngle);

				glm::vec3 pos = glm::vec3(
					(x * scale + xOffset),
					(z * scale + yOffset),
					(y * scale + zOffset)
				);

				// Adăugăm o mică variație în culoare pentru mai mult realism
				float colorVar = 0.95f + static_cast<float>(rand()) / RAND_MAX * 0.05f;
				glm::vec3 sphereColor = cloudColor * colorVar;

				vertices.push_back(VertexFormat(pos, sphereColor));
			}
		}

		// Generare indices pentru sferă
		for (int i = 0; i < stacks; ++i) {
			int k1 = i * (sectors + 1);
			int k2 = (i + 1) * (sectors + 1);

			for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
				if (i != 0) {
					indices.push_back(baseIndex + k1);
					indices.push_back(baseIndex + k2);
					indices.push_back(baseIndex + k1 + 1);
				}

				if (i != (stacks - 1)) {
					indices.push_back(baseIndex + k1 + 1);
					indices.push_back(baseIndex + k2);
					indices.push_back(baseIndex + k2 + 1);
				}
			}
		}
		};

	// Generam mai multe sfere suprapuse pentru a crea forma norului
	// Sfera centrală
	addSphere(0, 0, 0, 1.0f);

	// Sfere adiționale pentru a crea forma neregulatã
	const int numExtraSpheres = 6;
	for (int i = 0; i < numExtraSpheres; ++i) {
		float xOffset = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.5f;
		float yOffset = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f;
		float zOffset = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.5f;
		float scale = 0.6f + static_cast<float>(rand()) / RAND_MAX * 0.4f;

		addSphere(xOffset, yOffset, zOffset, scale);
	}

	Mesh* cloud = new Mesh(name);
	cloud->InitFromData(vertices, indices);
	return cloud;
}

// functie pentru actualizarea pozitiilor norilor
void Tema2::UpdateClouds(float deltaTimeSeconds) {
	for (auto& cloud : clouds) {
		// Actualizam pozitia norului
		cloud.position.x += cos(cloud.rotationAngle) * cloud.speed * deltaTimeSeconds;
		cloud.position.z += sin(cloud.rotationAngle) * cloud.speed * deltaTimeSeconds;

		// Daca norul iese din zona, il resetam pe partea opusa
		if (cloud.position.x > CLOUD_AREA / 2) cloud.position.x = -CLOUD_AREA / 2;
		if (cloud.position.x < -CLOUD_AREA / 2) cloud.position.x = CLOUD_AREA / 2;
		if (cloud.position.z > CLOUD_AREA / 2) cloud.position.z = -CLOUD_AREA / 2;
		if (cloud.position.z < -CLOUD_AREA / 2) cloud.position.z = CLOUD_AREA / 2;
	}
}

// functie pentru randarea scenei
void Tema2::RenderScene(const glm::mat4& viewMatrix, float deltaTimeSeconds) {
	// randez terenul
	glm::mat4 terrainModel = glm::scale(glm::mat4(1), glm::vec3(5.0f));
	RenderMesh(meshes["terrain"], shaders["TerrainShader"], terrainModel);

	// randez norii
	for (const auto& cloud : clouds) {
		glm::mat4 cloudModel = glm::mat4(1);
		cloudModel = glm::translate(cloudModel, cloud.position);
		cloudModel = glm::scale(cloudModel, cloud.scale);
		RenderMesh(meshes["cloud"], shaders["VertexColor"], cloudModel);
	}

	// randez copacii
	for (const auto& pos : treePositions) {
		glm::mat4 treeModel = glm::translate(glm::mat4(1), pos);
		treeModel = glm::scale(treeModel, glm::vec3(treeSizes[0]));
		RenderMesh(meshes["tree"], shaders["VertexColor"], treeModel);
	}

	// randez cladirile
	for (size_t i = 0; i < buildingPositions.size(); i++) {
		glm::mat4 buildingModel = glm::mat4(1);
		buildingModel = glm::translate(buildingModel, buildingPositions[i]);
		buildingModel = glm::scale(buildingModel, buildingSizes[i]);
		RenderMesh(meshes["building"], shaders["VertexColor"], buildingModel);
	}

	// randez drona
	RenderDrone();

	RenderPackageSystem();
}


void Tema2::Update(float deltaTimeSeconds)
{
	// verific daca ambele obiecte sunt initializate
	if (camera && drone)
	{
		camera->UpdatePosition(drone->position, drone->rotation.y);
	}

	drone->UpdateBlade(deltaTimeSeconds);
	CheckDroneGroundCollision();

	// randez pentru camera principala (viewport principal)
	glViewport(0, 0, window->GetResolution().x, window->GetResolution().y);
	glm::mat4 mainViewMatrix = camera->GetViewMatrix();
	UpdateClouds(deltaTimeSeconds);
	RenderScene(mainViewMatrix, deltaTimeSeconds);
	RenderDirectionArrow();
	// randez pentru camera de minimap (viewport secundar)
	glViewport(window->GetResolution().x - miniViewportArea.width - 10,  // pozitie in dreapta sus
		window->GetResolution().y - miniViewportArea.height - 10,
		miniViewportArea.width, miniViewportArea.height);

	float viewSize = 10.0f; // valoare mai mica = zoom in
	glm::mat4 orthoProjectionMatrix = glm::ortho(-viewSize, viewSize, -viewSize, viewSize, 0.1f, 100.0f);

	// camera deasupra dronei
	glm::mat4 minimapViewMatrix = glm::lookAt(
		glm::vec3(drone->position.x, drone->position.y + 50.0f, drone->position.z), // deasupra dronei cu 50 unitati
		glm::vec3(drone->position.x, drone->position.y, drone->position.z),         // privire catre drona
		glm::vec3(0, 0, -1)                                                          // vector de directie
	);

	projectionMatrix = orthoProjectionMatrix;

	RenderDirectionArrow();
	RenderScene(minimapViewMatrix, deltaTimeSeconds);

	projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);

	UpdatePackageSystem(deltaTimeSeconds);
	// resetez viewport-ul pentru urmatorul cadru
	glViewport(0, 0, window->GetResolution().x, window->GetResolution().y);
}

void Tema2::FrameEnd()
{
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
	const float speed = 6.0f;
	const float rotationSpeed = 2.0f;
	const float droneRadius = 0.5f;

	glm::vec3 forward = camera->forward;
	forward.y = 0; // elimin componenta y
	forward = glm::normalize(forward);

	glm::vec3 right = camera->right;
	right.y = 0;
	right = glm::normalize(right);

	glm::vec3 newPosition = drone->position;

	// deplasare inainte
	if (window->KeyHold(GLFW_KEY_W)) {
		newPosition += forward * deltaTime * speed;
	}

	// deplasare inapoi
	if (window->KeyHold(GLFW_KEY_S)) {
		newPosition -= forward * deltaTime * speed;
	}

	// deplasare la stanga
	if (window->KeyHold(GLFW_KEY_A)) {
		newPosition -= right * deltaTime * speed;
	}

	// deplasare la dreapta
	if (window->KeyHold(GLFW_KEY_D)) {
		newPosition += right * deltaTime * speed;
	}

	// deplasare in sus
	if (window->KeyHold(GLFW_KEY_Q)) {
		newPosition.y += deltaTime * speed;
	}

	// deplasare in jos
	if (window->KeyHold(GLFW_KEY_E)) {
		newPosition.y -= deltaTime * speed;
	}

	bool collision = false;
	//coliziune cu copacii
	for (size_t i = 0; i < treePositions.size() && !collision; i++) {
		float treeRadius = 0.4f * treeSizes[0];
		if (CollisionWithTree(newPosition, droneRadius, treePositions[i], treeRadius)) {
			collision = true;
		}
	}

	// coliziune cu cladirile
	for (size_t i = 0; i < buildingPositions.size() && !collision; i++) {
		glm::vec3 halfSize = buildingSizes[i] * 0.5f;
		glm::vec3 boxMin = buildingPositions[i] - halfSize;
		glm::vec3 boxMax = buildingPositions[i] + halfSize;

		if (CollisionWithBuilding(newPosition, droneRadius, boxMin, boxMax)) {
			collision = true;
			break;
		}
	}

	// actualizam pozitia doar daca nu a fost detectata o coliziune
	if (!collision) {
		drone->position = newPosition;
	}

	// rotire la stanga
	if (window->KeyHold(GLFW_KEY_LEFT)) {
		drone->rotation.y -= deltaTime * rotationSpeed;
	}

	// rotire la dreapta
	if (window->KeyHold(GLFW_KEY_RIGHT)) {
		drone->rotation.y += deltaTime * rotationSpeed;
	}
}


void Tema2::OnKeyPress(int key, int mods)
{
	// trecerea dintre first person si third person
	if (key == GLFW_KEY_P) {
		static bool firstPerson = true;
		firstPerson = !firstPerson;
		camera->SetFirstPerson(firstPerson);
	}
}

void Tema2::RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix)
{
	if (!mesh || !shader || !camera)
		return;

	shader->Use();
	glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	mesh->Render();
}

void Tema2::OnKeyRelease(int key, int mods)
{
	// Add key release event
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// Add mouse move event
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// Add mouse button press event
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// Add mouse button release event
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema2::OnWindowResize(int width, int height)
{
}
