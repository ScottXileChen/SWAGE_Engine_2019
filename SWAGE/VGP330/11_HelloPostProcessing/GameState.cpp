#include "GameState.h"

using namespace SWAGE::Graphics;
using namespace SWAGE::Input;
using namespace SWAGE::Math;

namespace
{
	bool specularMapOn = false;
	bool NormalMapOn = false;
	float displacementWeight = 0.0f;
	float postProcessRedLineOffset = 0.5f;
	float pw = 15.0f;
	float ph = 20.0f;;
}

void GameState::Initialize()
{
	auto graphicsSystem = GraphicsSystem::Get();
	graphicsSystem->SetClearColor(SWAGE::Graphics::Colors::Gray);

	mRenderTarget.Initialize(
		graphicsSystem->GetBackBufferWidth(),
		graphicsSystem->GetBackBufferHeight()
	);

	mSphereMesh = MeshBuilder::CreateSphere(256,256,1);
	mSphereBuffer.Initialize(mSphereMesh);

	mSkyBoxMesh = MeshBuilder::CreateSkybox(500.0f, true);
	mSkyBoxBuffer.Initialize(mSkyBoxMesh);
	mSkyBoxTexture.Initialize(L"../../Assets/Images/Space.png");

	mEarthCloudMesh = MeshBuilder::CreateSpherePNX(256, 256, 1.05f);
	mEarthCloudBuffer.Initialize(mEarthCloudMesh);
	mEarthCloudTexture.Initialize(L"../../Assets/Images/earth_clouds.jpg");

	mPostProcessParams.Initialize();
	mBlendState.Initialize(SWAGE::Graphics::BlendState::Mode::Additive);

	mCloudVertexShader.Initialize(L"../../Assets/Shaders/Earth_Cloud.fx", VertexPNX::Format);
	mCloudPixelShader.Initialize(L"../../Assets/Shaders/Earth_Cloud.fx");

	mTextureVertexShader.Initialize(L"../../Assets/Shaders/DoTexturing.fx", VertexPX::Format);
	mTexturePixelShader.Initialize(L"../../Assets/Shaders/DoTexturing.fx");

	mVertexShader.Initialize(L"../../Assets/Shaders/Earth.fx", Vertex::Format);
	mPixelShader.Initialize(L"../../Assets/Shaders/Earth.fx");

	mPostProcessVertexShader.Initialize(L"../../Assets/Shaders/PostProcess.fx", VertexPX::Format);
	mPostProcessPixelShader.Initialize(L"../../Assets/Shaders/PostProcess.fx");

	mSamplers[0].Initialize(Sampler::Filter::Linear, Sampler::AddressMode::Wrap);
	mSamplers[1].Initialize(Sampler::Filter::Anisotropic, Sampler::AddressMode::Wrap);
	mSamplers[2].Initialize(Sampler::Filter::Point, Sampler::AddressMode::Wrap);

	mTexture[0].Initialize("../../Assets/Images/earth.jpg");
	mTexture[1].Initialize("../../Assets/Images/White.png");
	mTexture[2].Initialize("../../Assets/Images/earth_spec.jpg");
	mTexture[3].Initialize("../../Assets/Images/earth_bump.jpg");
	mTexture[4].Initialize("../../Assets/Images/earth_normal.jpg");
	mTexture[5].Initialize("../../Assets/Images/earth_lights.jpg");

	// Create constant buffer
	mTransformBuffer.Initialize();
	mTransformBuffer2.Initialize();
	mLightBuffer.Initialize();
	mMaterialBuffer.Initialize();
	mSettingBuffer.Initialize();

	mPosition.z = 5.0f;

	mDirectionalLight.direction = { 0.577f, -0.577f, 0.577f };
	mDirectionalLight.ambient = { 0.2f,0.2f,0.2f,1.0f };
	mDirectionalLight.diffuse = { 0.9f,0.9f,0.9f,1.0f };
	mDirectionalLight.specular = { 0.2f,0.2f,0.2f,1.0f };

	mMaterial.ambient = { 0.2f,0.2f,0.2f,1.0f };
	mMaterial.diffuse = { 1.0f,1.0f,1.0f,1.0f };
	mMaterial.specular = { 0.5f,0.5f,0.5f,1.0f };
	mMaterial.power = 10;

	// NPC Space
	mScreenMesh.vertices.push_back({ {-1.0f,-1.0f,0.0f}, {0.0f,1.0f} });
	mScreenMesh.vertices.push_back({ {-1.0f,+1.0f,0.0f}, {0.0f,0.0f} });
	mScreenMesh.vertices.push_back({ {+1.0f,+1.0f,0.0f}, {1.0f,0.0f} });
	mScreenMesh.vertices.push_back({ {+1.0f,-1.0f,0.0f}, {1.0f,1.0f} });

	mScreenMesh.indices.push_back(0);
	mScreenMesh.indices.push_back(1);
	mScreenMesh.indices.push_back(2);

	mScreenMesh.indices.push_back(0);
	mScreenMesh.indices.push_back(2);
	mScreenMesh.indices.push_back(3);

	nScreenMeshBuffer.Initialize(mScreenMesh);
}

