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

	useGPU = true;
	useInstancing = true;

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
	loadFunc("rotatedGoose.msh", &gooseMesh);

	cylMesh = new OBJMesh(Assets::MESHDIR + "cylinder.obj");

	basicTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
	instanceShader = new OGLShader("InstanceVert.glsl", "GameTechFrag.glsl");


	InitCamera();
	InitWorld();
}

void FlockingSim::UpdateGame(float dt)
{
	world->GetMainCamera()->UpdateCamera(dt);

	UpdateKeys();

	if (!useGPU)
	{
		flock->UpdateFlock(dt);

		for (int i = 0; i < flock->GetSize(); i++)
		{
			flock->GetBoid(i)->GetTransform().SetWorldPosition(flock->GetBoid(i)->GetPos());
			float theta = flock->GetBoid(i)->Angle(flock->GetBoid(i)->GetVel());
			flock->GetBoid(i)->GetRenderObject()->GetTransform()->SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), theta));
		}
	}
	else
	{
		if (useInstancing)
			flock->UpdateInstanceFlock(dt);
		else
			flock->UpdateGPUFlock(dt);
	}
	

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

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
	
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::C)) {
		std::cout << "Pos: " << world->GetMainCamera()->GetPosition() << std::endl;
		std::cout << "Pitch: " << world->GetMainCamera()->GetPitch() << std::endl;
		std::cout << "Yaw: " << world->GetMainCamera()->GetYaw() << std::endl;
	}
}

void FlockingSim::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.5f);
	world->GetMainCamera()->SetFarPlane(1500.0f);
	world->GetMainCamera()->SetPitch(-30.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-300, 304, 331));
}

void FlockingSim::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	AddFloorToWorld(Vector3(0, -8, 0));
	flock = new FlockSystem();

	if (!useGPU)
	{
		for (int i = 0; i < 100; i++)
		{
			CPUBoid* boid = new CPUBoid(rand() % 200, rand() % 200, gooseMesh, basicShader);
			flock->AddBoid(boid);
			world->AddGameObject(boid);
		}

		InitObstacles();
	}
	else
	{
		if (!useInstancing)
		{
			for (int i = 0; i < 256; i++)
			{
				GPUBoid* boid = new GPUBoid(rand() % 200, rand() % 200, gooseMesh, basicShader);
				flock->AddBoid(boid);
				world->AddGameObject(boid);
			}
			
			InitObstacles();

			flock->InitGPU();
		}
		else
		{
			instanceGoose = AddGooseToWorld(Vector3(0, 0, 0));
			InitObstacles();
			flock->InitInstanceFlock(gooseMesh, instanceGoose->GetRenderObject());
		}
		
	}
	
}

void FlockingSim::InitObstacles()
{
	flock->AddObstacle(AddCylinderToWorld(Vector3(200, 0, 200)));
	flock->AddObstacle(AddCylinderToWorld(Vector3(-200, 0, -200)));
}

GameObject* FlockingSim::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize = Vector3(1000, 2, 1000);
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

GameObject* FlockingSim::AddGooseToWorld(const Vector3& position)
{
	float size = 10.0f;
	float inverseMass = 1.0f;

	GameObject* goose = new GameObject("Instance Goose", false);


	SphereVolume* volume = new SphereVolume(size);
	goose->SetBoundingVolume((CollisionVolume*)volume);

	goose->GetTransform().SetWorldScale(Vector3(size, size, size));
	goose->GetTransform().SetWorldPosition(position);

	goose->SetRenderObject(new RenderObject(&goose->GetTransform(), gooseMesh, nullptr, instanceShader));
	goose->GetRenderObject()->SetInstances(256);
	goose->GetRenderObject()->SetColour(Vector4(rand() % 2, rand() % 2, rand() % 2, 1));
	goose->SetPhysicsObject(new PhysicsObject(&goose->GetTransform(), goose->GetBoundingVolume()));

	goose->GetPhysicsObject()->SetInverseMass(inverseMass);
	goose->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(goose);

	return goose;
}

GameObject* FlockingSim::AddCylinderToWorld(const Vector3& position)
{
	Vector3 size = Vector3(50, 100, 50);
	GameObject* cyl = new GameObject();

	AABBVolume* volume = new AABBVolume(size);
	cyl->SetBoundingVolume((CollisionVolume*)volume);

	cyl->GetTransform().SetWorldScale(size);
	cyl->GetTransform().SetWorldPosition(position);

	cyl->SetRenderObject(new RenderObject(&cyl->GetTransform(), cylMesh, nullptr, basicShader));

	cyl->SetPhysicsObject(new PhysicsObject(&cyl->GetTransform(), cyl->GetBoundingVolume()));

	cyl->GetPhysicsObject()->SetInverseMass(0);
	cyl->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cyl);

	return cyl;
}