#include "fb_game.h"

using namespace fb::game;

IMPLEMENT_LUA_EXTENSION(fb::game::acceptor, "")
    {"name2mob", builtin_name2mob},
END_LUA_EXTENSION

acceptor::acceptor(uint16_t port) : fb_acceptor<fb::game::session>(port)
{
    lua::env<acceptor>("acceptor", this);

    lua::bind_class<object::core>();
    lua::bind_class<object>();
    
    lua::bind_class<life::core, object::core>();
    lua::bind_class<life, object>();
    
    lua::bind_class<mob::core, life::core>();
    lua::bind_class<mob, life>();

    lua::bind_class<npc::core, object::core>();
    lua::bind_class<npc, object>();

    lua::bind_class<game::session, life>();

    luaL_register(lua::main::get(), "fb", LUA_METHODS);


    this->register_handle(0x10, &acceptor::handle_login);               // ���Ӽ��� ���� �ڵ鷯
    this->register_handle(0x11, &acceptor::handle_direction);           // ������ȯ �ڵ鷯
    this->register_handle(0x06, &acceptor::handle_update_move);         // �̵��� �� ������ ������Ʈ �ڵ鷯
    this->register_handle(0x32, &acceptor::handle_move);                // �̵� �ڵ鷯
    this->register_handle(0x13, &acceptor::handle_attack);              // ���� �ڵ鷯
    this->register_handle(0x07, &acceptor::handle_pickup);              // ������ �ݱ� �ڵ鷯
    this->register_handle(0x1D, &acceptor::handle_emotion);             // ����ǥ�� �ڵ鷯
    this->register_handle(0x05, &acceptor::handle_update_map);          // �� ������ ������Ʈ �ڵ鷯
    this->register_handle(0x38, &acceptor::handle_refresh);             // ���ΰ�ħ �ڵ鷯
    this->register_handle(0x1C, &acceptor::handle_active_item);         // ������ ��� �ڵ鷯
    this->register_handle(0x1F, &acceptor::handle_inactive_item);       // ������ ���� ���� �ڵ鷯
    this->register_handle(0x08, &acceptor::handle_drop_item);           // ������ ������ �ڵ鷯
    this->register_handle(0x24, &acceptor::handle_drop_cash);           // ���� ������ �ڵ鷯
    this->register_handle(0x09, &acceptor::handle_front_info);          // �չ��� ���� �ڵ鷯
    this->register_handle(0x2D, &acceptor::handle_self_info);           // �� �ڽ��� ���� �ڵ鷯
    this->register_handle(0x1B, &acceptor::handle_option_changed);      // �ɼ� ���� �ڵ鷯
    this->register_handle(0x43, &acceptor::handle_click_object);        // ������Ʈ Ŭ�� �ڵ鷯
    this->register_handle(0x66, &acceptor::handle_item_info);           // �κ��丮 ��Ŭ�� �ڵ鷯
    this->register_handle(0x6B, &acceptor::handle_itemmix);             // ������ ���� �ڵ鷯
    this->register_handle(0x4A, &acceptor::handle_trade);               // ��ȯ �ڵ鷯
    this->register_handle(0x2E, &acceptor::handle_group);               // �׷� �ڵ鷯
    this->register_handle(0x18, &acceptor::handle_user_list);           // ���� ����Ʈ �ڵ鷯
    this->register_handle(0x0E, &acceptor::handle_chat);                // ���� ä�� �ڵ鷯
    this->register_handle(0x3B, &acceptor::handle_board);               // �Խ��� ���� ����Ʈ �ڵ鷯
	this->register_handle(0x30, &acceptor::handle_swap);                // ���� ���� ����
    this->register_handle(0x3A, &acceptor::handle_dialog);              // ���̾�α�
    this->register_handle(0x39, &acceptor::handle_dialog);              // ���̾�α�
    this->register_handle(0x17, &acceptor::handle_throw_item);          // ������ ������ �ڵ鷯

    this->register_timer(100, &acceptor::handle_mob_action);            // �� �ൿ Ÿ�̸�
    this->register_timer(1000, &acceptor::handle_mob_respawn);          // �� ���� Ÿ�̸�
}

acceptor::~acceptor()
{ 
    
}

bool acceptor::handle_connected(fb::game::session& session)
{
    auto& maps = db::maps();
    session.map(maps[315], point16_t(6, 8));
    session.name("ä����");
    session.look(0x61);
    session.color(0x0A);
    session.money(150);
    session.sex(fb::game::sex::MAN);
    session.legends_add(0x4A, 0x10, "������ ����");
    session.legends_add(0x4A, 0x10, "������ ����");
    session.base_hp(0xFFFFFFFF);
    session.hp(0xFFFFFFFF);
    session.title("������ Ÿ��Ʋ");

    auto& items = db::items();
    session.items.add(static_cast<item*>(items[1015]->make())); // ��ȭ�ǹ���
    session.items.add(static_cast<item*>(items[243]->make())); // ������
    session.items.add(static_cast<item*>(items[698]->make())); // ����
    session.items.add(static_cast<item*>(items[3014]->make())); // ���丮
    session.items.add(static_cast<item*>(items[2200]->make())); // ������

    // ������ ���·� ���� (������ �� ���� �� �ִ� ������� �����ص־� ��)
    session.items.weapon(static_cast<weapon*>(items[15]->make())); // �ʽ����� ��
    session.items.helmet(static_cast<helmet*>(items[1340]->make()));
    session.items.ring(static_cast<ring*>(items[1689]->make()));
    session.items.ring(static_cast<ring*>(items[1689]->make()));
    session.items.auxiliary(static_cast<auxiliary*>(items[2135]->make()));
    session.items.auxiliary(static_cast<auxiliary*>(items[2129]->make()));


    auto& spells = db::spells();
    for(auto pair : spells)
    {
        if(session.spells.add(pair.second) == -1)
            break;
    }

    return true;
}

bool acceptor::handle_disconnected(fb::game::session& session)
{
    return false;
}

void fb::game::acceptor::handle_timer(uint64_t elapsed_milliseconds)
{
    for(auto pair : db::maps())
        pair.second->handle_timer(elapsed_milliseconds);
}

fb::ostream fb::game::acceptor::make_time_stream()
{
    fb::ostream             ostream;
    uint8_t                 hours = 25;
    ostream.write_u8(0x20)      // cmd : 0x20
           .write_u8(hours%24)  // hours
           .write_u8(0x00)      // Unknown
           .write_u8(0x00);     // Unknown

    return ostream;
}

void fb::game::acceptor::send_stream(object& object, const fb::ostream& stream, acceptor::scope scope, bool exclude_self, bool encrypt)
{
    if(stream.empty())
        return;

    switch(scope)
    {
    case acceptor::scope::SELF:
        __super::send_stream(static_cast<fb::game::session&>(object), stream, encrypt);
        break;

    case acceptor::scope::PIVOT:
    {
        const auto sessions = object.looking_sessions();
        if(!exclude_self)
            __super::send_stream(static_cast<fb::game::session&>(object), stream, encrypt);

        for(const auto session : sessions)
            __super::send_stream(*session, stream, encrypt);
        break;
    }

    case acceptor::scope::MAP:
    {
        const auto& sessions = this->sessions();
        for(const auto session : sessions)
        {
            if(session->map() != object.map())
                continue;

            if(exclude_self && session == &object)
                continue;

            __super::send_stream(*session, stream, encrypt);
        }

        break;
    }

    }
}

