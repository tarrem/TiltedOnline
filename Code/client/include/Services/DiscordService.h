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

/* Handles Discord Integration (currently only Activity/Rich Presence) */
struct DiscordService
{
public:
    DiscordService(World& aWorld, entt::dispatcher& aDispatcher, ImguiService& aImguiService);

    void OnUpdate(const UpdateEvent&) noexcept;
    void OnConnected(const ConnectedEvent&) noexcept;
    void OnDisconnected(const DisconnectedEvent&) noexcept;
    void OnDraw() noexcept;

private:
    /* Discord specific */
    const int64_t mc_clientId = 731895579151433788; // TODO change to official app client ID
    discord::Core* m_discord;
    discord::Activity* m_activity;
    std::string m_connectedState = "Online";
    std::string m_disconnectedState = "Offline";

    entt::dispatcher& m_dispatcher;
    World& m_world;

    entt::scoped_connection m_updateConnection;
    entt::scoped_connection m_connectedConnection;
    entt::scoped_connection m_disconnectedConnection;
    entt::scoped_connection m_drawImguiConnection;
};
