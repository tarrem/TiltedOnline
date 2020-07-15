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
    Result result = Core::Create(m_clientId, (uint64_t)CreateFlags::NoRequireDiscord, &mp_discord);
    spdlog::info("[Discord] Result: {}", static_cast<int>(result));

    time_t now;
    time(&now);
    mp_activity = new Activity();
#ifdef TP_SKYRIM
    mp_activity->GetAssets().SetLargeImage("sktlogobig");
    mp_activity->GetAssets().SetLargeText("In Skyrim");
#elif TP_FALLOUT4
    mp_activity->GetAssets().SetLargeImage("fltlogobig");
    mp_activity->GetAssets().SetLargeText("In Fallout");
#endif
    mp_activity->SetApplicationId(m_clientId);
    mp_activity->SetState("Offline");
    mp_activity->SetDetails("In Main Menu");
    mp_activity->GetTimestamps().SetStart(now);
    mp_activity->GetParty().GetSize().SetCurrentSize(1);
    mp_activity->GetParty().GetSize().SetMaxSize(1);
    mp_activity->SetInstance(true);

    // Output Discord messages to reverse console
    mp_discord->SetLogHook(LogLevel::Warn, [](LogLevel level, const char* msg) {
        spdlog::info("[Discord] {} - {}", static_cast<int>(level), msg);
        });

    UpdateActivity();
    mp_discord->RunCallbacks();
}

DiscordActivityService::~DiscordActivityService()
{
    delete mp_discord;
    delete mp_activity;
}

void DiscordActivityService::UpdateActivity()
{
    mp_discord->ActivityManager().UpdateActivity(*mp_activity, [](Result result) {
        if (result != Result::Ok) spdlog::info("[Discord] Activity update failure: {}", static_cast<int>(result));
        });
}

void DiscordActivityService::OnUpdate(const UpdateEvent& acUpdateEvent) noexcept
{
    const auto view = m_world.view<FormIdComponent>();
    if (!view.empty())
    {
        // Set rich presence details to player location
        // Note: Player is either in a worldspace or a cell; when one is null, the other isn't
        const TESWorldSpace* worldSpace = PlayerCharacter::Get()->GetWorldSpace();
        const TESObjectCELL* parentCell = PlayerCharacter::Get()->GetParentCell();
        if (worldSpace)
        {
            mp_activity->SetDetails(worldSpace->GetName());
        }
        else if (parentCell)
        {
            mp_activity->SetDetails(parentCell->GetName());
        }
    }
    
    //mp_activity->GetParty().GetSize().SetCurrentSize();

    UpdateActivity();
    mp_discord->RunCallbacks();
}

void DiscordActivityService::OnConnected(const ConnectedEvent& acConnectedEvent) noexcept
{
    mp_activity->SetState("Online");
    mp_activity->GetParty().GetSize().SetMaxSize(64);
}

void DiscordActivityService::OnDisconnected(const DisconnectedEvent& acDisconnectedEvent) noexcept
{
    mp_activity->SetState("Offline");
    mp_activity->GetParty().GetSize().SetMaxSize(1);
}

void DiscordActivityService::OnDraw() noexcept
{
    ImGui::Begin("Discord Activity");

    ImGui::Text("Players: %d", m_world.view<RemoteComponent, FormIdComponent, entt::exclude_t<LocalComponent>>().size());

    ImGui::Text("Application ID: %" PRId64, mp_activity->GetApplicationId());
    ImGui::Text("Name: %s", mp_activity->GetName());
    ImGui::Text("State: %s", mp_activity->GetState());
    ImGui::Text("Details: %s", mp_activity->GetDetails());
    ImGui::Text("Timestamps:");
    ImGui::Text("\tStart: %d", mp_activity->GetTimestamps().GetStart());
    ImGui::Text("Assets:");
    ImGui::Text("\tLarge Image: %s", mp_activity->GetAssets().GetLargeImage());
    ImGui::Text("\tLarge Text: %s", mp_activity->GetAssets().GetLargeText());
    ImGui::Text("Party:");
    ImGui::Text("\tSize:");
    //ImGui::Text("\t\tCurrent: %s", mp_activity->GetParty().GetSize().GetCurrentSize());
    //ImGui::Text("\t\\tMax: %s", mp_activity->GetParty().GetSize().GetMaxSize());
    ImGui::Text("Instance: %d", mp_activity->GetInstance());

    ImGui::End();
}