bool fb::game::acceptor::handle_move_life(fb::game::life* life, fb::game::direction direction)
{
    if(life->alive() == false)
        return false;

    bool                        result = false;

    try
    {
        std::vector<fb::game::session*> shown_sessions, hidden_sessions;

        if(life->move(direction, NULL, NULL, NULL, NULL, NULL, NULL, &shown_sessions, &hidden_sessions) == false)
            throw std::exception();

        for(auto session : shown_sessions)
            this->send_stream(*session, life->make_show_stream(), scope::SELF);

        for(auto session : hidden_sessions)
            this->send_stream(*session, life->make_hide_stream(), scope::SELF);

        for(const auto session : life->map()->sessions())
            this->send_stream(*session, life->make_move_stream(direction), scope::SELF);

        result = true;
    }
    catch(std::exception&)
    {
        result = false;
    }

    if(life->direction() != direction)
    {
        life->direction(direction);
        this->send_stream(*life, life->make_direction_stream(), scope::PIVOT, true);
    }

    return result;
}

void fb::game::acceptor::handle_damage(fb::game::session& session, fb::game::mob& mob, uint32_t damage)
{
    std::stringstream       sstream;

    try
    {
        auto                map = mob.map();

        // �� ü�� ��� ü�°����� ǥ��
        mob.hp_down(damage);
        this->send_stream(mob, mob.make_show_hp_stream(damage, true), scope::PIVOT, true);

        // �°��� ��������� �� �̻� ������ �� ����
        if(mob.alive())
        {
            if(mob.offensive() != mob::offensive_type::NONE)
                mob.target(&session);

            return;
        }

        // �� ü���� �� ������� ���δ�.
        this->send_stream(mob, mob.make_die_stream(), scope::PIVOT, true);
        mob.dead_time(::GetTickCount64());

        // ��� ������ ������
        std::vector<object*> dropped_items;
        for(auto candidate : mob.items())
        {
            if(std::rand() % 100 > candidate.percentage)
                continue;

            auto            item = static_cast<fb::game::item*>(candidate.item->make());
            item->map(map, mob.position());

            dropped_items.push_back(item);
        }

        if(dropped_items.size() != 0)
            this->send_stream(mob, object::make_show_stream(dropped_items), scope::PIVOT, true);

#if defined DEBUG | defined _DEBUG
        this->handle_experience(session, mob.experience(), false);
#else
        this->handle_experience(session, mob.experience(), true);
#endif
    }
    catch(std::exception& e)
    {
        sstream << e.what();
    }

    this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);
}

void fb::game::acceptor::handle_damage(fb::game::life& from, fb::game::life& to, uint32_t damage)
{
    to.hp_down(damage);

    // �����ϴ� ��Ŷ ������.
    this->send_stream(from, from.make_action_stream(action::ATTACK, duration::DURATION_ATTACK), scope::PIVOT, true);
    this->send_stream(to, to.make_show_hp_stream(damage, false), scope::PIVOT);
    this->send_stream(to, to.make_state_stream(state_level::LEVEL_MIDDLE), scope::SELF);
}

void fb::game::acceptor::handle_experience(fb::game::session& session, uint32_t exp, bool limit)
{
    // ���� 5�鼭 ���� �Ȱ������� ���� ������� �Ѵ�.
    if(session.level() >= 5 && session.cls() == 0)
        throw session::require_class_exception();

    // ���� ������ ���� ���� ����ġ
    auto                    next_exp = session.max_level() ? 0xFFFFFFFF : db::required_exp(session.cls(), session.level()+1);
    auto                    prev_exp = session.max_level() ? 0x00000000 : db::required_exp(session.cls(), session.level());
    auto                    exp_range = next_exp - prev_exp;

    // 3.3% ������ ����ġ
    if(limit && session.max_level() == false)
        exp = std::min(uint32_t(exp_range / 100.0f*3.3f + 1), exp);


    // ����ġ ���
    session.experience_add(exp);
    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);

    // ����ġ ��� �޽���
    auto                    exp_now = session.experience();
    auto                    percentage = std::min(100.0f, ((exp_now - prev_exp) / float(exp_range)) * 100.0f);
    std::stringstream       sstream;
    sstream << "����ġ�� " << exp << '(' << int(percentage) << "%) �ö����ϴ�.";
    this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);

    // ������ Ȯ��
    if(session.max_level() == false && (exp_now >= next_exp))
        this->handle_level_up(session);
}

void fb::game::acceptor::handle_level_up(fb::game::session& session)
{
    session.level_up();

    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
    this->send_stream(session, session.make_effet_stream(0x02), scope::PIVOT);

    this->send_stream(session, message::make_stream(message::level::UP, message::type::MESSAGE_STATE), scope::SELF);
}

void fb::game::acceptor::handle_click_mob(fb::game::session& session, fb::game::mob& mob)
{
    this->send_stream(session, message::make_stream(mob.name(), message::type::MESSAGE_STATE), scope::SELF);
}

void fb::game::acceptor::handle_click_npc(fb::game::session& session, fb::game::npc& npc)
{
	if(session.dialog_thread != nullptr)
		delete session.dialog_thread;

	session.dialog_thread = new lua::thread();

    luaL_dofile(*session.dialog_thread, "scripts/script.lua");
    lua_getglobal(*session.dialog_thread, "handle_click");

	// ��ƽ�ũ��Ʈ�� handle_click �Լ��� ���ϰ� ����
    session.new_lua<fb::game::session>(*session.dialog_thread);
    npc.new_lua<fb::game::npc>(*session.dialog_thread);
	session.dialog_thread->resume(2);
}

bool acceptor::handle_login(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 enc_key[0x09] = {0,};

    uint8_t cmd = istream.read_u8();
    uint8_t enc_type = istream.read_u8();
    uint8_t key_size = istream.read_u8();
    istream.read(enc_key, key_size);
    session.crt(enc_type, enc_key);

    fb::ostream             ostream;
    ostream.write_u8(0x1E)
           .write_u8(0x06)
           .write_u8(0x00);
    this->send_stream(session, ostream, scope::SELF);
    
    this->send_stream(session, this->make_time_stream(), scope::SELF);

    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);

    this->send_stream(session, message::make_stream("0�ð� 1�и��� �ٶ�����", message::type::MESSAGE_STATE), scope::SELF);

    this->send_stream(session, session.make_id_stream(), scope::SELF);

    this->send_stream(session, session.map()->make_config_stream(), scope::SELF);

    this->send_stream(session, session.map()->make_bgm_stream(), scope::SELF);

    this->send_stream(session, session.make_position_stream(), scope::SELF);

    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);

    this->send_stream(session, session.make_show_objects_stream(), scope::SELF);

    this->send_stream(session, session.make_visual_stream(false), scope::PIVOT);
    this->send_stream(session, session.make_direction_stream(), scope::SELF);
    this->send_stream(session, session.make_option_stream(), scope::SELF);

    // Initialize spell list
    for(int i = 0; i < spell::MAX_SLOT; i++)
    {
        auto spell = session.spells[i];
        if(spell == nullptr)
            continue;

        this->send_stream(session, spell->make_show_stream(i+1), scope::SELF);
    }

    // Initialize mobs
    for(auto i : session.looking_sessions())
    {
        if(i == &session)
            continue;

        this->send_stream(session, i->make_visual_stream(false), scope::SELF);
    }

    for(int i = 0; i < fb::game::item::MAX_SLOT; i++)
        this->send_stream(session, session.make_update_item_slot_stream(i), scope::SELF);

    return true;
}

