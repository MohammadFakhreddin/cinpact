#include "CinpactApp.hpp"

#include "CinpactCurve.hpp"

using namespace MFA;

static constexpr float DefaultZ = 0.5f;

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

    cameraBufferTracker = std::make_shared<HostVisibleBufferTracker<glm::mat4>>(cameraBuffer, glm::identity<glm::mat4>());

    device->ResizeEventSignal2.Register([this]()->void {
        cameraBufferTracker->SetData(glm::identity<glm::mat4>());
    });

    linePipeline = std::make_shared<LinePipeline>(displayRenderPass, cameraBuffer, 10000);
    pointPipeline = std::make_shared<PointPipeline>(displayRenderPass, cameraBuffer, 10000);

    pointRenderer = std::make_shared<PointRenderer>(pointPipeline);

    device->SDL_EventSignal.Register([&](SDL_Event* event)->void
    {
        OnSDL_Event(event);
    });
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

        Update();

        auto recordState = device->AcquireRecordState(swapChainResource->GetSwapChainImages().swapChain);
        if (recordState.isValid == true)
        {
            device->BeginCommandBuffer(
                recordState,
                RT::CommandBufferType::Graphic
            );

            cameraBufferTracker->Update(recordState);

            if (curveBufferNeedUpdate == true && curvePoints.empty() == false)
            {
                curveBufferNeedUpdate = false;

                auto const blob = MFA::Alias{ curvePoints.data(), curvePoints.size() };
                
                if (curveVertices == nullptr || curveVertices->size < blob.Len())
                {
                    stageBuffer = RB::CreateStageBuffer(
                        LogicalDevice::Instance->GetVkDevice(),
                        LogicalDevice::Instance->GetPhysicalDevice(),
                        curvePoints.size() * sizeof(curvePoints[0]),
                        1
                    );
                    curveVertices = RB::CreateVertexBuffer(
                        LogicalDevice::Instance->GetVkDevice(),
                        LogicalDevice::Instance->GetPhysicalDevice(),
                        recordState.commandBuffer,
                        *stageBuffer->buffers[0],
                        blob
                    );
                }
                else
                {
                    RB::UpdateHostVisibleBuffer(
                        LogicalDevice::Instance->GetVkDevice(),
                        *stageBuffer->buffers[0],
                        blob
                    );
                    RB::UpdateLocalBuffer(
                        recordState.commandBuffer,
                        *curveVertices,
                        *stageBuffer->buffers[0]
                    );
                }
            }

            displayRenderPass->Begin(recordState);

            Render(recordState);

        	ui->Render(recordState, deltaTimeSec);

            displayRenderPass->End(recordState);

            device->EndCommandBuffer(recordState);

            device->SubmitQueues(recordState);

            device->Present(recordState, swapChainResource->GetSwapChainImages().swapChain);
        }

        deltaTimeMs = std::max<uint32_t>(SDL_GetTicks() - startTime, static_cast<uint32_t>(1000.0f / 240.0f));
        startTime = SDL_GetTicks();
        deltaTimeSec = static_cast<float>(deltaTimeMs) / 1000.0f;
    }

    device->DeviceWaitIdle();
}

//-----------------------------------------------------

CinpactApp::~CinpactApp()
{
    stageBuffer.reset();
    curveVertices.reset();
    pointRenderer.reset();
    linePipeline.reset();
    pointPipeline.reset();
    cameraBufferTracker.reset();
	cameraBuffer.reset();
    ui.reset();
    displayRenderPass.reset();
    swapChainResource.reset();
    depthResource.reset();
    msaaResource.reset();
    device.reset();
    path.reset();
}

//-----------------------------------------------------

void CinpactApp::Update()
{
    if (mode == Mode::Move && leftMouseDown == true && selectedCP != nullptr)
    {
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        auto const screen = device->GetSurfaceCapabilities().currentExtent;
        auto const mousePos = Math::ScreenSpaceToProjectedSpace({ mx, my }, screen.width, screen.height);
        selectedCP->position = glm::vec3{ mousePos, DefaultZ };
        curveChanged = true;
    }
    else if (curveChanged == true)
    {
        curveChanged = false;
        curveBufferNeedUpdate = true;

        std::vector<glm::vec3> controlPoints(cps.size());
        std::vector<float> cConstants(cps.size());
        std::vector<float> kConstants(cps.size());
    	for (int i = 0; i < static_cast<int>(cps.size()); ++i)
    	{
            controlPoints[i] = cps[i].position;
            kConstants[i] = cps[i].k;
            cConstants[i] = cps[i].c;
    	}

        curvePoints = Cinpact::Generate(interpolate, controlPoints, cConstants, kConstants, deltaU);
    }
}

//-----------------------------------------------------

void CinpactApp::Render(MFA::RT::CommandRecordState& recordState)
{
    for (auto const& cp : cps)
    {
        if (selectedCP != nullptr && selectedCP->idx == cp.idx)
        {
            pointRenderer->Draw(recordState, selectedCP->position, SelectedCP_Color);
        }
        else if (cp.isOpenInTree == true)
        {
            pointRenderer->Draw(recordState, cp.position, ActiveTreeCP_Color);
        }
        else
        {
            pointRenderer->Draw(recordState, cp.position, DefaultCP_Color);
        }
    }
    if (curvePoints.empty() == false)
    {
        linePipeline->BindPipeline(recordState);
        linePipeline->SetPushConstants(
            recordState, 
            LinePipeline::PushConstants{
                .model = glm::identity<glm::mat4>(),
                .color = glm::vec4{0.0f, 1.0f, 1.0f, 1.0f}
			}
        );
        RB::BindVertexBuffer(recordState, *curveVertices);
        vkCmdDraw(
			recordState.commandBuffer,
            curvePoints.size(),
            1,
            0,
            0
        );
    }
}

