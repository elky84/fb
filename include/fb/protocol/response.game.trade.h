#ifndef __PROTOCOL_RESPONSE_GAME_TRADE_H__
#define __PROTOCOL_RESPONSE_GAME_TRADE_H__

#include <fb/protocol/protocol.h>
#include <fb/game/trade.h>

namespace fb { namespace protocol { namespace game { namespace response { namespace trade {

class dialog : public fb::protocol::base::header
{
public:
    const fb::game::session&        me;

public:
    dialog(const fb::game::session& me) : fb::protocol::base::header(0x42),
        me(me)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        const auto              class_name = fb::game::model::classes.class2name(this->me.cls(), this->me.promotion());
        if(class_name == nullptr)
            return;

        std::stringstream sstream;
        sstream << this->me.name() << '(' << class_name->c_str() << ')';

        base::header::serialize(out_stream);
        out_stream.write_u8(0x00)
                  .write_u32(this->me.sequence())
                  .write(sstream.str())
                  .write_u8(0x00);
    }
};

class upload : public fb::protocol::base::header
{
public:
    const fb::game::session&    me;
    const uint8_t               index;
    const bool                  mine;

public:
    upload(const fb::game::session& me, uint8_t index, bool mine) : fb::protocol::base::header(0x42),
        me(me), index(index), mine(mine)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        const auto              item = this->me.trade.item(this->index);

        base::header::serialize(out_stream);
        out_stream.write_u8(0x02)
            .write_u8(this->mine ? 0x00 : 0x01)
            .write_u8(this->index) // trade slot index
            .write_u16(item->look())
            .write_u8(item->color())
            .write(item->name_trade())
            .write_u8(0x00);
    }
};

class bundle : public fb::protocol::base::header
{
public:
    bundle() : fb::protocol::base::header(0x42)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x01)
                  .write_u8(0x00);
    }
};

class money : public fb::protocol::base::header
{
public:
    const fb::game::session&    me;
    const bool                  mine;

public:
    money(const fb::game::session& me, bool mine) : fb::protocol::base::header(0x42),
        me(me), mine(mine)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x03)
            .write_u8(this->mine ? 0x00 : 0x01)
            .write_u32(this->me.trade.money())
            .write_u8(0x00);
    }
};

class close : public fb::protocol::base::header
{
public:
    const std::string&          message;

public:
    close(const std::string& message) : fb::protocol::base::header(0x42),
        message(message)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x04)
            .write(this->message, true)
            .write_u8(0x00);
    }
};

class lock : public fb::protocol::base::header
{
public:
    lock() : fb::protocol::base::header(0x42)
    { }

public:
    void serialize(fb::ostream& out_stream) const
    {
        base::header::serialize(out_stream);
        out_stream.write_u8(0x05)
                  .write_u8(0x00);
    }
};

} } } } }

#endif // !__PROTOCOL_RESPONSE_GAME_TRADE_H__