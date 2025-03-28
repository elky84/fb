#include <fb/game/npc.h>
#include <fb/game/context.h>

using namespace fb::game;

fb::game::npc::model::model(const fb::game::npc::model::config& config) : 
    fb::game::object::model(config),
    script(config.script),
    sell(config.sell),
    buy(config.buy)
{ }

fb::game::npc::model::~model()
{ }