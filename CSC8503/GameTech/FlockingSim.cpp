#include "FlockingSim.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

using namespace NCL;
using namespace CSC8503;

FlockingSim::FlockingSim()
{
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	physics = new PhysicsSystem(*world);

	forceMagnitude = 10.0f;
	useGravity = false;
	inSelectionMode = false;

	Debug::SetRenderer(renderer);

	InitialiseAssets();
}

FlockingSim::~FlockingSim()
{
	delete cubeMesh;
	delete sphereMesh;
	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void FlockingSim::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh", &cubeMesh);
	loadFunc("sphere.msh", &sphereMesh);
	loadFunc("goose.msh", &gooseMesh);

	basicTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitCamera();
	InitWorld();
}

void FlockingSim::UpdateGame(float dt)
{
	world->GetMainCamera()->UpdateCamera(dt);

	UpdateKeys();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);
	flock->UpdateFlock(dt);

	Debug::FlushRenderables();
	renderer->Render();
}

void FlockingSim::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	
}

void FlockingSim::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.5f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
}

void FlockingSim::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	//AddFloorToWorld(Vector3(0, -2, 0));
	flock = new FlockSystem();

	for (int i = 0; i < 50; i++)
	{
		CPUBoid* boid = new CPUBoid(0, 0);

		SphereVolume* volume = new SphereVolume(1.0f);
		boid->SetBoundingVolume((CollisionVolume*)volume);

		boid->GetTransform().SetWorldScale(Vector3(1, 1, 1));

		boid->GetTransform().SetWorldPosition(Vector3(rand() % 80, rand() % 10, rand() % 80));

		boid->SetRenderObject(new RenderObject(&boid->GetTransform(), gooseMesh, nullptr, basicShader));
		boid->SetPhysicsObject(new PhysicsObject(&boid->GetTransform(), boid->GetBoundingVolume()));

		boid->GetPhysicsObject()->SetInverseMass(1);
		boid->GetPhysicsObject()->InitSphereInertia();


		boid->GetPhysicsObject()->SetLinearVelocity(Vector3(rand() % 20, 0, rand() % 20));

		flock->AddBoid(boid);
		world->AddGameObject(boid);
	}
}

GameObject* FlockingSim::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(100, 2, 100);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform().SetWorldScale(floorSize);
	floor->GetTransform().SetWorldPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}