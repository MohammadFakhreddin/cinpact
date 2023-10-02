#include "CinpactCurve.hpp"

#include "BedrockAssert.hpp"

#include <ext/scalar_constants.hpp>

//-----------------------------------------------------

std::vector<glm::vec3> Cinpact::Generate(std::vector<glm::vec3> const& controlPoints, std::vector<float> const& cConstants,
	std::vector<float> const& kConstants, float const deltaU)
{
	std::vector<glm::vec3> result{};

	for (float u = 0.0f; u <= 1.0f; u += deltaU)
	{
		std::vector<float> weights(controlPoints.size());
		float weightSum = 0.0f;
		for (int i = 0 ; i < static_cast<int>(weights.size()); ++i)
		{
			weights[i] = CalcA(u, static_cast<float>(i), cConstants[i], kConstants[i])
				* CalcI(u, static_cast<float>(i));
			weightSum += weights[i];
		}

		if (weightSum != 0.0f)
		{
			result.emplace_back();
			glm::vec3& value = result.back();
			for (int i = 0; i < static_cast<int>(controlPoints.size()); ++i)
			{
				value += weights[i] * controlPoints[i];
			}
			value /= weightSum;
		}
	}

	return result;
}

//-----------------------------------------------------

float Cinpact::CalcA(float const u, float const i, float const c, float const k)
{
	if (u < - c + i || u > c + i)
	{
		return 0.0f;
	}

	auto const uMinI = u - i;
	auto const uMinISquare = uMinI * uMinI;

	auto const bottom = (c * c) - uMinISquare;
	if (bottom == 0.0f)
	{
		return 0.0f;
	}

	auto const top = -k * uMinISquare;
	auto const pow = top / bottom;

	return std::exp(pow);
}

//-----------------------------------------------------

float Cinpact::CalcI(float const u, float const i)
{
	auto const uMinI = u - i;
	if (uMinI == 0.0f)
	{
		return 0.0f;
	}
	return std::sin(glm::pi<float>() * uMinI) / (uMinI * glm::pi<float>());
}

//-----------------------------------------------------