//-----------------------------------------------------

void CinpactApp::OnUI()
{
    ui->BeginWindow("Settings");

    ImGui::Text("Operations");

    if (ImGui::Checkbox("Interpolate", &interpolate))
    {
        curveChanged = true;
    }

    if (ImGui::RadioButton("Add", mode == Mode::Add))
    {
        mode = Mode::Add;
    }

	ImGui::SameLine();
    if (ImGui::RadioButton("Move", mode == Mode::Move))
    {
        mode = Mode::Move;
    }

    ImGui::SameLine();
    if (ImGui::RadioButton("Edit", mode == Mode::Edit))
    {
        mode = Mode::Edit;
    }

    ImGui::SameLine();
	if (ImGui::RadioButton("Remove", mode == Mode::Remove))
    {
        mode = Mode::Remove;
    }

    ImGui::Spacing();

    if (ImGui::Button("Remove all control points"))
    {
        cps.clear();
        curveChanged = true;
    }

    ImGui::Spacing();

    //ImGui::InputFloat("Delta U", &deltaU);

	ImGui::InputFloat("Default K", &defaultK);

    ImGui::InputFloat("Default C", &defaultC);

    if (selectedCP != nullptr && ImGui::TreeNode("Selected point: %s", selectedCP->name.c_str()))
    {
        curveChanged |= ImGui::InputFloat("K", &selectedCP->k);
        curveChanged |= ImGui::InputFloat("C", &selectedCP->c);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("All points"))
    {
        for (auto & cp : cps)
        {
	        if (ImGui::TreeNode(cp.name.c_str()))
	        {
                curveChanged |= ImGui::InputFloat("K", &cp.k);
                curveChanged |= ImGui::InputFloat("C", &cp.c);
                ImGui::TreePop();
                cp.isOpenInTree = true;
	        }
	        else
	        {
                cp.isOpenInTree = false;
	        }
        }
        ImGui::TreePop();
    }
    ui->EndWindow();
}

//-----------------------------------------------------

void CinpactApp::OnSDL_Event(SDL_Event* event)
{
    if (ui->HasFocus() == true)
    {
        return;
    }

    if (event->button.button == SDL_BUTTON_LEFT)
    {
        if (event->type == SDL_MOUSEBUTTONDOWN)
        {
            leftMouseDown = true;

            int mx, my;
            SDL_GetMouseState(&mx, &my);

            auto const screen = device->GetSurfaceCapabilities().currentExtent;
            auto const mousePos = Math::ScreenSpaceToProjectedSpace({ mx, my }, screen.width, screen.height);

            selectedCP = GetClickedControlPoint(mousePos);

            switch (mode)
            {
	            case Mode::Add:
		        {
	                if (selectedCP == nullptr)
	                {
	                    auto& newControlPoint = cps.emplace_back();
                        newControlPoint.idx = nextCpIdx++;
	                    newControlPoint.name = "Point" + std::to_string(newControlPoint.idx);
	                    newControlPoint.position = glm::vec3{ mousePos, DefaultZ };
                        newControlPoint.c = defaultC;
	                    newControlPoint.k = defaultK;
	                    selectedCP = &newControlPoint;
                        curveChanged = true;
	                }
	            }
	            break;
	            case Mode::Move:
                break;
	            case Mode::Edit:
	                break;
	            case Mode::Remove:
	            {
                    if (selectedCP != nullptr)
                    {
                        for (int i = static_cast<int>(cps.size() - 1); i >= 0; --i)
                        {
                            if (cps[i].idx == selectedCP->idx)
                            {
                                cps.erase(cps.begin() + i);
                                break;
                            }
                        }
                        selectedCP = nullptr;
                        curveChanged = true;
                    }
	            }
	            break;
            }
        }
        else if (event->type == SDL_MOUSEBUTTONUP)
        {
            leftMouseDown = false;
        }
    }
    else if (event->button.button == SDL_BUTTON_RIGHT)
    {
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        if (event->type == SDL_MOUSEBUTTONDOWN)
        {
            rightMouseDown = true;
        }
        else if (event->type == SDL_MOUSEBUTTONUP)
        {
            rightMouseDown = false;
        }
    }
}

//-----------------------------------------------------

CinpactApp::ControlPointInfo* CinpactApp::GetClickedControlPoint(glm::vec2 const& mousePos)
{
    ControlPointInfo* selectedControlPoint = nullptr;
    float selectedDistance = 0.0f;
    for (auto& cp : cps)
    {
        auto const distance = glm::length2(glm::vec2{ cp.position } - mousePos);
        if ((selectedControlPoint == nullptr && distance < 1e-3f) || distance < selectedDistance)
        {
            selectedControlPoint = &cp;
            selectedDistance = distance;
        }
    }

    return selectedControlPoint;
}

//-----------------------------------------------------