void GameState::Terminate()
{
	mPostProcessParams.Terminate();
	mRenderTarget.Terminate();
	mPostProcessVertexShader.Terminate();
	mPostProcessPixelShader.Terminate();
	mBlendState.Terminate();
	mTextureVertexShader.Terminate();
	mCloudVertexShader.Terminate();
	mTexturePixelShader.Terminate();
	mCloudPixelShader.Terminate();
	mSkyBoxTexture.Terminate();
	mSkyBoxBuffer.Terminate();
	mSphereBuffer.Terminate();
	mEarthCloudBuffer.Terminate();
	mEarthCloudTexture.Terminate();
	mTransformBuffer.Terminate();
	mTransformBuffer2.Terminate();
	mLightBuffer.Terminate();
	mMaterialBuffer.Terminate();
	mVertexShader.Terminate();
	mPixelShader.Terminate();
	mSettingBuffer.Terminate();
	for (size_t i = 0; i < std::size(mSamplers); i++)
		mSamplers[i].Terminate();
	for (size_t i = 0; i < std::size(mTexture); i++)
		mTexture[i].Terminate();
	nScreenMeshBuffer.Terminate();
}

void GameState::Update(float deltaTime)
{
	if (SWAGE::Input::InputSystem::Get()->IsKeyDown(SWAGE::Input::KeyCode::LEFT))
		mRoataion.y += deltaTime;
	if (SWAGE::Input::InputSystem::Get()->IsKeyDown(SWAGE::Input::KeyCode::RIGHT))
		mRoataion.y -= deltaTime;

	if (SWAGE::Input::InputSystem::Get()->IsKeyDown(SWAGE::Input::KeyCode::UP))
		mRoataion.x += deltaTime;
	if (SWAGE::Input::InputSystem::Get()->IsKeyDown(SWAGE::Input::KeyCode::DOWN))
		mRoataion.x -= deltaTime;

	const float moveSpeed = 10.0f;
	const float turnSpeed = 10.0f * Constants::DegToRad;
	if (SWAGE::Input::InputSystem::Get()->IsKeyDown(SWAGE::Input::KeyCode::W))
		mCamera.Walk(moveSpeed * deltaTime);
	if (SWAGE::Input::InputSystem::Get()->IsKeyDown(SWAGE::Input::KeyCode::S))
		mCamera.Walk(-moveSpeed * deltaTime);
	if (SWAGE::Input::InputSystem::Get()->IsKeyDown(SWAGE::Input::KeyCode::D))
		mCamera.Strafe(moveSpeed * deltaTime);
	if (SWAGE::Input::InputSystem::Get()->IsKeyDown(SWAGE::Input::KeyCode::A))
		mCamera.Strafe(-moveSpeed * deltaTime);
	if (SWAGE::Input::InputSystem::Get()->IsKeyDown(SWAGE::Input::KeyCode::Q))
		mCamera.Rise(moveSpeed * deltaTime);
	if (SWAGE::Input::InputSystem::Get()->IsKeyDown(SWAGE::Input::KeyCode::E))
		mCamera.Rise(-moveSpeed * deltaTime);
	if (SWAGE::Input::InputSystem::Get()->IsMouseDown(SWAGE::Input::MouseButton::RBUTTON))
	{
		mCamera.Yaw(SWAGE::Input::InputSystem::Get()->GetMouseMoveX() * turnSpeed * deltaTime);
		mCamera.Pitch(SWAGE::Input::InputSystem::Get()->GetMouseMoveY() * turnSpeed * deltaTime);
	}

	mCloudRoataion -= deltaTime * 0.1f;
	mFPS = 1.0f / deltaTime;
}