bool fb::game::acceptor::handle_direction(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    fb::game::direction     direction = fb::game::direction(istream.read_u8());

    if(session.direction(direction) == false)
        return false;

    this->send_stream(session, session.make_direction_stream(), scope::PIVOT, true);
    return true;
}

bool fb::game::acceptor::handle_move(fb::game::session& session)
{
    auto&                   istream = session.in_stream();

    fb::game::map*          map = session.map();
    if(map == NULL)
        return false;

    uint8_t                 cmd = istream.read_u8();
    fb::game::direction     direction = fb::game::direction(istream.read_u8());
    uint8_t                 sequence = istream.read_u8();
    uint16_t                x = istream.read_u16();
    uint16_t                y = istream.read_u16();

    session.direction(direction);

    point16_t               before(x, y);
    if(session.position() != before)
        this->send_stream(session, session.make_position_stream(), scope::SELF);

    std::vector<object*>            show_objects, hide_objects;
    std::vector<fb::game::session*> show_sessions, hide_sessions, shown_sessions, hidden_sessions;
    if(session.move_forward(&show_objects, &hide_objects, &show_sessions, &hide_sessions, NULL, NULL, &shown_sessions, &hidden_sessions))
    {
        this->send_stream(session, session.make_move_stream(), scope::PIVOT, true);
    }
    else
    {
        this->send_stream(session, session.make_position_stream(), scope::SELF);
        return true;
    }


    // ���� ��ġ��� �����Ѵ�.
    const auto              warp = map->warpable(session.position());
    if(warp != NULL)
    {
        this->handle_session_warp(session, warp);
        return true;
    }
    
    // ������Ʈ ����
    this->send_stream(session, object::make_show_stream(show_objects), scope::SELF);
    for(auto i : hide_objects)
        this->send_stream(session, i->make_hide_stream(), scope::SELF);

    // ������ ���ǿ��� ���� ���̴� ���� ����
    for(auto i : show_sessions)
        this->send_stream(session, i->make_visual_stream(false), scope::SELF);

    // ������ ���ǿ��� ����� ���� ����
    for(auto i : hide_sessions)
        this->send_stream(session, i->make_hide_stream(), scope::SELF);

    for(auto i : shown_sessions)
        this->send_stream(*i, session.make_visual_stream(false), scope::SELF);

    for(auto i : hidden_sessions)
        this->send_stream(*i, session.make_hide_stream(), scope::SELF);

    return true;
}

bool fb::game::acceptor::handle_update_move(fb::game::session& session)
{
    if(this->handle_move(session) == false)
        return false;

    auto&                   istream = session.in_stream();
    uint16_t                begin_x = istream.read_u16();
    uint16_t                begin_y = istream.read_u16();
    uint8_t                 width = istream.read_u8();
    uint8_t                 height = istream.read_u8();
    uint16_t                crc = istream.read_u16();
    this->send_stream(session, session.map()->make_update_stream(begin_x, begin_y, width, height, crc), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_attack(fb::game::session& session)
{
    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        this->send_stream(session, session.make_action_stream(action::ATTACK, duration::DURATION_ATTACK), scope::PIVOT);
        auto*               weapon = session.items.weapon();
        if(weapon != NULL)
        {
            uint16_t        sound = weapon->sound();
            if(sound != 0)
                this->send_stream(session, session.make_sound_stream(fb::game::action_sounds(sound)), scope::PIVOT);
        }
        else
        {
            // �̰� ���� �������� Ȯ���ϵ��� ����
            this->send_stream(session, session.make_sound_stream(fb::game::action_sounds(0x015D)), scope::PIVOT);
        }

        auto*               front = session.forward_object(object::types::MOB);
        if(front == NULL)
            return true;

        auto*               front_mob = static_cast<fb::game::mob*>(front);

#if !defined DEBUG && !defined _DEBUG
        if(std::rand() % 3 == 0)
            return true;
#endif

        bool                critical = false;
        uint32_t            random_damage = session.random_damage(*front_mob, critical);

        this->handle_damage(session, *front_mob, random_damage);
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::SELF);
    }

    return true;
}

bool fb::game::acceptor::handle_pickup(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    bool                    boost = bool(istream.read_u8());

    try
    {
        auto                map = session.map();
        if(map == NULL)
            return false;

        session.state_assert(state::GHOST | state::RIDING);

        
        // Do action : pick up
        this->send_stream(session, session.make_action_stream(action::PICKUP, duration::DURATION_PICKUP), scope::PIVOT);


        std::string         message;
        const auto&         objects = map->objects();
        std::vector<item*>  gains;

        // Pick up items in reverse order
        for(auto i = objects.rbegin(); i != objects.rend(); i++)
        {
            auto            object = *i;
            if(object->position() != session.position())
                continue;

            if(object->type() != object::types::ITEM)
                continue;

            auto            below = static_cast<fb::game::item*>(object);
            bool            item_moved = false;
            if(below->attr() & fb::game::item::attrs::ITEM_ATTR_CASH)
            {
                auto        cash = static_cast<fb::game::cash*>(below);
                uint32_t    remain = session.money_add(cash->chunk());
                cash->chunk(remain); // �԰� ���� ������ ����

                if(remain != 0)
                    this->send_stream(session, message::make_stream(message::money::FULL, message::type::MESSAGE_STATE), scope::SELF);

                this->send_stream(*cash, cash->make_show_stream(), scope::PIVOT, true);
                this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
            }
            else
            {
                uint8_t         index = session.items.add(below);
                if(index == -1)
                    break;

                this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);

                item_moved = (session.items[index] == below);
            }

            if(item_moved || below->empty())
            {
                gains.push_back(below);
                this->send_stream(*below, below->make_hide_stream(), scope::PIVOT, true);
            }

            if(boost == false)
                break;
        }

        for(auto gain : gains)
        {
            map->object_delete(gain);
            if(gain->empty())
                delete gain;
        }
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::SELF);
    }

    return true;
}

