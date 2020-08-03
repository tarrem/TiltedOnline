#include <Services/DiscordActivityService.h>
#include <discord.h>

#include <World.h>
#ifdef TP_SKYRIM
#include <Games/Skyrim/Forms/TESObjectCELL.h>
#include <Games/Skyrim/Forms/TESWorldSpace.h>
#include <Games/Skyrim/PlayerCharacter.h>
#elif TP_FALLOUT4
#include <Games/Fallout4/Forms/TESObjectCELL.h>
#include <Games/Fallout4/Forms/TESWorldSpace.h>
#include <Games/Fallout4/PlayerCharacter.h>
#endif

#include <Components.h>
#include <Events/ConnectedEvent.h>
#include <Events/DisconnectedEvent.h>
#include <Events/UpdateEvent.h>
#include <Services/ImguiService.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <imgui.h>
#include <inttypes.h>
#include <ctime>

using namespace discord;

DiscordActivityService::DiscordActivityService(World& aWorld, entt::dispatcher& aDispatcher, ImguiService& aImguiService)
    : m_dispatcher(aDispatcher)
    , m_world(aWorld)
{
    m_updateConnection = m_dispatcher.sink<UpdateEvent>().connect<&DiscordActivityService::OnUpdate>(this);
    m_connectedConnection = m_dispatcher.sink<ConnectedEvent>().connect<&DiscordActivityService::OnConnected>(this);
    m_disconnectedConnection = m_dispatcher.sink<DisconnectedEvent>().connect<&DiscordActivityService::OnDisconnected>(this);
    m_drawImguiConnection = aImguiService.OnDraw.connect<&DiscordActivityService::OnDraw>(this);


#ifdef TP_SKYRIM
    m_clientId = 731895579151433788;
#elif TP_FALLOUT4
    m_clientId = 732397217804976138;
#endif

    spdlog::info("[Discord] Initalizing Discord...");
    Result result = Core::Create(m_clientId, (uint64_t)CreateFlags::NoRequireDiscord, &m_pDiscord);
    if (result != Result::Ok) {
        spdlog::warn("[Discord] Initialization error, shutting down");
        m_world.unset<DiscordActivityService>();
        m_dispatcher.sink<UpdateEvent>().disconnect(this);
        m_dispatcher.sink<ConnectedEvent>().disconnect(this);
        m_dispatcher.sink<DisconnectedEvent>().disconnect(this);
        aImguiService.OnDraw.disconnect(this);
    }
    else
    {
        // Output Discord messages to reverse console
        m_pDiscord->SetLogHook(LogLevel::Debug, [](LogLevel level, const char* msg) {
            spdlog::info("[Discord] {} - {}", static_cast<int>(level), msg);
            });

        time_t now;
        time(&now);
        m_pActivity = new Activity();
#ifdef TP_SKYRIM
        m_pActivity->GetAssets().SetLargeImage("sktlogobig");
        m_pActivity->GetAssets().SetLargeText("In Skyrim");
#elif TP_FALLOUT4
        m_pActivity->GetAssets().SetLargeImage("fltlogobig");
        m_pActivity->GetAssets().SetLargeText("In Fallout");
#endif
        m_pActivity->SetApplicationId(m_clientId);
        m_pActivity->SetState("Offline");
        m_pActivity->SetDetails("In Main Menu");
        m_pActivity->GetTimestamps().SetStart(now);
        m_pActivity->GetParty().GetSize().SetCurrentSize(1);
        m_pActivity->GetParty().GetSize().SetMaxSize(1);
        m_pActivity->SetInstance(true);

        UpdateActivity();
        m_pDiscord->RunCallbacks();
    }
}

DiscordActivityService::~DiscordActivityService()
{
    delete m_pDiscord;
    delete m_pActivity;
}

void DiscordActivityService::UpdateActivity()
{
    m_pDiscord->ActivityManager().UpdateActivity(*m_pActivity, [](Result result) {
        if (result != Result::Ok) spdlog::info("[Discord] Activity update failure: {}", static_cast<int>(result));
        });
}

void DiscordActivityService::OnUpdate(const UpdateEvent& acUpdateEvent) noexcept
{
    const auto view = m_world.view<FormIdComponent>();
    if (!view.empty())
    {
        // Note: Player is either in a worldspace or a cell; when one is null, the other isn't
        const TESWorldSpace* worldSpace = PlayerCharacter::Get()->GetWorldSpace();
        const TESObjectCELL* parentCell = PlayerCharacter::Get()->GetParentCell();
        if (worldSpace)
        {
            m_pActivity->SetDetails(worldSpace->GetName());
        }
        else if (parentCell)
        {
            m_pActivity->SetDetails(parentCell->GetName());
        }
    }
    
    //m_pActivity->GetParty().GetSize().SetCurrentSize();

    UpdateActivity();
    m_pDiscord->RunCallbacks();

    spdlog::info("[Discord] Updating");
}

void DiscordActivityService::OnConnected(const ConnectedEvent& acConnectedEvent) noexcept
{
    m_pActivity->SetState("Online");
    m_pActivity->GetParty().GetSize().SetMaxSize(64);
}

void DiscordActivityService::OnDisconnected(const DisconnectedEvent& acDisconnectedEvent) noexcept
{
    m_pActivity->SetState("Offline");
    m_pActivity->GetParty().GetSize().SetMaxSize(1);
}

void DiscordActivityService::OnDraw() noexcept
{
    ImGui::Begin("Discord Activity");

    ImGui::Text("Players: %d", m_world.view<RemoteComponent, FormIdComponent, entt::exclude_t<LocalComponent>>().size());

    ImGui::Text("Application ID: %" PRId64, m_pActivity->GetApplicationId());
    ImGui::Text("Name: %s", m_pActivity->GetName());
    ImGui::Text("State: %s", m_pActivity->GetState());
    ImGui::Text("Details: %s", m_pActivity->GetDetails());
    ImGui::Text("Timestamps:");
    ImGui::Text("\tStart: %d", m_pActivity->GetTimestamps().GetStart());
    ImGui::Text("Assets:");
    ImGui::Text("\tLarge Image: %s", m_pActivity->GetAssets().GetLargeImage());
    ImGui::Text("\tLarge Text: %s", m_pActivity->GetAssets().GetLargeText());
    ImGui::Text("Party:");
    ImGui::Text("\tSize:");
    //ImGui::Text("\t\tCurrent: %s", m_pActivity->GetParty().GetSize().GetCurrentSize());
    //ImGui::Text("\t\\tMax: %s", m_pActivity->GetParty().GetSize().GetMaxSize());
    ImGui::Text("Instance: %d", m_pActivity->GetInstance());

    ImGui::End();
}
