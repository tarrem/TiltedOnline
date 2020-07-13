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
    m_connectedConnection = m_dispatcher.sink<ConnectedEvent>().connect<&DiscordService::OnConnected>(this);
    m_disconnectedConnection = m_dispatcher.sink<DisconnectedEvent>().connect<&DiscordService::OnDisconnected>(this);
    m_drawImguiConnection = aImguiService.OnDraw.connect<&DiscordService::OnDraw>(this);

    // Initialize Discord SDK
    Result result = Core::Create(mc_clientId, (uint64_t)CreateFlags::NoRequireDiscord, &mp_discord);
    spdlog::info("Discord initialization result: {}", static_cast<int>(result));
    // Initial Activity update
    time_t now;
    time(&now);
    mp_activity = new Activity();
    mp_activity->SetApplicationId(731895579151433788);
    mp_activity->SetName("Skyrim: Together");
    mp_activity->SetDetails("In Main Menu");
    mp_activity->GetParty().GetSize().SetCurrentSize(1);
    mp_activity->GetParty().GetSize().SetMaxSize(8);
    mp_activity->SetState("Offline");
    mp_activity->GetAssets().SetLargeImage("sktlogobig");
    mp_activity->GetAssets().SetLargeText("In Skyrim"); // TODO randomized flavor text?
    mp_activity->SetInstance(true);
    mp_activity->GetTimestamps().SetStart(now);
    // Setup logging callback
    mp_discord->SetLogHook(LogLevel::Info, [](LogLevel level, const char* msg) {
        // TODO Parse JSON msg
        spdlog::info("[Discord] {} - {}", static_cast<int>(level), msg);
        });
    UpdateActivity();
}

DiscordService::~DiscordService() {
    delete mp_discord;
    delete mp_activity;
}

void DiscordService::UpdateActivity() {
    mp_discord->ActivityManager().UpdateActivity(*mp_activity, [](Result result) {});
    //mp_discord->RunCallbacks();
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
            mp_activity->SetDetails(worldSpace->GetName());
        }
        else if (parentCell) 
        {
            mp_activity->SetDetails(parentCell->GetName());
        }
    }

    //m_activity->GetParty().GetSize().SetCurrentSize();

    UpdateActivity();
    mp_discord->RunCallbacks();
}

void DiscordService::OnConnected(const ConnectedEvent& acConnectedEvent) noexcept 
{
    mp_activity->SetState("Online");
    //m_activity->GetParty().GetSize().SetMaxSize();
}

void DiscordService::OnDisconnected(const DisconnectedEvent& acDisconnectedEvent) noexcept 
{
    mp_activity->SetState("Offline");
    mp_activity->GetParty().GetSize().SetMaxSize(8);
}

void DiscordService::OnDraw() noexcept 
{
    ImGui::Begin("Discord");

    ImGui::Text("Application ID: %" PRId64, mp_activity->GetApplicationId());
    ImGui::Text("Name: %s", mp_activity->GetName());
    ImGui::Text("State: %s", mp_activity->GetState());
    ImGui::Text("Details: %s", mp_activity->GetDetails());
    ImGui::Text("Timestamps: %d, %d", mp_activity->GetTimestamps().GetStart(), mp_activity->GetTimestamps().GetEnd());
    ImGui::Text("Join Secret: %d", mp_activity->GetSecrets().GetJoin());
    ImGui::Text("Match Secret: %d", mp_activity->GetSecrets().GetMatch());
    ImGui::Text("Spectate Secret: %d", mp_activity->GetSecrets().GetSpectate());

    ImGui::End();
}
