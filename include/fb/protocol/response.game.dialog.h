#ifndef __PROTOCOL_RESPONSE_DIALOG_ITEM_H__
#define __PROTOCOL_RESPONSE_DIALOG_ITEM_H__

#include <fb/protocol/protocol.h>
#include <fb/game/npc.h>

namespace fb { namespace protocol { namespace game { namespace response { namespace dialog {



class common : public fb::protocol::base::header
{
public:
    const fb::game::object::model&      object;
    const std::string                   message;
    const bool                          button_prev;
    const bool                          button_next;
    const fb::game::dialog::interaction interaction;

public:
    common(const fb::game::object::model& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction) : fb::protocol::base::header(0x30),
        object(object), message(message), button_prev(button_prev), button_next(button_next), interaction(interaction)
    { }

    common(const fb::game::object& object, const std::string& message, bool button_prev, bool button_next, fb::game::dialog::interaction interaction) : 
        common(*object.based(), message, button_prev, button_next, interaction)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x00)     // unknown
                  .write_u8(this->interaction)     // interaction
                  .write_u32(0x01)
                  .write_u8(this->object.look > 0xBFFF ? 0x02 : 0x01)
                  .write_u8(0x01)
                  .write_u16(this->object.look)
                  .write_u8(this->object.color)
                  .write_u8(this->object.look > 0xBFFF ? 0x02 : 0x01)
                  .write_u16(this->object.look)
                  .write_u8(this->object.color)
                  .write_u32(0x01)
                  .write_u8(this->button_prev)
                  .write_u8(this->button_next)
                  .write(this->message, true);
    }
};

class menu : public fb::protocol::base::header
{
public:
    const fb::game::npc::model&         npc;
    const std::vector<std::string>      menus;
    const std::string                   message;
    const fb::game::dialog::interaction interaction;

public:
    menu(const fb::game::npc::model& npc, const std::vector<std::string>& menus, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::MENU) : fb::protocol::base::header(0x2F),
        npc(npc), menus(menus), message(message), interaction(interaction)
    { }

    menu(const fb::game::npc& npc, const std::vector<std::string>& menus, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::MENU) : 
        menu(*npc.based<fb::game::npc>(), menus, message, interaction)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x01)
            .write_u8(interaction)
            .write_u32(0x01)
            .write_u8(this->npc.look > 0xBFFF ? 0x02 : 0x01)
            .write_u8(0x01)
            .write_u16(this->npc.look)
            .write_u8(this->npc.color)
            .write_u8(this->npc.look > 0xBFFF ? 0x02 : 0x01)
            .write_u16(this->npc.look)
            .write_u8(this->npc.color)
            .write(message, true);

        out_stream.write_u16((uint16_t)menus.size());
        for(int i = 0; i < menus.size(); i++)
        {
            out_stream.write(menus[i])
                      .write_u16(i);
        }

        out_stream.write_u8(0x00);

        for(int i = 0; i < this->menus.size(); i++)
        {
            out_stream.write(menus[i])
                      .write_u16(i);
        }

        out_stream.write_u8(0x00);
    }
};

class slot : public fb::protocol::base::header
{
public:
    const fb::game::npc::model&         npc;
    const std::vector<uint8_t>          slots;
    const std::string                   message;
    const fb::game::dialog::interaction interaction;

public:
    slot(const fb::game::npc::model& npc, const std::vector<uint8_t>& slots, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::SLOT) : fb::protocol::base::header(0x2F),
        npc(npc), slots(slots), message(message), interaction(interaction)
    { }

    slot(const fb::game::npc& npc, const std::vector<uint8_t>& slots, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::SLOT) : 
        slot(*npc.based<fb::game::npc>(), slots, message, interaction)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x05)
                  .write_u8(this->interaction)
                  .write_u32(0x01)
                  .write_u8(this->npc.look > 0xBFFF ? 0x02 : 0x01)
                  .write_u8(0x01)
                  .write_u16(this->npc.look)
                  .write_u8(this->npc.color)
                  .write_u8(this->npc.look > 0xBFFF ? 0x02 : 0x01)
                  .write_u16(this->npc.look)
                  .write_u8(this->npc.color)
                  .write(this->message, true)
                  .write_u16(0xFFFF)
                  .write_u8((uint8_t)this->slots.size());

        for(auto slot : this->slots)
            out_stream.write_u8(slot);
        out_stream.write_u8(0x00);
    }
};