bool fb::game::acceptor::handle_emotion(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    uint8_t                 emotion = istream.read_u8();

    this->send_stream(session, session.make_action_stream(action(action::EMOTION + emotion), duration::DURATION_EMOTION), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_update_map(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    uint16_t                x = istream.read_u16();
    uint16_t                y = istream.read_u16();
    uint8_t                 width = istream.read_u8();
    uint8_t                 height = istream.read_u8();
    uint16_t                crc = istream.read_u16();

    this->send_stream(session, session.map()->make_update_stream(x, y, width, height, crc), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_refresh(fb::game::session& session)
{
    this->send_stream(session, session.make_position_stream(), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_active_item(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    uint8_t                 index = istream.read_u8() - 1;

    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        uint8_t             updated_index = 0;
        auto                slot(equipment::slot::UNKNOWN_SLOT);
        auto                item = session.items.active(index, &updated_index, slot);

        if(item->attr() & fb::game::item::attrs::ITEM_ATTR_EQUIPMENT)
        {
            this->send_stream(session, session.make_delete_item_slot_stream(fb::game::item::delete_attr::DELETE_NONE, index), scope::SELF);
            if(updated_index != 0xFF)
                this->send_stream(session, session.make_update_item_slot_stream(updated_index), scope::SELF);

            this->send_stream(session, session.make_state_stream(fb::game::state_level::LEVEL_MAX), scope::SELF);
            this->send_stream(session, session.make_update_equipment_stream(slot), scope::SELF);
            this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
            this->send_stream(session, session.make_sound_stream(action_sounds::SOUND_EQUIPMENT_ON), scope::PIVOT);

            std::stringstream sstream;
            switch(slot)
            {
            case fb::game::equipment::slot::WEAPON_SLOT:
                sstream << "w:����  :";
                break;

            case fb::game::equipment::slot::ARMOR_SLOT:
                sstream << "a:����  :";
                break;

            case fb::game::equipment::slot::SHIELD_SLOT:
                sstream << "s:����  :";
                break;

            case fb::game::equipment::slot::HELMET_SLOT:
                sstream << "h:�Ӹ�  :";
                break;

            case fb::game::equipment::slot::LEFT_HAND_SLOT:
                sstream << "l:�޼�  :";
                break;

            case fb::game::equipment::slot::RIGHT_HAND_SLOT:
                sstream << "r:������  :";
                break;

            case fb::game::equipment::slot::LEFT_AUX_SLOT:
                sstream << "[:����1  :";
                break;

            case fb::game::equipment::slot::RIGHT_AUX_SLOT:
                sstream << "]:����2  :";
                break;
            }
            sstream << item->name();
            this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);

            sstream.str(std::string());
            sstream << "���� ����  " << session.defensive_physical() <<"  " << session.regenerative() << " S  " << session.defensive_magical();
            this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);

            return true;
        }


        if(item->attr() & fb::game::item::attrs::ITEM_ATTR_CONSUME)
        {   
            this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
            this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
            this->send_stream(session, session.make_action_stream(action::EAT, duration::DURATION_EAT), scope::SELF);
            this->send_stream(session, session.make_sound_stream(action_sounds::SOUND_EAT), scope::SELF);

            if(item->empty())
            {
                this->send_stream(session, session.make_delete_item_slot_stream(fb::game::item::delete_attr::DELETE_EAT, index), scope::SELF);
                delete item;
            }
        }
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::PIVOT);
    }

    return true;
}

bool fb::game::acceptor::handle_inactive_item(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    auto                    slot(equipment::slot(istream.read_u8()));

    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        uint8_t             item_index = session.items.inactive(slot);
        if(item_index == -1)
            return true;


        // ���� ������ ��Ʈ�� �ʿ�
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
        this->send_stream(session, session.make_equipment_off_stream(slot), scope::SELF);
        this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
        this->send_stream(session, session.make_sound_stream(action_sounds::SOUND_EQUIPMENT_OFF), scope::PIVOT);
        this->send_stream(session, session.make_update_item_slot_stream(item_index), scope::SELF);
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::PIVOT);
    }

    return true;
}

bool fb::game::acceptor::handle_drop_item(fb::game::session& session)
{
    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        auto&               istream = session.in_stream();
        uint8_t             cmd = istream.read_u8();
        uint8_t             index = istream.read_u8() - 1;
        bool                drop_all = bool(istream.read_u8());

        auto                item = session.items[index];
        if(item == NULL)
            return true;

        auto                dropped = item->handle_drop(session, drop_all ? item->count() : 1);
        if(item == dropped)
        {
            this->send_stream(session, session.make_delete_item_slot_stream(fb::game::item::delete_attr::DELETE_DROP, index), scope::SELF);
            session.items.remove(index);
        }

        auto                map = session.map();
        this->send_stream(session, session.make_action_stream(action::PICKUP, duration::DURATION_PICKUP), scope::PIVOT);
        this->send_stream(session, dropped->make_show_stream(), scope::PIVOT);
        this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
    }
    catch(std::exception& e)
    { 
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::SELF);
    }

    return true;
}

bool fb::game::acceptor::handle_drop_cash(fb::game::session& session)
{
    try
    {
        session.state_assert(state::RIDING | state::GHOST);

        auto                map = session.map();
        if(map == NULL)
            return false;

        auto&               istream = session.in_stream();
        uint8_t             cmd = istream.read_u8();
        uint32_t            chunk = std::min(session.money(), istream.read_u32());
        cash*               cash = new fb::game::cash(chunk);

        session.money_reduce(chunk);

        cash->map(map);
        cash->position(session.position());
        this->send_stream(session, session.make_action_stream(action::PICKUP, duration::DURATION_PICKUP), scope::PIVOT);
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        this->send_stream(*cash, cash->make_show_stream(), scope::PIVOT, true);
        this->send_stream(session, message::make_stream(message::money::DROP, message::type::MESSAGE_STATE), scope::SELF);

        return true;
    }
    catch(std::exception& e)
    {
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::SELF);
    }
}

bool fb::game::acceptor::handle_front_info(fb::game::session& session)
{
    auto                    map = session.map();
    if(map == NULL)
        return false;

    for(auto i : session.forward_sessions())
    {
        this->send_stream(session, message::make_stream(i->name(), message::type::MESSAGE_STATE), scope::SELF);
    }

    for(auto i : session.forward_objects(object::types::UNKNOWN))
    {
        if(i->type() == fb::game::object::types::ITEM)
        {
            auto item = static_cast<fb::game::item*>(i);
            this->send_stream(session, message::make_stream(item->name_styled(), message::type::MESSAGE_STATE), scope::SELF);
        }
        else
        {
            this->send_stream(session, message::make_stream(i->name(), message::type::MESSAGE_STATE), scope::SELF);
        }
    }

    return true;
}

bool fb::game::acceptor::handle_self_info(fb::game::session& session)
{
    this->send_stream(session, session.make_internal_info_stream(), scope::SELF);
    return true;
}

bool fb::game::acceptor::handle_option_changed(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    options                 option = options(istream.read_u8());


    bool                    enabled = session.option_toggle(option);
    std::stringstream       sstream;
    switch(option)
    {
    case options::RIDE:
    {
        static auto         horse_core = db::name2mob("��");
        if(horse_core == NULL)
            break;

        try
        {
            session.state_assert(state::GHOST | state::DISGUISE);

            object*         forward = NULL;
            std::string     message;

            if(session.state() == fb::game::state::RIDING)
            {
                message = game::message::ride::ON;
                session.state(fb::game::state::NORMAL);

                auto horse = static_cast<fb::game::mob*>(horse_core->make());
                horse->map(session.map(), session.position_forward());
                horse->heal();
                this->send_stream(*horse, horse->make_show_stream(), scope::PIVOT, true);
            }
            else
            {
                forward = session.forward_object(object::types::MOB);
                if(forward == NULL || forward->based() != horse_core)
                    throw session::no_conveyance_exception();

                forward->map()->object_delete(forward);
                this->send_stream(*forward, forward->make_hide_stream(), scope::PIVOT, true);
                message = game::message::ride::OFF;
                session.state(fb::game::state::RIDING);

                forward->map()->object_delete(forward);
                delete forward;
            }

            this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
            this->send_stream(session, message::make_stream(message, message::type::MESSAGE_STATE), scope::SELF);
        }
        catch(std::exception& e)
        {
            this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::SELF);
        }
        return true;
    }

    case options::WHISPER:
        sstream << "�ӼӸ����  ";
        break;

    case options::GROUP:
        sstream << "�׷��㰡    ";
        break;

    case options::ROAR:
        sstream << "��ġ����  ";
        break;

    case options::ROAR_WORLDS:
        sstream << "������      ";
        break;

    case options::MAGIC_EFFECT:
        sstream << "��������Ʈ  ";
        break;

    case options::WEATHER_EFFECT:
        sstream << "������ȭ    ";
        break;

    case options::FIXED_MOVE:
        sstream << "�����̵�     ";
        break;

    case options::TRADE:
        sstream << "��ȯ����    ";
        break;

    case options::FAST_MOVE:
        sstream << "�����̵�    ";
        break;

    case options::EFFECT_SOUND:
        sstream << "�Ҹ����    ";
        break;

    case options::PK:
        sstream << "PK��ȣ      ";
        break;

    default:
        return false;
    }

    sstream << ": " << (enabled ? "ON" : "OFF");
    this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);
    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
    this->send_stream(session, session.make_option_stream(), scope::SELF);

    return true;
}

