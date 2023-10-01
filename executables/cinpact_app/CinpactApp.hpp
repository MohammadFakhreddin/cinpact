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

class CinpactApp
{
public:

	explicit CinpactApp();

	void Run();

	~CinpactApp();

private:

	void OnUI();

	std::shared_ptr<MFA::Path> path{};
	std::shared_ptr<MFA::LogicalDevice> device{};
	std::shared_ptr<MFA::UI> ui{};
	std::shared_ptr<MFA::SwapChainRenderResource> swapChainResource{};
	std::shared_ptr<MFA::DepthRenderResource> depthResource{};
	std::shared_ptr<MFA::MSSAA_RenderResource> msaaResource{};
	std::shared_ptr<MFA::DisplayRenderPass> displayRenderPass{};

	std::shared_ptr<MFA::RT::BufferGroup> cameraBuffer{};
	std::shared_ptr<MFA::PerspectiveCamera> camera{};
	std::shared_ptr<MFA::HostVisibleBufferTracker<glm::mat4>> cameraBufferTracker{};

	std::shared_ptr<MFA::LinePipeline> linePipeline{};
	std::shared_ptr<MFA::PointPipeline> pointPipeline{};

	std::shared_ptr<MFA::RT::BufferAndMemory> vertexBuffer{};

};
