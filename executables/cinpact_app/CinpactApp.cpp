#include "CinpactApp.hpp"

using namespace MFA;

//-----------------------------------------------------

CinpactApp::CinpactApp()
{
    MFA_LOG_DEBUG("Loading...");

    path = Path::Instantiate();

    LogicalDevice::InitParams params
    {
        .windowWidth = 800,
        .windowHeight = 600,
        .resizable = true,
        .fullScreen = false,
        .applicationName = "cinpact"
    };

    device = LogicalDevice::Instantiate(params);
    assert(device->IsValid() == true);

    
    swapChainResource = std::make_shared<SwapChainRenderResource>();
    depthResource = std::make_shared<DepthRenderResource>();
    msaaResource = std::make_shared<MSSAA_RenderResource>();
    displayRenderPass = std::make_shared<DisplayRenderPass>(
        swapChainResource,
        depthResource,
        msaaResource
    );

    ui = std::make_shared<UI>(displayRenderPass);
    ui->UpdateSignal.Register([this]()->void{ OnUI(); });

    cameraBuffer = RB::CreateHostVisibleUniformBuffer(
        device->GetVkDevice(),
        device->GetPhysicalDevice(),
        sizeof(glm::mat4),
        device->GetMaxFramePerFlight()
    );

    camera = std::make_shared<PerspectiveCamera>();

    cameraBufferTracker = std::make_shared<HostVisibleBufferTracker<glm::mat4>>(cameraBuffer, camera->GetViewProjection());

    device->ResizeEventSignal2.Register([this]()->void {
        cameraBufferTracker->SetData(camera->GetViewProjection());
    });

    linePipeline = std::make_shared<LinePipeline>(displayRenderPass, cameraBuffer, 10000);
    pointPipeline = std::make_shared<PointPipeline>(displayRenderPass, cameraBuffer, 10000);
}

//-----------------------------------------------------

void CinpactApp::Run()
{
    SDL_GL_SetSwapInterval(0);
    SDL_Event e;
    uint32_t deltaTimeMs = 1000.0f / 30.0f;
    float deltaTimeSec = static_cast<float>(deltaTimeMs) / 1000.0f;
    uint32_t startTime = SDL_GetTicks();

    bool shouldQuit = false;

    while (shouldQuit == false)
    {
        //Handle events
        while (SDL_PollEvent(&e) != 0)
        {
            //User requests quit
            if (e.type == SDL_QUIT)
            {
                shouldQuit = true;
            }
        }

        device->Update();

        ui->Update();

        auto recordState = device->AcquireRecordState(swapChainResource->GetSwapChainImages().swapChain);
        if (recordState.isValid == true)
        {
            device->BeginCommandBuffer(
                recordState,
                RT::CommandBufferType::Graphic
            );
            cameraBufferTracker->Update(recordState);

            displayRenderPass->Begin(recordState);

        	ui->Render(recordState, deltaTimeSec);

            displayRenderPass->End(recordState);

            device->EndCommandBuffer(recordState);

            device->SubmitQueues(recordState);

            device->Present(recordState, swapChainResource->GetSwapChainImages().swapChain);
        }

        deltaTimeMs = std::max<int>(SDL_GetTicks() - startTime, static_cast<int>(1000.0f / 120.0f));
        startTime = SDL_GetTicks();
        deltaTimeSec = static_cast<float>(deltaTimeMs) / 1000.0f;
    }

    device->DeviceWaitIdle();
}

//-----------------------------------------------------

CinpactApp::~CinpactApp()
{
    linePipeline.reset();
    pointPipeline.reset();
    cameraBufferTracker.reset();
	cameraBuffer.reset();
    camera.reset();
    ui.reset();
    displayRenderPass.reset();
    swapChainResource.reset();
    depthResource.reset();
    msaaResource.reset();
    device.reset();
    path.reset();
}

//-----------------------------------------------------

void CinpactApp::OnUI()
{
    ui->BeginWindow("Settings");
    ImGui::Text("placeholder");
	ui->EndWindow();
}

//-----------------------------------------------------
