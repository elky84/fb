#ifndef __SECTOR_H__
#define __SECTOR_H__

#include "model/object/object.h"
#include <set>

namespace fb { namespace game {

class map;
class session;
class sectors;

class sector : private std::vector<fb::game::object*>
{
public:
    using std::vector<fb::game::object*>::begin;
    using std::vector<fb::game::object*>::cbegin;
    using std::vector<fb::game::object*>::end;
    using std::vector<fb::game::object*>::cend;
    using std::vector<fb::game::object*>::erase;
    using std::vector<fb::game::object*>::at;
    using std::vector<fb::game::object*>::operator[];
    using std::vector<fb::game::object*>::size;

public:
    friend class fb::game::sectors;

private:
    const uint32_t                  _id;
    std::function<void(sector&)>    _state_changed;
    bool                            _activated;

public:
    sector(uint32_t id, std::function<void(sector&)> state_changed);
    ~sector();

public:
    void                            push(fb::game::object& object);
    void                            erase(fb::game::object& object);
    uint32_t                        id() const;
    bool                            activated() const;
};

class sectors
{
private:
    const size16_t                  _map_size;
    const size16_t                  _size;
    const uint32_t                  _rows, _columns;
    const uint32_t                  _count;
    std::map<uint32_t, sector*>     _pool;
    std::map<uint32_t, sector*>     _activated_cache;

public:
    sectors(const size16_t& map_size, const size16_t& size);
    ~sectors();

private:
    uint32_t                        index(const point16_t& position) const;
    std::set<sector*>               activated_sectors() const;

public:
    sector*                         at(const point16_t& position) const;
    sector*                         at(uint32_t index) const;
    uint32_t                        push(fb::game::object& object);

public:
    std::vector<sector*>            nears(uint32_t index) const;
    std::vector<sector*>            nears(const point16_t& pivot) const;
    std::vector<object*>            objects(const point16_t& pivot, fb::game::object::types type) const;
    std::vector<object*>            activated_objects(fb::game::object::types type) const;
};

} }

#endif // !__SECTOR_H__