class item : public fb::protocol::base::header
{
public:
    const fb::game::npc::model&                 npc;
    const fb::game::dialog::item_pairs&         items;
    const std::string                           message;
    const uint16_t                              pursuit;
    const fb::game::dialog::interaction         interaction;

public:
    item(const fb::game::npc::model& npc, const fb::game::dialog::item_pairs& items, const std::string& message, uint16_t pursuit = 0xFFFF, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::ITEM) : fb::protocol::base::header(0x2F),
        npc(npc), items(items), message(message), pursuit(pursuit), interaction(interaction)
    { }

    item(const fb::game::npc& npc, const fb::game::dialog::item_pairs& items, const std::string& message, uint16_t pursuit = 0xFFFF, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::ITEM) :
        item(*npc.based<fb::game::npc>(), items, message, pursuit, interaction)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x04)
            .write_u8(interaction)
            .write_u32(0x01)
            .write_u8(this->npc.look > 0xBFFF ? 0x02 : 0x01)
            .write_u8(0x01)
            .write_u16(this->npc.look)
            .write_u8(this->npc.color)
            .write_u8(this->npc.look > 0xBFFF ? 0x02 : 0x01)
            .write_u16(this->npc.look)
            .write_u8(this->npc.color)
            .write(this->message, true)
            .write_u16(this->pursuit)
            .write_u16((uint16_t)this->items.size());

        for (auto& [item, value] : this->items)
        {
            out_stream.write_u16(item->look)
                .write_u8(item->color)
                .write_u32(value)
                .write(item->name)
                .write(item->desc);
        }

        out_stream.write_u8(0x00);
    }
};


class input : public fb::protocol::base::header
{
public:
    const fb::game::npc::model&         npc;
    const std::vector<uint8_t>          slots;
    const std::string                   message;
    const fb::game::dialog::interaction interaction;

public:
    input(const fb::game::npc::model& npc, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT) : fb::protocol::base::header(0x2F),
        npc(npc), message(message), interaction(interaction)
    { }

    input(const fb::game::npc& npc, const std::string& message, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT) : 
        input(*npc.based<fb::game::npc>(), message, interaction)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x03)
                  .write_u8(this->interaction)
                  .write_u32(0x01)
                  .write_u8(this->npc.look > 0xBFFF ? 0x02 : 0x01)
                  .write_u8(0x01)
                  .write_u16(this->npc.look)
                  .write_u8(this->npc.color)
                  .write_u8(this->npc.look > 0xBFFF ? 0x02 : 0x01)
                  .write_u16(this->npc.look)
                  .write_u8(this->npc.color)
                  .write(this->message, true)
                  .write_u8(0x00);
    }
};

class input_ext : public fb::protocol::base::header
{
public:
    const fb::game::npc::model&         npc;
    const std::vector<uint8_t>          slots;
    const std::string                   message;
    const std::string                   top, bottom;
    const int                           maxlen;
    const bool                          button_prev;
    const fb::game::dialog::interaction interaction;

public:
    input_ext(const fb::game::npc::model& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool button_prev = false, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT_EX) : fb::protocol::base::header(0x30),
        npc(npc), message(message), top(top), bottom(bottom), maxlen(maxlen), button_prev(button_prev), interaction(interaction)
    { }

    input_ext(const fb::game::npc& npc, const std::string& message, const std::string& top, const std::string& bottom, int maxlen = 0xFF, bool button_prev = false, fb::game::dialog::interaction interaction = fb::game::dialog::interaction::INPUT_EX) : 
        input_ext(*npc.based<fb::game::npc>(), message, top, bottom, maxlen, button_prev, interaction)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x04)
                  .write_u8(this->interaction)
                  .write_u32(0x01)
                  .write_u8(this->npc.look > 0xBFFF ? 0x02 : 0x01)
                  .write_u8(0x01)
                  .write_u16(this->npc.look)
                  .write_u8(this->npc.color)
                  .write_u8(this->npc.look > 0xBFFF ? 0x02 : 0x01)
                  .write_u16(this->npc.look)
                  .write_u8(this->npc.color)
                  .write_u32(0x00000001)
                  .write_u8(this->button_prev)
                  .write_u8(0x00)
                  .write(this->message, true)
                  .write(this->top, false)
                  .write_u8(this->maxlen)
                  .write(this->bottom, false)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_DIALOG_ITEM_H__