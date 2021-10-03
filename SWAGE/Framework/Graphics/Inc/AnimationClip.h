#pragma once

#include "Animation.h"

namespace SWAGE::Graphics
{
	struct AnimationClip
	{
		std::string name;
		float duration = 0.0f;
		float ticksPerSecond = 0.0f;
		std::vector<std::unique_ptr<Animation>> boneAnimations;
	};
}