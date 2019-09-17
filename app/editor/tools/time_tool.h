#pragma once

#include "../../libs/glfw/glfw3.h"

namespace tools {
	inline float Now(float offset = 0.0f)
	{
        return static_cast<float>(glfwGetTime()) + offset;
	}
}