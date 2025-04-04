#ifndef __NPC_H__
#define __NPC_H__

#include <fb/game/object.h>

namespace fb { namespace game {

class npc : public object
{
public:
    LUA_PROTOTYPE

public:
    class model;

public:
    npc(fb::game::context& context, const model* model);
    npc(const npc& right);
    ~npc();

public:
    npc*                        make(fb::game::context& context);

public:
    static int                  builtin_input(lua_State* lua);
    static int                  builtin_menu(lua_State* lua);
    static int                  builtin_item(lua_State* lua);
    static int                  builtin_slot(lua_State* lua);
    static int                  builtin_sell(lua_State* lua);
    static int                  builtin_buy(lua_State* lua);
    static int                  builtin_repair(lua_State* lua);
    static int                  builtin_repair_all(lua_State* lua);
};

class npc::model : public object::model
{
public:
    LUA_PROTOTYPE

public:
    using pursuits = std::vector<std::pair<std::string, uint16_t>>;

public:
    interface listener;

    struct config : public fb::game::object::model::config
    {
    public:
        std::string                         script;
        std::map<std::string, uint16_t>     sell, buy;
    };

public:
    const std::string                       script;
    const std::map<std::string, uint16_t>   sell, buy;

public:
    friend class npc;

public:
    model(const fb::game::npc::model::config& config);
    ~model();

public:
    object::types               type() const { return object::types::NPC; }

public:
    static int                  builtin_input(lua_State* lua);
    static int                  builtin_menu(lua_State* lua);
    static int                  builtin_item(lua_State* lua);
    static int                  builtin_slot(lua_State* lua);
    static int                  builtin_sell(lua_State* lua);
    static int                  builtin_buy(lua_State* lua);
    static int                  builtin_repair(lua_State* lua);
    static int                  builtin_repair_all(lua_State* lua);
};


interface npc::model::listener : public virtual fb::game::object::listener
{ };

} }

#endif // !__NPC_H__