bool fb::game::acceptor::handle_click_object(fb::game::session& session)
{
    auto&                   istream = session.in_stream();
    uint8_t                 cmd = istream.read_u8();
    uint8_t                 unknown = istream.read_u8();
    uint32_t                fd = istream.read_u32();

    if(fd == 0xFFFFFFFF) // Press F1
    {
        return true;
    }

    if(fd == 0xFFFFFFFE) // Preff F2
    {
        return true;
    }

    auto                    other = this->session(fd);
    if(other != NULL) // character
    {
        this->send_stream(session, other->make_external_info_stream(), scope::SELF);
        return true;
    }

    auto                    object = session.map()->object(fd);
    if(object != NULL) // object
    {
        switch(object->type())
        {
        case fb::game::object::types::MOB:
            this->handle_click_mob(session, *static_cast<mob*>(object));
            break;

        case fb::game::object::types::NPC:
            this->handle_click_npc(session, *static_cast<npc*>(object));
            break;
        }

        return true;
    }

    return true;
}

bool fb::game::acceptor::handle_item_info(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    uint8_t                     cmd = istream.read_u8();
    uint16_t                    position = istream.read_u16();
    uint8_t                     unknown1 = istream.read_u8();
    uint8_t                     unknown2 = istream.read_u8();
    uint8_t                     unknown3 = istream.read_u8();
    uint8_t                     slot = istream.read_u8() - 1;

    auto                        item = session.items[slot];
    if(item == NULL)
        return false;

    this->send_stream(session, item->make_tip_stream(position), scope::SELF);

    return true;
}

bool fb::game::acceptor::handle_itemmix(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    uint8_t                     cmd = istream.read_u8();
    uint8_t                     count = istream.read_u8();
    if(count > item::MAX_SLOT - 1)
        return false;

    std::vector<item*>          items;
    for(int i = 0; i < count; i++)
    {
        uint8_t                 index = istream.read_u8() - 1;
        auto                    item = session.items[index];
        if(item == NULL)
            return true;

        items.push_back(item);
    }
    
    try
    {
        auto                    itemmix = db::find_itemmix(items);
        if(itemmix == NULL)
            throw itemmix::no_match_exception();

        uint8_t                 free_size = session.items.free_size();
        if(int(itemmix->success.size()) - int(itemmix->require.size()) > free_size)
            throw item::full_inven_exception();


        for(auto require : itemmix->require)
        {
            uint8_t             index = session.items.to_index(require.item);
            if(index == 0xFF)
                return true;

            auto                item = session.items[index];
            item->reduce(require.count);


            if(item->empty())
            {
                this->send_stream(session, session.make_delete_item_slot_stream(item::delete_attr::DELETE_NONE, index), scope::SELF);
                session.items.remove(index);
                delete item;
            }
            else
            {
                this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
            }
        }

        bool                success = (std::rand() % 100) < itemmix->percentage;
        std::string         message;
        if(success)
        {
            for(auto success : itemmix->success)
            {
                auto        item = static_cast<fb::game::item*>(success.item->make());
                item->count(success.count);
                
                uint8_t     index = session.items.add(item);
                this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
            }

            message = game::message::mix::SUCCESS;
        }
        else
        {
            for(auto failed : itemmix->failed)
            {
                auto        item = static_cast<fb::game::item*>(failed.item->make());
                item->count(failed.count);
                
                uint8_t     index = session.items.add(item);
                this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
            }

            message = game::message::mix::FAILED;
        }

        this->send_stream(session, message::make_stream(message, message::type::MESSAGE_STATE), scope::SELF);
    }
    catch(std::exception& e)
    { 
        this->send_stream(session, message::make_stream(e.what(), message::type::MESSAGE_STATE), scope::SELF);
        return true;
    }

    

    return true;
}

