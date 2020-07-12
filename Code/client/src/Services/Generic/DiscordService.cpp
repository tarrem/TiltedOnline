#include <Services/DiscordService.h>
#include <World.h>

DiscordService::DiscordService(World& aWorld, entt::dispatcher& aDispatcher)
    : m_dispatcher(aDispatcher)
    , m_world(aWorld)
{
    
}