void GameState::Render()
{
	// Capture a screen shot.
	RenderScene();

	// Process screen shot before presenting.
	PostProcess();
}

void GameState::DebugUI()
{
	ImGui::Begin("Debug Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("FPS: %f", mFPS);

	const char* themeItems[] = { "Classic", "Dark","Light" };
	static int themeItemIndex = 0;
	if (ImGui::Combo("UI Theme", &themeItemIndex, themeItems, IM_ARRAYSIZE(themeItems)))
	{
		if (themeItemIndex == 0)
		{
			DebugUI::SetTheme(DebugUI::Theme::Classic);
		}
		else if (themeItemIndex == 1)
		{
			DebugUI::SetTheme(DebugUI::Theme::Dark);
		}
		else
		{
			DebugUI::SetTheme(DebugUI::Theme::Light);
		}
	}

	if (ImGui::CollapsingHeader("Light"))
	{
		if (ImGui::DragFloat3("Light Direction", &mDirectionalLight.direction.x, 0.01f))
			mDirectionalLight.direction = SWAGE::Math::Normalize(mDirectionalLight.direction);
		ImGui::ColorEdit3("Light Ambient",&mDirectionalLight.ambient.x);
		ImGui::ColorEdit3("Light Diffuse",&mDirectionalLight.diffuse.x);
		ImGui::ColorEdit3("Light Specular",&mDirectionalLight.specular.x);
	}

	if (ImGui::CollapsingHeader("Material"))
	{
		ImGui::ColorEdit3("Material Ambient", &mMaterial.ambient.x);
		ImGui::ColorEdit3("Material Diffuse", &mMaterial.diffuse.x);
		ImGui::ColorEdit3("Material Specular", &mMaterial.specular.x);
		ImGui::DragFloat("Material Power", &mMaterial.power);
	}

	if (ImGui::CollapsingHeader("Settings"))
	{
		ImGui::DragFloat("Displacement Weight", &displacementWeight, 0.1f);
		ImGui::Checkbox("Specular Map", &specularMapOn);
		ImGui::Checkbox("Normal Map", &NormalMapOn);
	}

	if (ImGui::CollapsingHeader("Transform"))
	{
		ImGui::DragFloat3("Rotation", (float*)&mRoataion, 0.01f);
	}
	ImGui::End();

	ImGui::Begin("Post Processing");
	ImGui::DragFloat("Red line offset", &postProcessRedLineOffset, 0.01f, 0.0f, 1.0f);
	if (ImGui::CollapsingHeader("Pixelation"))
	{
		ImGui::DragFloat("Width of a low resolution pixel", &pw, 1.0f, 0.0f, 100.0f);
		ImGui::DragFloat("Height of a low resolution pixel", &ph, 1.0f, 0.0f, 100.0f);
	}
	ImGui::End();
}

void GameState::RenderScene()
{
	mRenderTarget.BeginRender();

	SWAGE::Math::Matrix4 matView = mCamera.GetViewMatrix();
	SWAGE::Math::Matrix4 matProj = mCamera.GetProjectionMatrix();

	TransformData transformData;
	TransformData2 skyBoxtransformData2;

	mSamplers[1].BindVS(0);

	mTextureVertexShader.Bind();
	mTexturePixelShader.Bind();

	SWAGE::Math::Matrix4 matWorld = Matrix4::RotationX(20) * Matrix4::Translation(mCamera.GetPosition());
	skyBoxtransformData2.wvp = SWAGE::Math::Transpose(matWorld * matView * matProj);
	mTransformBuffer2.Update(skyBoxtransformData2);

	mTransformBuffer2.BindVS(0);
	mSkyBoxTexture.BindPS(0);

	mSkyBoxBuffer.Render();

	transformData.viewPosition = mCamera.GetPosition();
	mTransformBuffer.BindVS(0);
	mTransformBuffer.BindPS(0);

	mLightBuffer.Update(mDirectionalLight);
	mLightBuffer.BindVS(1);
	mLightBuffer.BindPS(1);

	mMaterialBuffer.Update(mMaterial);
	mMaterialBuffer.BindVS(2);
	mMaterialBuffer.BindPS(2);

	Setting setting;
	setting.displacementWeight = displacementWeight;
	setting.isNormalMapOn = NormalMapOn;
	mSettingBuffer.Update(setting);
	mSettingBuffer.BindVS(3);
	mSettingBuffer.BindPS(3);

	mVertexShader.Bind();
	mPixelShader.Bind();

	matWorld = SWAGE::Math::Matrix4::RotationX(mRoataion.x) * Matrix4::RotationY(mRoataion.y) * Matrix4::Translation(mPosition);
	transformData.world = SWAGE::Math::Transpose(matWorld);
	transformData.wvp = SWAGE::Math::Transpose(matWorld * matView * matProj);
	mTransformBuffer.Update(transformData);

	mTexture[0].BindPS(0);
	//mTexture[1].BindPS(1);
	mTexture[2].BindPS(1);
	mTexture[3].BindVS(2);
	mTexture[4].BindPS(3);
	mTexture[5].BindPS(4);

	mSphereBuffer.Render();

	// Cloud
	mCloudVertexShader.Bind();
	mCloudPixelShader.Bind();

	matWorld = SWAGE::Math::Matrix4::RotationX(mRoataion.x) * Matrix4::RotationY(mRoataion.y + mCloudRoataion) * Matrix4::Translation(mPosition);
	transformData.world = SWAGE::Math::Transpose(matWorld);
	transformData.wvp = SWAGE::Math::Transpose(matWorld * matView * matProj);
	mTransformBuffer.Update(transformData);
	mTransformBuffer.BindVS(0);
	mTransformBuffer.BindPS(0);

	mLightBuffer.Update(mDirectionalLight);
	mLightBuffer.BindVS(1);
	mLightBuffer.BindPS(1);

	mMaterialBuffer.Update(mMaterial);
	mMaterialBuffer.BindVS(2);
	mMaterialBuffer.BindPS(2);

	//mTransformBuffer2.BindVS(0);
	mEarthCloudTexture.BindPS(0);
	mBlendState.Set();

	mEarthCloudBuffer.Render();

	BlendState::ClearState();

	mRenderTarget.EndRender();
}

void GameState::PostProcess()
{
	mPostProcessVertexShader.Bind();
	mPostProcessPixelShader.Bind();

	mSamplers[0].BindPS(0);

	mRenderTarget.BindPS(0);

	PostProcessParams ppp;
	ppp.offset = postProcessRedLineOffset;
	ppp.screenWidth = static_cast<float>(GraphicsSystem::Get()->GetBackBufferWidth());
	ppp.screenHeight = static_cast<float>(GraphicsSystem::Get()->GetBackBufferHeight());
	ppp.pixel_h = ph;
	ppp.pixel_w = pw;
	mPostProcessParams.Update(ppp);
	mPostProcessParams.BindVS(0);
	mPostProcessParams.BindPS(0);

	nScreenMeshBuffer.Render();
	mRenderTarget.UnbindPS(0);
}

// Device interface - used for VRAM allocation/resource creation
// Context interface - used to issue draw commands