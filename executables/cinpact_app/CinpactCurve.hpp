#pragma once

#include <vec3.hpp>
#include <vector>

namespace Cinpact
{
	std::vector<glm::vec3> Generate(
		std::vector<glm::vec3> const & controlPoints, 
		std::vector<float> const & cConstants, 
		std::vector<float> const & kConstants,
		float deltaU
	);

	float CalcA(float u, float i, float c, float k);

	float CalcI(float u, float i);
}