bool fb::game::acceptor::handle_trade(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    uint8_t                     cmd = istream.read_u8();
    uint8_t                     action = istream.read_u8();
    uint32_t                    fd = istream.read_u32();

    auto                        partner = this->session(fd);   // ��Ʈ��
    auto&                       ts_mine = session.trade;       // ���� �ŷ��ý���
    auto&                       ts_your = partner->trade;      // ������ �ŷ��ý���

    if(partner == NULL)
        return true;

    switch(action)
    {
    case 0:
    {
        if(session.id() == partner->id())
        {
            // �ڱ� �ڽŰ� �ŷ��� �Ϸ��� �õ��ϴ� ���
            break;
        }

        if(session.option(options::TRADE) == false)
        {
            // ���� ��ȯ �ź���
            this->send_stream(session, message::make_stream(message::trade::REFUSED_BY_ME, message::type::MESSAGE_STATE), scope::SELF);
            break;
        }

        if(partner->option(options::TRADE) == false)
        {
            // ������ ��ȯ �ź���
            std::stringstream sstream;
            sstream << partner->name() << message::trade::REFUSED_BY_PARTNER;
            this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);
            break;
        }

        if(ts_mine.trading())
        {
            // ���� �̹� ��ȯ��
            break;
        }

        if(ts_your.trading())
        {
            // ������ �̹� ��ȯ��
            std::stringstream sstream;
            sstream << partner->name() << message::trade::PARTNER_ALREADY_TRADING;
            this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);
            break;
        }

        if(session.sight(*partner) == false)
        {
            // ������ �þ߿��� ������ ����
            this->send_stream(session, message::make_stream(message::trade::PARTNER_INVISIBLE, message::type::MESSAGE_STATE), scope::SELF);
            break;
        }

        if(session.distance_sqrt(*partner) > 16)
        {
            // ������� �Ÿ��� �ʹ� ��
            std::stringstream sstream;
            sstream << partner->name() << message::trade::PARTNER_TOO_FAR;
            this->send_stream(session, message::make_stream(sstream.str(), message::type::MESSAGE_STATE), scope::SELF);
            break;
        }

        // ��ȯ ����
        ts_mine.begin(partner);
        this->send_stream(session, ts_your.make_dialog_stream(), scope::SELF);

        ts_your.begin(&session);
        this->send_stream(*partner, ts_mine.make_dialog_stream(), scope::SELF);
        break;
    }

    case 1: // ������ �ø���
    {
        uint8_t             index = istream.read_u8() - 1;
        auto                item = session.items[index];
        if(item == NULL)
            return true;

        // ��ȯ �Ұ����� ������ �ŷ� �õ�
        if(item->trade_enabled() == false)
        {
            this->send_stream(session, message::make_stream(message::trade::NOT_ALLOWED_TO_TRADE, message::type::MESSAGE_TRADE), scope::SELF);
            break;
        }

        if((item->attr() & fb::game::item::attrs::ITEM_ATTR_BUNDLE) && (item->count() > 1))
        {
            // ���� ������ ������ ���� �ŷ� �õ�
            ts_mine.select(item);
            this->send_stream(session, ts_mine.make_bundle_stream(), scope::SELF);
        }
        else
        {
            // �Ϲ� �������� �ŷ� �õ�

            uint8_t trade_index = ts_mine.add(item); // �ŷ����� ������ ����Ʈ�� �ε���
            if(trade_index == 0xFF)
                return false;

            // ���� �κ��丮���� �ŷ����� ������ ����Ʈ�� ������ �̵�
            session.items.remove(index);
            this->send_stream(session, session.make_delete_item_slot_stream(item::delete_attr::DELETE_NONE, index), scope::PIVOT);

            // ���� ��� �� �ٿ��� �ø� �������� ǥ����
            this->send_stream(session, ts_mine.make_show_stream(true, trade_index), scope::SELF);
            this->send_stream(*partner, ts_mine.make_show_stream(false, trade_index), scope::SELF);
        }
    }
        break;

    case 2: // ������ �������� �ؼ� �ø� ��
    {
        // ������ �ø����� �õ��� ���� ������ ������
        auto                selected = ts_mine.selected();
        if(selected == NULL)
            return false;

        // �ø� ���� (Ŭ���̾�Ʈ�� �Է��� ��)
        uint16_t            count = istream.read_u16();
        if(selected->count() < count)
        {
            this->send_stream(session, message::make_stream(message::trade::INVALID_COUNT, message::type::MESSAGE_TRADE), scope::SELF);
            break;
        }

        uint8_t             index = session.items.to_index(selected->based<item::core>());
        if(selected->count() == count)
        {
            // ��� �� �ø��� ���, �������� ���� �������� �ʰ� �ִ� �״�� �ŷ�����Ʈ�� �Űܹ�����.
            uint8_t         trade_index = ts_mine.add(selected);
            if(index != 0xFF)
            {
                session.items.remove(index);
                this->send_stream(session, session.make_delete_item_slot_stream(item::delete_attr::DELETE_NONE, index), scope::SELF);

                this->send_stream(session, ts_mine.make_show_stream(true, trade_index), scope::SELF);
                this->send_stream(*partner, ts_mine.make_show_stream(false, trade_index), scope::SELF);
            }
        }
        else
        {
            // �Ϻθ� �ø��� ���, ������ �Ϳ��� ������ ��� ���� ����� �������� �ŷ� ����Ʈ�� �ű��.
            selected->reduce(count);
            auto            came_out = selected->clone<fb::game::item>();
            came_out->count(count);

            uint8_t trade_index = ts_mine.add(came_out);

            this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);

            this->send_stream(session, ts_mine.make_show_stream(true, trade_index), scope::SELF);
            this->send_stream(*partner, ts_mine.make_show_stream(false, trade_index), scope::SELF);
        }
        break;
    }

    case 3: // ���� �ø� ��
    {
        // Ŭ���̾�Ʈ�� �Է��� ���� ��
        uint32_t            money = istream.read_u32();

        // �Է��� ���� ���� ����ؼ� ���� �ȵȴ�.
        // 100�� �Է��� ��� -1, -10, -100 �̷��� �����
        uint32_t            total = session.money() + ts_mine.money();
        if(money > total)
            money = total;

        session.money(total - money);
        ts_mine.money(money);

        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        this->send_stream(session, ts_mine.make_money_stream(true), scope::SELF);
        this->send_stream(*partner, ts_mine.make_money_stream(false), scope::SELF);
        break;
    }

    case 4: // ����� ���
    {
        std::vector<uint8_t> indices;

        // �ŷ�����Ʈ�� �÷ȴ� �����۰� ������ ���󺹱ͽ�Ų��.
        indices = ts_mine.restore();
        this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        for(auto i : indices)
            this->send_stream(session, session.make_update_item_slot_stream(i), scope::SELF);

        // �޽��� ǥ���ϰ� �ŷ�����
        this->send_stream(session, ts_mine.make_close_stream(message::trade::CANCELLED_BY_ME), scope::SELF);
        ts_mine.end();



        // �ŷ�����Ʈ�� �÷ȴ� �����۰� ������ ���󺹱ͽ�Ų��.
        indices = ts_your.restore();
        this->send_stream(*partner, partner->make_state_stream(state_level::LEVEL_MIN), scope::SELF);
        for(auto i : indices)
            this->send_stream(*partner, partner->make_update_item_slot_stream(i), scope::SELF);

        // �޽��� ǥ���ϰ� �ŷ�����
        this->send_stream(*partner, ts_your.make_close_stream(message::trade::CANCELLED_BY_PARTNER), scope::SELF);
        ts_your.end();
        break;
    }

    case 5:
    {
        this->send_stream(session, ts_mine.make_lock_stream(), scope::SELF);
        ts_mine.lock(true);

        if(ts_your.lock())
        {
            // ������ �̹� ��ȯȮ�� ���� ���
            if(ts_mine.flushable(*partner) == false || ts_your.flushable(session) == false)
            {
                // �� Ȥ�� ��밡 �ŷ�����Ʈ�� �ö�� �������̳� ������ �� ���� �� ���� ��Ȳ�� ��
                // �κ��丮�� ���� ���ų�, �� �̻� ������ ���� �� ���� ���
                this->send_stream(session, ts_mine.make_close_stream(message::trade::FAILED), scope::SELF);
                this->send_stream(*partner, ts_your.make_close_stream(message::trade::FAILED), scope::SELF);
            }
            else
            {
                std::vector<uint8_t> indices;
                
                // ����� �ŷ�����Ʈ ��ǰ���� ���� �ް� ������Ʈ
                ts_mine.flush(*partner, indices);
                for(auto i : indices)
                    this->send_stream(*partner, partner->make_update_item_slot_stream(i), scope::SELF);
                this->send_stream(*partner, partner->make_state_stream(state_level::LEVEL_MIN), scope::SELF);

                // ���� �ŷ�����Ʈ ��ǰ���� ���� �ְ� ������Ʈ
                ts_your.flush(session, indices);
                for(auto i : indices)
                    this->send_stream(session, session.make_update_item_slot_stream(i), scope::SELF);
                this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);


                // �޽��� ǥ���ϰ� �ŷ� ����
                this->send_stream(session, ts_mine.make_close_stream(message::trade::SUCCESS), scope::SELF);
                ts_mine.end();

                this->send_stream(*partner, ts_your.make_close_stream(message::trade::SUCCESS), scope::SELF);
                ts_your.end();
            }
        }
        else
        {
            // ������ ���� ��ȯȮ���� ������ ���� ���
            this->send_stream(*partner, message::make_stream(message::trade::NOTIFY_LOCK_TO_PARTNER, message::type::MESSAGE_TRADE), scope::SELF);
        }
        break;
    }
    }

    return true;
}

bool fb::game::acceptor::handle_group(fb::game::session& session)
{
    auto&                       istream = session.in_stream();

    uint8_t                     cmd = istream.read_u8();
    uint8_t                     name_size = istream.read_u8();
    
    char*                       buffer = new char[name_size];
    istream.read(buffer, name_size);
    std::string                 name(buffer);
    delete[] buffer;


    // ���⼭ �������ִ� ���� ��ü ������� ������ �� �� �ƴ϶�
    // DB�� �����ؼ� �ϴ� ����� ���� �� ���ٴ� ������ ��±���
    
    return true;
}

bool fb::game::acceptor::handle_user_list(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    uint8_t                     cmd = istream.read_u8();
    uint8_t                     unknown = istream.read_u8();


    fb::ostream                 ostream;
    auto&                       sessions = this->sessions();
    ostream.write_u8(0x36)
        .write_u16(sessions.size())
        .write_u16(sessions.size())
        .write_u8(0x00);

    for(const auto& i : sessions)
    {
        const auto& name = i->name();

        ostream.write_u8(0x10 * i->nation())
            .write_u8(0x10 * i->promotion())
            .write_u8((&session == i) ? 0x88 : 0x0F)
            .write(name, false);
    }

    this->send_stream(session, ostream, scope::SELF);

    return true;
}

