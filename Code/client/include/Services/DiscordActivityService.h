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
    Handles Discord Activity
*/
struct DiscordActivityService
{
public:
    DiscordActivityService(World& aWorld, entt::dispatcher& aDispatcher, ImguiService& aImguiService);
    virtual ~DiscordActivityService();

    void OnUpdate(const UpdateEvent&) noexcept;
    void OnConnected(const ConnectedEvent&) noexcept;
    void OnDisconnected(const DisconnectedEvent&) noexcept;
    void OnDraw() noexcept;

private:
    /* Discord specific */
    int64_t m_clientId; // App's Client ID from https://discord.com/developers/applications/
    discord::Core* m_pDiscord; // Discord SDK app instance
    discord::Activity* m_pActivity; // Keeps track of information to display on Rich Presence
    void UpdateActivity();

    entt::dispatcher& m_dispatcher;
    World& m_world;

    entt::scoped_connection m_updateConnection;
    entt::scoped_connection m_connectedConnection;
    entt::scoped_connection m_disconnectedConnection;
    entt::scoped_connection m_drawImguiConnection;
};
