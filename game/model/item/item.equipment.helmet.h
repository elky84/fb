#ifndef __ITEM_EQUIPMENT_HELMET_H__
#define __ITEM_EQUIPMENT_HELMET_H__

#include "model/item/item.equipment.h"

namespace fb { namespace game {
    
class helmet : public equipment
{
#pragma region forward nested declaration
public:
    class master;
#pragma endregion

#pragma region constructor / destructor
public:
    helmet(const master* master, listener* listener);
    helmet(const helmet& right);
    ~helmet();
#pragma endregion
};


#pragma region nested class
class helmet::master : public equipment::master
{
public:
    master(uint32_t id, const std::string& name, uint16_t look, uint16_t dress, uint8_t color = 0);
    ~master();

public:
    virtual fb::game::item::attrs       attr() const;
    virtual fb::game::item*             make(fb::game::item::listener* listener) const;
};
#pragma endregion

} }

#endif // !__ITEM_EQUIPMENT_HELMET_H__