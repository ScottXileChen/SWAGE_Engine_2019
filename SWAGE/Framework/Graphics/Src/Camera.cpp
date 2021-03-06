#include "Precompiled.h"
#include "Camera.h"

#include "GraphicsSystem.h"

using namespace SWAGE;
using namespace SWAGE::Graphics;

void Camera::SetPosition(const Math::Vector3& position)
{
	mPosition = position;
}

void Camera::SetDirection(const Math::Vector3& direction)
{
	// Prevent setting direction straight up or down
	auto dir = Math::Normalize(direction);
	if (Math::Abs(Math::Dot(dir, Math::Vector3::YAxis)) < 0.995f)
		mDirection = dir;
}

void Camera::SetFov(float fov)
{
	constexpr float kMinFov = 10.0f * Math::Constants::DegToRad;
	constexpr float kMaxFov = 170.0f * Math::Constants::DegToRad;
	mFov = Math::Clamp(fov, kMinFov, kMaxFov);
}

void Camera::SetAspectRatio(float ratio)
{
	mAspectRatio = ratio;
}

void Camera::SetNearPlane(float nearPlane)
{
	mNearPlane = nearPlane;
}

void Camera::SetFarPlane(float farPlane)
{
	mFarPlane = farPlane;
}

void Camera::Walk(float distance)
{
	mPosition += mDirection * distance;
}

void Camera::Strafe(float distance)
{
	const Math::Vector3 right = Math::Normalize(Cross(Math::Vector3::YAxis, mDirection));
	mPosition += right * distance;
}

void Camera::Rise(float distance)
{
	mPosition += Math::Vector3::YAxis * distance;
}

void Camera::Yaw(float radian)
{
	Math::Matrix4 matRotate = Math::Matrix4::RotationY(radian);
	mDirection = Math::TransformNormal(mDirection, matRotate);
}

void Camera::Pitch(float radian)
{
	const Math::Vector3 right = Math::Normalize(Cross(Math::Vector3::YAxis, mDirection));
	const Math::Matrix4 matRot = Math::Matrix4::RotationAxis(right, radian);
	const Math::Vector3 newLook = Math::TransformNormal(mDirection, matRot);
	SetDirection(newLook);
}

void Camera::Zoom(float amount)
{
	constexpr float minZoom = 170.0f * Math::Constants::DegToRad;
	constexpr float maxZoom = 10.0f * Math::Constants::DegToRad;
	mFov = Math::Clamp(mFov - amount, maxZoom, minZoom);
}

const Math::Vector3& Camera::GetPosition() const
{
	return mPosition;
}

const Math::Vector3& Camera::GetDirection() const
{
	return mDirection;
}

Math::Matrix4 Camera::GetViewMatrix() const
{
	const Math::Vector3 l = mDirection;
	const Math::Vector3 r = Math::Normalize(Math::Cross(Math::Vector3::YAxis, l));
	const Math::Vector3 u = Math::Normalize(Math::Cross(l, r));
	const float x = -Math::Dot(r, mPosition);
	const float y = -Math::Dot(u, mPosition);
	const float z = -Math::Dot(l, mPosition);
	return {
		r.x, u.x, l.x, 0.0f,
		r.y, u.y, l.y, 0.0f,
		r.z, u.z, l.z, 0.0f,
		x,   y,   z,   1.0f
	};
}

Math::Matrix4 Camera::GetProjectionMatrix() const
{
	const float a = (mAspectRatio == 0.0f) ? GraphicsSystem::Get()->GetBackBufferAspectRatio() : mAspectRatio;
	const float h = 1.0f / tan(mFov * 0.5f);
	const float w = h / a;
	const float zf = mFarPlane;
	const float zn = mNearPlane;
	const float q = zf / (zf - zn);
	return {
		w,    0.0f, 0.0f,    0.0f,
		0.0f, h,    0.0f,    0.0f,
		0.0f, 0.0f, q,       1.0f,
		0.0f, 0.0f, -zn * q, 0.0f
	};
}

Math::Shapes::Ray Camera::ScreenPointToRay(int screenX, int screenY, uint32_t screenWidth, uint32_t screenHeight) const
{
	const float aspect = (float)screenWidth / (float)screenHeight;
	const float halfWidth = screenWidth * 0.5f;
	const float halfHeight = screenHeight * 0.5f;
	const float tanFOV = tanf(mFov * 0.5f);
	const float dx = tanFOV * ((float)screenX / halfWidth - 1.0f) * aspect;
	const float dy = tanFOV * (1.0f - (float)screenY / halfHeight);

	Math::Shapes::Ray ray;
	ray.org = Math::Vector3::Zero;
	ray.dir = Normalize(Math::Vector3(dx, dy, 1.0f));

	Math::Matrix4 invMatView = Inverse(GetViewMatrix());
	ray.org = TransformCoord(ray.org, invMatView);
	ray.dir = TransformNormal(ray.dir, invMatView);
	return ray;
}
