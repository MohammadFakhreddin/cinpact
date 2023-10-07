#include "CinpactCurve.hpp"

#include "BedrockAssert.hpp"

#include <ext/scalar_constants.hpp>

#define USE_OMP

//-----------------------------------------------------

std::vector<glm::vec3> Cinpact::Generate(
	bool interpolate,
	std::vector<glm::vec3> const& controlPoints, 
	std::vector<float> const& cConstants,
	std::vector<float> const& kConstants, 
	float const deltaU
)
{
	std::vector<glm::vec3> result{};

#ifdef USE_OMP

	float stepCountF = static_cast<float>(controlPoints.size()) - 1.0f - 2.0f * deltaU;
	stepCountF /= deltaU;
	auto stepCount = static_cast<int>(std::ceil(stepCountF));
	stepCount = std::max(0, stepCount);

	result.resize(stepCount);
	std::vector<bool> isValid(stepCount);

	#pragma omp parallel for
	for (int k = 0; k < result.size(); ++k)
	{
		auto const u = (k * deltaU) + deltaU;
		std::vector<float> weights(controlPoints.size());
		float weightSum = 0.0f;
		for (int i = 0; i < static_cast<int>(weights.size()); ++i)
		{
			weights[i] = CalcA(u, static_cast<float>(i), cConstants[i], kConstants[i]);
			if (interpolate == true)
			{
				weights[i] *= CalcI(u, static_cast<float>(i));
			}
			weightSum += weights[i];
		}

		if (weightSum != 0.0f)
		{
			glm::vec3& value = result[k];
			for (int i = 0; i < static_cast<int>(controlPoints.size()); ++i)
			{
				value += weights[i] * controlPoints[i];
			}
			value /= weightSum;
		}

		isValid[k] = weightSum > 0.0f;
	}

	for (int i = static_cast<int>(result.size()) - 1; i >= 0; --i)
	{
		if (isValid[i] == false)
		{
			result.erase(result.begin() + i);
		}
	}

#else
	for (float u = deltaU; u <= static_cast<float>(controlPoints.size()) - 1.0 - deltaU; u += deltaU)
	{
		std::vector<float> weights(controlPoints.size());
		float weightSum = 0.0f;
		for (int i = 0; i < static_cast<int>(weights.size()); ++i)
		{
			weights[i] = CalcA(u, static_cast<float>(i), cConstants[i], kConstants[i]);
			if (interpolate == true)
			{
				weights[i] *= CalcI(u, static_cast<float>(i));
			}
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
#endif

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

	auto bottom = (c * c) - uMinISquare;
	if (bottom == 0.0f)
	{
		bottom += glm::epsilon<float>();
	}

	auto const top = -k * uMinISquare;
	auto const pow = top / bottom;

	return std::exp(pow);
}

//-----------------------------------------------------

float Cinpact::CalcI(float const u, float const i)
{
	auto const uMinI = u - i;
	auto bottom = uMinI * glm::pi<float>();
	if (bottom == 0.0f)
	{
		bottom += glm::epsilon<float>();
	}
	auto const top = std::sin(glm::pi<float>() * uMinI);
	return top / bottom;
}

//-----------------------------------------------------