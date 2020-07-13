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

/* 
    Handles Discord Integration (currently only Activity/Rich Presence) 
    Represents a single Discord app
*/
struct DiscordService
{
public:
    DiscordService(World& aWorld, entt::dispatcher& aDispatcher, ImguiService& aImguiService);
    virtual ~DiscordService();

    void OnUpdate(const UpdateEvent&) noexcept;
    void OnConnected(const ConnectedEvent&) noexcept;
    void OnDisconnected(const DisconnectedEvent&) noexcept;
    void OnDraw() noexcept;

private:
    /* Discord specific */
    void UpdateActivity();
    const int64_t mc_clientId = 731895579151433788; // TODO change to official app client ID
    discord::Core* mp_discord;
    discord::Activity* mp_activity;

    entt::dispatcher& m_dispatcher;
    World& m_world;

    entt::scoped_connection m_updateConnection;
    entt::scoped_connection m_connectedConnection;
    entt::scoped_connection m_disconnectedConnection;
    entt::scoped_connection m_drawImguiConnection;
};
