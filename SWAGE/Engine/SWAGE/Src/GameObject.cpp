#include "Precompiled.h"
#include "GameObject.h"

#include "Component.h"

using namespace SWAGE;

MEMPOOL_DEFINE(GameObject, 1000)

void GameObject::Initialize()
{
	ASSERT(!mInitialized, "GameObject - Already initialized.");
	for (auto& component : mComponents)
		component->Initialize();
	mInitialized = true;
}

void GameObject::Terminate()
{
	for (auto& component : mComponents)
		component->Terminate();
}

void GameObject::Update(float deltaTime)
{
	for (auto& component : mComponents)
		component->Update(deltaTime);
}

void GameObject::DebugUI()
{
	for (auto& component : mComponents)
		component->DebugUI();
}
