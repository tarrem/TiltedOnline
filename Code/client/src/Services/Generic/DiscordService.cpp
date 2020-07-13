#include <Services/DiscordService.h>
#include <discord.h>

#include <World.h>
#include <Games/Skyrim/Forms/TESObjectCELL.h>
#include <Games/Skyrim/Forms/TESWorldSpace.h>
#include <Games/Skyrim/PlayerCharacter.h>

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

DiscordService::DiscordService(World& aWorld, entt::dispatcher& aDispatcher, ImguiService& aImguiService)
    : m_dispatcher(aDispatcher)
    , m_world(aWorld)
{

    m_updateConnection = m_dispatcher.sink<UpdateEvent>().connect<&DiscordService::OnUpdate>(this);
    //m_connectedConnection = m_dispatcher.sink<ConnectedEvent>().connect<&DiscordService::OnConnected>(this);
    //m_disconnectedConnection = m_dispatcher.sink<DisconnectedEvent>().connect<&DiscordService::OnDisconnected>(this);
    m_drawImguiConnection = aImguiService.OnDraw.connect<&DiscordService::OnDraw>(this);

    // Initialize Discord SDK
    Result result = Core::Create(mc_clientId, (uint64_t)CreateFlags::NoRequireDiscord, &m_discord);
    spdlog::info("Discord initialization result: " + std::to_string((int)result));
    m_discord->ActivityManager().ClearActivity([](Result result) {});
    // Initial Activity update
    time_t now;
    time(&now);
    m_activity = new Activity();
    m_activity->SetApplicationId(731895579151433788);
    m_activity->SetName("Skyrim: Together");
    m_activity->SetDetails("In Main Menu");
    m_activity->GetParty().GetSize().SetCurrentSize(1);
    m_activity->GetParty().GetSize().SetMaxSize(8);
    m_activity->SetState(m_disconnectedState.c_str());
    m_activity->GetAssets().SetLargeImage("sktlogobig");
    m_activity->GetAssets().SetLargeText("In Skyrim"); // TODO randomized flavor text?
    m_activity->SetInstance(true);
    m_activity->GetTimestamps().SetStart(now);
    m_discord->ActivityManager().UpdateActivity(*m_activity, [](Result result) {
        if ((int)result != 0) spdlog::info("Discord update result: " + std::to_string((int)result));
        });
    // Setup logging
    // TODO Parse JSON msg
    m_discord->SetLogHook(LogLevel::Info, [](LogLevel level, const char* msg) {
        spdlog::info("[Discord] " + std::to_string((int)level) + ' ' + std::string(msg));
        });
    m_discord->RunCallbacks();
}

void DiscordService::OnUpdate(const UpdateEvent& acUpdateEvent) noexcept
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
            m_activity->SetDetails(worldSpace->GetName());
        }
        else if (parentCell) 
        {
            m_activity->SetDetails(parentCell->GetName());
        }
    }

    //m_activity->GetParty().GetSize().SetCurrentSize();

    m_discord->ActivityManager().UpdateActivity(*m_activity, [](Result result) {
        if ((int)result != 0) spdlog::info("Discord update result: " + std::to_string((int)result));
        });
    m_discord->RunCallbacks();
}

void DiscordService::OnConnected(const ConnectedEvent& acConnectedEvent) noexcept 
{
    m_activity->SetState(m_connectedState.c_str());
    //m_activity->GetParty().GetSize().SetMaxSize();
}

void DiscordService::OnDisconnected(const DisconnectedEvent& acDisconnectedEvent) noexcept 
{
    m_activity->SetState(m_disconnectedState.c_str());
    m_activity->GetParty().GetSize().SetMaxSize(8);
}

void DiscordService::OnDraw() noexcept 
{
    ImGui::Begin("Discord");

    ImGui::Text("Application ID: %" PRId64, m_activity->GetApplicationId());
    ImGui::Text("Name: %s", m_activity->GetName());
    ImGui::Text("State: %s", m_activity->GetState());
    ImGui::Text("Details: %s", m_activity->GetDetails());
    ImGui::Text("Timestamps: %d, %d", m_activity->GetTimestamps().GetStart(), m_activity->GetTimestamps().GetEnd());
    ImGui::Text("Join Secret: %d", m_activity->GetSecrets().GetJoin());
    ImGui::Text("Match Secret: %d", m_activity->GetSecrets().GetMatch());
    ImGui::Text("Spectate Secret: %d", m_activity->GetSecrets().GetSpectate());

    ImGui::End();
}
