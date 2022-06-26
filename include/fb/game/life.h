#ifndef __LIFE_H__
#define __LIFE_H__

#include <fb/game/object.h>

namespace fb { namespace game {

class life : public object
{
#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

#pragma region forward nested declaration
public:
    interface listener;

public:
    class master;
#pragma endregion

#pragma region structure
struct config : public fb::game::object::config
{
public:
    uint32_t                    hp  = 0;
    uint32_t                    mp  = 0;
    uint32_t                    exp = 0;
};
#pragma endregion

#pragma region protected field
protected:
    uint32_t                    _hp = 0, _mp = 0;
    fb::game::condition         _condition = fb::game::condition::NONE;
#pragma endregion

#pragma region public field
public:
    fb::game::spells            spells;
#pragma endregion

#pragma region constructor / destructor
protected:
    life(fb::game::context& context, const master* master, const fb::game::life::config& config);
    virtual ~life();
#pragma endregion

#pragma region protected method
protected:
    uint32_t                    calculate_damage(uint32_t value, const fb::game::life& life) const;
#pragma endregion

#pragma region public method
public:
    virtual void                attack();
    virtual uint32_t            hp() const;
    virtual void                hp(uint32_t value);

    virtual uint32_t            mp() const;
    virtual void                mp(uint32_t value);

    virtual uint32_t            base_hp() const;
    virtual uint32_t            base_mp() const;

    virtual uint32_t            experience() const;

    virtual uint32_t            defensive_physical() const;
    virtual uint32_t            defensive_magical() const;

    virtual uint32_t            hp_up(uint32_t value, fb::game::object* from = nullptr);
    virtual uint32_t            hp_down(uint32_t value, fb::game::object* from = nullptr, bool critical = false);
    
    virtual uint32_t            mp_up(uint32_t value, fb::game::object* from = nullptr);
    virtual uint32_t            mp_down(uint32_t value, fb::game::object* from = nullptr);

    fb::game::condition         condition() const;
    fb::game::condition         condition_add(fb::game::condition value);
    fb::game::condition         condition_remove(fb::game::condition value);
    bool                        condition_contains(fb::game::condition value) const;

    virtual bool                alive() const;
    void                        kill();

    bool                        active(fb::game::spell& spell);
    bool                        active(fb::game::spell& spell, uint32_t fd);
    bool                        active(fb::game::spell& spell, const std::string& message);
    bool                        active(fb::game::spell& spell, fb::game::object& to);
#pragma endregion

#pragma region virtual method
protected:
    virtual void                handle_update() { }
    virtual uint32_t            handle_calculate_damage(bool critical) const = 0;
    virtual bool                handle_calculate_critical(fb::game::life& you) const;
    virtual bool                handle_calculate_miss(fb::game::life& you) const;
    virtual void                handle_attack(fb::game::object* you);
    virtual void                handle_hit(fb::game::life& you, uint32_t damage, bool critical);
    virtual void                handle_damaged(fb::game::object* from, uint32_t damage, bool critical);
    virtual void                handle_die(fb::game::object* from);

public:
    virtual uint32_t            handle_exp() const { return 0; }
    virtual void                handle_kill(fb::game::life& you);

#pragma endregion

#pragma region built-in method
public:
    static int                  builtin_hp(lua_State* lua);
    static int                  builtin_mp(lua_State* lua);
    static int                  builtin_base_hp(lua_State* lua);
    static int                  builtin_base_mp(lua_State* lua);
    static int                  builtin_hp_inc(lua_State* lua);
    static int                  builtin_hp_dec(lua_State* lua);
    static int                  builtin_mp_inc(lua_State* lua);
    static int                  builtin_mp_dec(lua_State* lua);
    static int                  builtin_action(lua_State* lua);
    static int                  builtin_spell(lua_State* lua);
    static int                  builtin_damage(lua_State* lua);
    static int                  builtin_cast(lua_State* lua);
#pragma endregion
};

#pragma region interface
interface life::listener : public virtual fb::game::object::listener,
    public virtual fb::game::spell::listener
{
    virtual void                on_attack(life& me, object* you) = 0;
    virtual void                on_hit(life& me, life& you, uint32_t damage, bool critical) = 0;
    virtual void                on_kill(life& me, life& you) = 0;
    virtual void                on_damaged(life& me, object* you, uint32_t damage, bool critical) = 0;
    virtual void                on_die(life& me, object* you) = 0;

    virtual void                on_heal_hp(life& me, uint32_t value, fb::game::object* from) = 0;
    virtual void                on_heal_mp(life& me, uint32_t value, fb::game::object* from) = 0;
    virtual void                on_hp(life& me, uint32_t before, uint32_t current) = 0;
    virtual void                on_mp(life& me, uint32_t before, uint32_t current) = 0;
};
#pragma endregion

#pragma region master
class life::master : public fb::game::object::master
{
#pragma region structure
public:
    struct config : public fb::game::object::master::config
    {
    public:
        fb::game::defensive     defensive;
        uint32_t                hp  = 0;
        uint32_t                mp  = 0;
        uint32_t                exp = 0;
    };
#pragma endregion

#pragma region friend
public:
    friend class life;
#pragma endregion

#pragma region lua
public:
    LUA_PROTOTYPE
#pragma endregion

#pragma region protected field
public:
    const fb::game::defensive   defensive;
    const uint32_t              hp          = 0;
    const uint32_t              mp          = 0;
    const uint32_t              experience  = 0;
#pragma endregion

#pragma region constructor / destructor
public:
    master(const fb::game::life::master::config& config);
    virtual ~master();
#pragma endregion

#pragma region built-in method
public:
    static int                  builtin_hp(lua_State* lua);
    static int                  builtin_mp(lua_State* lua);
#pragma endregion
};
#pragma endregion

} }

#endif // !__LIFE_H__