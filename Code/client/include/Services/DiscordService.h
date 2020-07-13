#pragma once

namespace discord
{
    class Core;
    class Activity;
}
struct World;
struct UpdateEvent;
struct ConnectedEvent;
struct DisconnectedEvent;
struct ImguiService;

/* Handles Discord Activity (supercedes Rich Presence) */
struct DiscordService
{
public:
    DiscordService(World& aWorld, entt::dispatcher& aDispatcher, ImguiService& aImguiService);

    void OnUpdate(const UpdateEvent&) noexcept;
    void OnConnected(const ConnectedEvent&) noexcept;
    void OnDisconnected(const DisconnectedEvent&) noexcept;
    void OnDraw() noexcept;

private:
    // Discord stuff

    // TODO change to production app client ID
    const int64_t mc_clientId = 731895579151433788;
    discord::Core* m_discord; // Discord SDK instance
    discord::Activity* m_activity; // Current activity status
    char* m_connectedState = "Online";
    char* m_disconnectedState = "Offline";

    entt::dispatcher& m_dispatcher;
    World& m_world;

    entt::scoped_connection m_updateConnection;
    entt::scoped_connection m_connectedConnection;
    entt::scoped_connection m_disconnectedConnection;
    entt::scoped_connection m_drawImguiConnection;
};
