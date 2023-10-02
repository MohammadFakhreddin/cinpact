#pragma once
#include <memory>

#include "BedrockPath.hpp"
#include "BufferTracker.hpp"
#include "LogicalDevice.hpp"
#include "UI.hpp"
#include "camera/PerspectiveCamera.hpp"
#include "pipeline/LinePipeline.hpp"
#include "pipeline/PointPipeline.hpp"
#include "render_pass/DisplayRenderPass.hpp"
#include "render_resource/DepthRenderResource.hpp"
#include "render_resource/MSAA_RenderResource.hpp"
#include "render_resource/SwapChainRenderResource.hpp"
#include "utils/LineRenderer.hpp"
#include "utils/PointRenderer.hpp"

class CinpactApp
{
public:

	explicit CinpactApp();

	void Run();

	~CinpactApp();

private:

	struct ControlPointInfo
	{
		int idx = 0;
		std::string name{};
		glm::dvec3 position{};
		float k{};
		float c{};

		bool isOpenInTree = false;
	};

	enum class Mode
	{
		Add,
		Move,
		Edit,
		Remove
	};

	void Update();

	void Render(MFA::RT::CommandRecordState & recordState);

	void OnUI();

	void OnSDL_Event(SDL_Event* event);

	ControlPointInfo * GetClickedControlPoint(glm::vec2 const & mousePos);

	// Render parameters
	std::shared_ptr<MFA::Path> path{};
	std::shared_ptr<MFA::LogicalDevice> device{};
	std::shared_ptr<MFA::UI> ui{};
	std::shared_ptr<MFA::SwapChainRenderResource> swapChainResource{};
	std::shared_ptr<MFA::DepthRenderResource> depthResource{};
	std::shared_ptr<MFA::MSSAA_RenderResource> msaaResource{};
	std::shared_ptr<MFA::DisplayRenderPass> displayRenderPass{};

	std::shared_ptr<MFA::RT::BufferGroup> cameraBuffer{};
	std::shared_ptr<MFA::HostVisibleBufferTracker<glm::mat4>> cameraBufferTracker{};

	std::shared_ptr<MFA::LinePipeline> linePipeline{};
	std::shared_ptr<MFA::LineRenderer> lineRenderer{};

	std::shared_ptr<MFA::PointPipeline> pointPipeline{};
	std::shared_ptr<MFA::PointRenderer> pointRenderer{};

	std::vector<ControlPointInfo> cps{};		// Control points

	ControlPointInfo* selectedCP{};
	
	const glm::vec4 DefaultCP_Color{ 1.0, 0.0, 0.0, 1.0 };
	const glm::vec4 ActiveTreeCP_Color{ 1.0, 1.0, 0.0, 1.0 };
	const glm::vec4 SelectedCP_Color{ 0.0, 1.0, 0.0, 1.0 };

	float deltaU = 1e-4f;
	float defaultK = 1.0f;
	float defaultC = 1.0f;

	Mode mode = Mode::Add;
	bool leftMouseDown = false;
	bool rightMouseDown = false;

	int nextCpIdx = 0;

	bool curveChanged = false;
	std::vector<glm::vec3> curvePoints{};

};
