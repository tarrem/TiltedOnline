#pragma once

struct World;

struct DiscordService
{
public:
    DiscordService(World& aWorld, entt::dispatcher& aDispatcher);

private:
    // TODO change to production app client ID
    const unsigned int m_clientId = 731895579151433788;

    entt::dispatcher& m_dispatcher;
    World& m_world;
};