bool fb::game::acceptor::handle_chat(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    uint8_t                     cmd = istream.read_u8();
    
    uint8_t                     shout = istream.read_u8();
    uint8_t                     length = istream.read_u8();
    static char                 message[0xFF];
    istream.read(message, length);
    message[length] = 0;

#if defined DEBUG | defined _DEBUG
    if(this->handle_admin(session, message))
        return true;
#endif

    this->send_stream(session, session.make_chat_stream(message, shout), shout ? scope::MAP : scope::PIVOT);

    return true;
}

bool fb::game::acceptor::handle_board(fb::game::session& session)
{
    auto&                       board = db::board();
    auto&                       istream = session.in_stream();
    uint8_t                     cmd = istream.read_u8();
    uint8_t                     action = istream.read_u8();

    switch(action)
    {
    case 0x01: // section list
    {
        this->send_stream(session, board.make_sections_stream(), scope::SELF);
        break;
    }

    case 0x02: // article list
    {
        uint16_t                section_id = istream.read_u16();
        uint16_t                offset = istream.read_u16();

        this->send_stream(session, board.make_articles_stream(section_id, offset), scope::SELF);
        break;
    }

    case 0x03: // article
    {
        uint16_t                section_id = istream.read_u16();
        uint16_t                article_id = istream.read_u16();

        this->send_stream(session, board.make_article_stream(section_id, article_id, session), scope::SELF);
        break;
    }

    case 0x05: // delete
    {
        uint16_t                section_id = istream.read_u16();
        uint16_t                article_id = istream.read_u16();

        this->send_stream(session, board.make_delete_stream(section_id, article_id, session), scope::SELF);
        break;
    }

    }

    return true;
}

bool fb::game::acceptor::handle_swap(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    uint8_t                     cmd = istream.read_u8();
    uint8_t                     swap_type = istream.read_u8();
    uint8_t                     src = istream.read_u8();
    uint8_t                     dest = istream.read_u8();

    if(swap_type) // spell
    {
        if(session.spells.swap(src-1, dest-1) == false) // zero-based
            return true;

        const auto              right = session.spells[src-1];
        this->send_stream(session, right ? right->make_show_stream(src) : right->make_delete_stream(src), scope::SELF);

        const auto              left = session.spells[dest-1];
        this->send_stream(session, left ? left->make_show_stream(dest) : left->make_delete_stream(dest), scope::SELF);
    }
    else // item
    {}

    return true;
}

bool fb::game::acceptor::handle_dialog(fb::game::session& session)
{
	auto&                       istream = session.in_stream();
	uint8_t                     cmd = istream.read_u8();
	auto						interaction = istream.read_u8();

	switch(interaction)
	{
    case dialog::interaction::NORMAL: // �Ϲ� ���̾�α�
	{
		istream.read(nullptr, 0x07); // 7����Ʈ ����
		auto					action = istream.read_u8();

        if(session.dialog_thread == nullptr)
            return true;

		// ��ƽ�ũ��Ʈ ���̾�α� �Լ��� ���ϰ� ����
		session.dialog_thread->pushinteger(action);
		if(session.dialog_thread->resume(1))
		{
			delete session.dialog_thread;
			session.dialog_thread = nullptr;
		}
		break;
	}

    case dialog::interaction::INPUT:
	{
        auto unknown1 = istream.read_u16();
        auto unknown2 = istream.read_u32();
        auto message = istream.readstr_u16();

        if(session.dialog_thread == nullptr)
            return true;

        session.dialog_thread->pushstring(message);
        if(session.dialog_thread->resume(1))
        {
            delete session.dialog_thread;
            session.dialog_thread = nullptr;
        }
		break;
	}

    case dialog::interaction::INPUT_EX:
	{
		if(session.dialog_thread == nullptr)
			return true;

		istream.read(nullptr, 0x07); // 7����Ʈ ����
		auto					action = istream.read_u8();
		if(action == 0x02) // OK button
		{
			auto unknown1 = istream.read_u8();
			auto message = istream.readstr_u8();
			session.dialog_thread->pushstring(message);
		}
		else
		{
            session.dialog_thread->pushinteger(action);
		}

		if(session.dialog_thread->resume(1))
		{
			delete session.dialog_thread;
			session.dialog_thread = nullptr;
		}
		break;
	}

    case dialog::interaction::MENU:
	{
        auto unknown = istream.read_u32();
        auto index = istream.read_u16();

        if(session.dialog_thread == nullptr)
            return true;

        session.dialog_thread->pushinteger(index);
        if(session.dialog_thread->resume(1))
        {
            delete session.dialog_thread;
            session.dialog_thread = nullptr;
        }
		break;
	}

    case dialog::interaction::SLOT:
	{
		break;
	}

    case dialog::interaction::SELL:
    {
        auto unknown = istream.read_u32();
        auto pursuit = istream.read_u16();
        auto item_name = istream.readstr_u8();

        if(session.dialog_thread == nullptr)
            return true;

        session.dialog_thread->pushstring(item_name);
        if(session.dialog_thread->resume(1))
        {
            delete session.dialog_thread;
            session.dialog_thread = nullptr;
        }

        break;
    }

	default:
	{
		break;
	}
	}

	return true;
}

bool fb::game::acceptor::handle_throw_item(fb::game::session& session)
{
    auto&                       istream = session.in_stream();
    auto                        cmd = istream.read_u8();
    auto                        unknown = istream.read_u8();
    auto                        slot = istream.read_u8() - 1;

    try
    {
        auto                    item = session.items.at(slot);
        if(item == nullptr)
            throw std::exception();

        if(item->trade_enabled() == false)
            throw std::runtime_error("���� �� ���� �����Դϴ�.");

        auto                    map = session.map();
        if(map == nullptr)
            throw std::exception();

        auto                    dropped = item->handle_drop(session, 1);
        item->direction(session.direction());
        for(int i = 0; i < 7; i++)
        {
            if(map->movable_forward(*item) == false)
                break;

            item->position(item->position_forward());
        }

        if(item == dropped)
        {
            session.items.remove(slot);
            this->send_stream(session, session.make_delete_item_slot_stream(item::delete_attr::DELETE_THROW, slot), scope::SELF);
        }
        else
        {
            this->send_stream(session, session.make_update_item_slot_stream(slot), scope::SELF);
        }
        this->send_stream(session, session.make_throw_item_stream(*item), scope::PIVOT);
        this->send_stream(session, item->make_show_stream(), scope::PIVOT);
    }
    catch(std::runtime_error& e)
    {
        auto message = std::string(e.what());
        if(message.empty() == false)
            this->send_stream(session, message::make_stream(message, message::type::MESSAGE_STATE), scope::SELF);
    }
    catch(std::exception& e)
    {

    }
    return true;
}

