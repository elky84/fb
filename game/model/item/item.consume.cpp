#include "model/item/item.consume.h"
#include "model/session/session.h"

fb::game::consume::master::master(uint32_t id, const std::string& name, uint16_t look, uint8_t color, uint16_t capacity) : 
    fb::game::item::master(id, name, look, color, capacity)
{}

fb::game::consume::master::~master()
{}

fb::game::item::attrs fb::game::consume::master::attr() const
{
    item::attrs attr = item::attrs::ITEM_ATTR_CONSUME;
    if(this->_capacity > 1)
        attr = item::attrs(attr | item::attrs::ITEM_ATTR_BUNDLE);

    return attr;
}

fb::game::item* fb::game::consume::master::make(fb::game::item::listener* listener) const
{
    return new fb::game::consume(this, listener);
}



fb::game::consume::consume(const fb::game::consume::master* master, listener* listener, uint16_t count) : 
    fb::game::item(master, listener, count)
{}

fb::game::consume::consume(const consume& right) : 
    fb::game::item(right)
{}

fb::game::consume::~consume()
{}

bool fb::game::consume::active()
{
    if(this->_count == 0)
        return false;
    
    this->_count--;
    
    auto listener = this->_owner->get_listener<fb::game::session::listener>();
    if(listener != nullptr)
    {
        listener->on_item_update(*this->_owner, this->_owner->items.index(*this));
        listener->on_action(*this->_owner, fb::game::action::EAT, fb::game::duration::DURATION_EAT, fb::game::sound::EAT);
    }

    if(this->empty())
        this->_owner->items.remove(*this, -1, item::delete_attr::DELETE_EAT);
    return true;
}