void fb::game::acceptor::handle_counter_mob_action(fb::game::mob* mob)
{
    auto                    target = mob->target();
    if(target == NULL)
    {
        this->handle_move_life(mob, direction(std::rand() % 4));
        return;
    }

    // �����¿�� Ÿ���� �ִ��� �˻��Ѵ�.
    for(int i = 0; i < 4; i++)
    {
        auto                direction = game::direction(i);
        if(mob->near_session(direction) != target)
            continue;

        if(mob->direction() != direction)
        {
            mob->direction(direction);
            this->send_stream(*mob, mob->make_direction_stream(), scope::PIVOT, true);
        }

        auto                damage = mob->random_damage(*target);
        this->handle_damage(*mob, *target, damage);
        return;
    }


    // Ÿ�� �������� �̵��� �����ϴٸ� �̵��Ѵ�.
    auto                    x_axis = bool(std::rand()%2);
    if(x_axis)
    {
        if(mob->x() > target->x() && this->handle_move_life(mob, direction::LEFT))   return;
        if(mob->x() < target->x() && this->handle_move_life(mob, direction::RIGHT))  return;
        if(mob->y() > target->y() && this->handle_move_life(mob, direction::TOP))    return;
        if(mob->y() < target->y() && this->handle_move_life(mob, direction::BOTTOM)) return;
    }
    else
    {
        if(mob->y() > target->y() && this->handle_move_life(mob, direction::TOP))    return;
        if(mob->y() < target->y() && this->handle_move_life(mob, direction::BOTTOM)) return;
        if(mob->x() > target->x() && this->handle_move_life(mob, direction::LEFT))   return;
        if(mob->x() < target->x() && this->handle_move_life(mob, direction::RIGHT))  return;
    }


    // �̵��� �� �ִ� �������� �ϴ� �̵��Ѵ�.
    auto                    random_direction = std::rand() % 4;
    for(int i = 0; i < 4; i++)
    {
        if(this->handle_move_life(mob, direction((random_direction + i) % 4)))
            return;
    }
}

void fb::game::acceptor::handle_containment_mob_action(fb::game::mob* mob)
{
    try
    {
        auto                    target = mob->target();
        if(target == NULL || target->sight(*mob) == false)
            target = mob->autoset_target();

        if(target == NULL)
            throw std::runtime_error("no target");
    }
    catch(std::exception&)
    { }

    this->handle_counter_mob_action(mob);
}

void fb::game::acceptor::handle_mob_action(uint64_t now)
{
    for(auto pair : db::maps())
    {
        auto                map = pair.second;
        const auto&         objects = map->objects();

        for(auto obj_i = objects.begin(); obj_i != objects.end(); obj_i++)
        {
            auto            object = (*obj_i);
            if(object->type() != object::types::MOB)
                continue;

            auto            mob = static_cast<fb::game::mob*>(object);
            if(now < mob->action_time() + mob->speed())
                continue;

            if(mob->alive() == false)
                continue;

            switch(mob->offensive())
            {
            case mob::offensive_type::COUNTER:
                this->handle_counter_mob_action(mob);
                break;

            case mob::offensive_type::CONTAINMENT:
                this->handle_containment_mob_action(mob);
                break;

            default:
                this->handle_counter_mob_action(mob);
                break;
            }

            mob->action_time(now);
        }
    }
}

void fb::game::acceptor::handle_mob_respawn(uint64_t now)
{
    // ������ ��ü ���� ����
    std::vector<object*>    spawned_mobs;
    for(auto pair : db::maps())
    {
        fb::game::map* map = pair.second;
        for(auto object : map->objects())
        {
            if(object->type() != object::types::MOB)
                continue;

            auto mob = static_cast<fb::game::mob*>(object);
            if(mob == NULL)
                continue;

            if(mob->spawn(now) == false)
                continue;

            spawned_mobs.push_back(mob);
        }
    }


    // ȭ�鿡 ���̴� ���� ����
    std::vector<object*> shown_mobs;
    for(auto session : this->sessions())
    {
        if(session == NULL)
            continue;

        shown_mobs.clear();
        for(auto spawned : spawned_mobs)
        {
            if(session->sight(*spawned) == false)
                continue;

            shown_mobs.push_back(spawned);
        }

        this->send_stream(*session, object::make_show_stream(shown_mobs), scope::SELF);
    }
}

void fb::game::acceptor::handle_session_warp(fb::game::session& session, const map::warp* warp)
{
    map*                    map = session.map();

    // ������ ���̴� ������Ʈ�� ���� ����
    for(auto i : session.shown_objects())
        this->send_stream(session, i->make_hide_stream(), scope::SELF);

    for(auto i : session.shown_sessions())
        this->send_stream(session, i->make_hide_stream(), scope::SELF);

    session.map(warp->map, warp->after);

    this->send_stream(session, session.make_id_stream(), scope::SELF);
    this->send_stream(session, warp->map->make_config_stream(), scope::SELF);
    this->send_stream(session, warp->map->make_bgm_stream(), scope::SELF);
    this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
    this->send_stream(session, session.make_position_stream(), scope::SELF);
    this->send_stream(session, session.make_visual_stream(false), scope::SELF);
    this->send_stream(session, session.make_direction_stream(), scope::SELF);

    // ���� ���̴� ������Ʈ ������
    this->send_stream(session, session.make_show_objects_stream(), scope::SELF);

    // ���� ���̴� ���ǵ� ������
    for(auto i : session.shown_sessions())
        this->send_stream(session, i->make_visual_stream(false), scope::SELF);
}

#if defined DEBUG | defined _DEBUG
bool fb::game::acceptor::handle_admin(fb::game::session& session, const std::string& message)
{
    if(message[0] != '/')
        return false;


    auto npc = db::name2npc("����");
    std::string command(message.begin() + 1, message.end());

    if(command == "show")
    {
        auto item = db::name2item("���ڱ���");
        this->send_stream(session, item->make_dialog_stream("�������� ����մϴ�.", true, true), scope::SELF);
        return true;
    }

    if(command == "show short list")
    {
        std::vector<std::string> menus = {"��", "��", "��"};
        this->send_stream(session, npc->make_dialog_stream("�������� �����մϴ�.", menus), scope::SELF);
        return true;
    }

    if(command == "show long list")
    {
        std::vector<std::string> menus;
        std::string message = "���������� �����Ͻ� ���Ӽ��� ���߰��� ū ���̽ô�.";
        int i = 0;
        int mod = 0;
        for(auto c : message)
        {
            i++;
            
            if(c == ' ')
                mod = int(!(bool(mod)));

            if(i % 2 != mod)
                continue;

            menus.push_back(std::string(message.begin(), message.begin() + i));
        }
        this->send_stream(session, npc->make_dialog_stream("�������� �����մϴ�.", menus), scope::SELF);
        return true;
    }

    if(command == "show inventory items")
    {
        std::vector<uint8_t> slots;
        for(int i = 0; i < item::MAX_SLOT; i++)
        {
            const auto item = session.items[i];
            if(item == nullptr)
                continue;

            slots.push_back(i+1);
        }
        this->send_stream(session, npc->make_dialog_stream("���������� �κ��丮�� �����帳�ϴ�.", slots), scope::SELF);
        return true;
    }

    if(command == "show item core list")
    {
        std::vector<item::core*> items;
        int count = 0;
        for(auto pair : db::items())
        {
            items.push_back(pair.second);
            if(count++ > 100)
                break;
        }
        this->send_stream(session, npc->make_dialog_stream("�ƽù� �� �Ǵ�", items), scope::SELF);
        return true;
    }

    if(command == "show question")
    {
        this->send_stream(session, npc->make_input_dialog_stream("What is your name?"), scope::SELF);
        return true;
    }

	if(command == "show extend input")
	{
		this->send_stream(session, npc->make_input_dialog_stream("�ȳ�", "ž", "����", 0xFF, true), scope::SELF);
		return true;
	}

    if(command == "be ghost")
    {
        if(session.state() == state::NORMAL)
            session.state(state::GHOST);
        else
            session.state(state::NORMAL);

        this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
        return true;
    }

    return false;
}
#endif