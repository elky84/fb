#include <fb/game/object.h>
#include <fb/game/map.h>
#include <fb/game/context.h>

fb::game::object::model::model(const fb::game::object::model::config& config) : 
    name(config.name),
    look(config.look),
    color(config.color)
{ }

fb::game::object::model::~model()
{ }

uint8_t fb::game::object::model::dialog_look_type() const
{
    return this->look > 0xBFFF ? 0x02 : 0x01;
}

fb::game::object::types fb::game::object::model::type() const
{
    return types::UNKNOWN;
}


// fb::game::object
fb::game::object::object(fb::game::context& context, const model* model, const fb::game::object::config& config) : 
    luable(config.id),
    context(context),
    _listener(&context),
    _sequence(config.id),
    _model(model),
    _position(config.position),
    _direction(config.direction),
    _map(config.map),
    buffs(*this)
{
    this->_listener->on_create(*this);
}

fb::game::object::object(const object& right) :
    object(right.context, right._model, 
        fb::game::object::config 
        {
            .id = right._sequence,
            .position = right._position,
            .direction = right._direction,
            .map = right._map
        })
{ }

fb::game::object::~object()
{
    this->_listener->on_destroy(*this);
    this->map(nullptr);
}

const fb::game::object::model* fb::game::object::based() const
{
    return this->_model;
}

bool fb::game::object::is(object::types type) const
{
    auto mine = this->type();
    return (type & mine) == mine;
}

const std::string& fb::game::object::name() const
{
    return this->_model->name;
}

uint16_t fb::game::object::look() const
{
    return this->_model->look;
}

uint8_t fb::game::object::color() const
{
    return this->_model->color;
}

fb::game::object::types fb::game::object::type() const
{
    if(this->_model == nullptr)
        return fb::game::object::types::UNKNOWN;
    else
        return this->_model->type();
}

void fb::game::object::destroy()
{
    this->context.destroy(*this);
}

const fb::game::point16_t& fb::game::object::position() const
{
    return this->_position;
}

bool fb::game::object::position(uint16_t x, uint16_t y, bool refresh)
{
    if(this->_map == nullptr)
        return false;

    if(this->_position.x == x && this->_position.y == y)
        return true;

    auto before = this->_position;
    this->_position.x = std::max(0, std::min(this->_map->width() - 1, int32_t(x)));
    this->_position.y = std::max(0, std::min(this->_map->height() - 1, int32_t(y)));

    if(refresh)
        this->on_hold();

    this->_map->update(*this);
    
    auto nears_before = this->_map->nears(before);
    auto nears_after = this->_map->nears(this->_position);

    {
        // 내 이전 위치에서 내 시야에 보이는 오브젝트들
        auto befores = this->showings(nears_before, before);
        std::sort(befores.begin(), befores.end());

        // 내 현재 위치에서 내 시야에 보이는 오브젝트들
        auto afters = this->showings(nears_after, this->_position);
        std::sort(afters.begin(), afters.end());

        // 내가 이동한 뒤 내 시야에서 사라진 오브젝트들
        auto hides = std::vector<fb::game::object*>();
        std::set_difference(befores.begin(), befores.end(), afters.begin(), afters.end(), std::inserter(hides, hides.begin()));
        for(auto x : hides)
            this->_listener->on_hide(*x, *this);

        // 내가 이동한 뒤 내 시야에서 나타난 오브젝트들
        auto shows = std::vector<fb::game::object*>();
        std::set_difference(afters.begin(), afters.end(), befores.begin(), befores.end(), std::inserter(shows, shows.begin()));
        for(auto x : shows)
            this->_listener->on_show(*x, *this, false);

        if(refresh)
        {
            // 내가 이동한 뒤 내 시야에 여전히 남은 오브젝트들
            auto stay = std::vector<fb::game::object*>();
            std::set_difference(afters.begin(), afters.end(), shows.begin(), shows.end(), std::inserter(stay, stay.begin()));
            for(auto x : stay)
                this->_listener->on_show(*x, *this, false);
        }
    }

    {
        // 내 이전 위치에서 내가 포함된 시야를 가진 오브젝트들
        //auto befores = fb::game::object::showns(nears_before, *this, fb::game::object::types::UNKNOWN/* , true, false */);
        auto befores = this->showns(nears_before, before);
        std::sort(befores.begin(), befores.end());

        // 내 현재 위치에서 내가 포함된 시야를 가진 오브젝트들
        //auto afters = fb::game::object::showns(nears_after, *this, fb::game::object::types::UNKNOWN/* , false, false */);
        auto afters = this->showns(nears_after, this->_position);
        std::sort(afters.begin(), afters.end());

        // 내가 이동한 뒤 자기 시야에서 내가 사라진 오브젝트들
        auto hides = std::vector<fb::game::object*>();
        std::set_difference(befores.begin(), befores.end(), afters.begin(), afters.end(), std::inserter(hides, hides.begin()));
        for(auto x : hides)
            this->_listener->on_hide(*this, *x);

        // 내가 이동한 뒤 자기 시야에서 내가 나타난 오브젝트들
        auto shows = std::vector<fb::game::object*>();
        std::set_difference(afters.begin(), afters.end(), befores.begin(), befores.end(), std::inserter(shows, shows.begin()));
        for(auto x : shows)
            this->_listener->on_show(*this, *x, false);

        if(refresh)
        {
            // 내가 이동한 뒤 자기 시야에 여전히 내가 포함된 시야를 가진 오브젝트들
            auto stay = std::vector<fb::game::object*>();
            std::set_difference(afters.begin(), afters.end(), shows.begin(), shows.end(), std::inserter(stay, stay.begin()));
            for(auto x : stay)
                this->_listener->on_show(*this, *x, false);
        }
    }

    return true;
}

bool fb::game::object::position(const point16_t position, bool refresh)
{
    return this->position(position.x, position.y, refresh);
}

bool fb::game::object::move()
{
    return this->move(this->_direction);
}

bool fb::game::object::move(fb::game::DIRECTION_TYPE direction)
{
    if(this->_map == nullptr)
        return false;

    auto after = this->_position;
    switch(direction)
    {
    case fb::game::DIRECTION_TYPE::TOP:
        after.y--;
        break;

    case fb::game::DIRECTION_TYPE::BOTTOM:
        after.y++;
        break;

    case fb::game::DIRECTION_TYPE::LEFT:
        after.x--;
        break;

    case fb::game::DIRECTION_TYPE::RIGHT:
        after.x++;
        break;
    }

    if(this->_map->movable(after) == false)
        return false;

    if(this->direction(direction) == false)
        return false;

    auto before = this->_position;
    this->position(after);
    this->_listener->on_move(*this, before);

    return true;
}

const fb::game::point16_t fb::game::object::position_forward() const
{
    return this->position_forward(this->_direction);
}

const fb::game::point16_t fb::game::object::position_forward(fb::game::DIRECTION_TYPE direction) const
{
    auto current = point16_t(this->_position);
    auto forward = point16_t(current);
    forward.forward(direction);
    if(this->_map->movable(forward))
        return forward;


    auto left = point16_t(current);
    left.left(this->_direction);
    if(this->_map->movable(left))
        return left;


    auto right = point16_t(current);
    right.right(this->_direction);
    if(this->_map->movable(right))
        return right;

    auto backward = point16_t(current);
    backward.backward(this->_direction);
    if(this->_map->movable(backward))
        return backward;

    return current;
}

uint16_t fb::game::object::x() const
{
    return this->_position.x;
}

bool fb::game::object::x(uint16_t value)
{
    return this->position(value, this->_position.y);
}

uint16_t fb::game::object::y() const
{
    return this->_position.y;
}

bool fb::game::object::y(uint16_t value)
{
    return this->position(this->_position.x, value);
}

fb::game::DIRECTION_TYPE fb::game::object::direction() const
{
    return this->_direction;
}

bool fb::game::object::direction(fb::game::DIRECTION_TYPE value)
{
    if(value != fb::game::DIRECTION_TYPE::LEFT && 
       value != fb::game::DIRECTION_TYPE::TOP && 
       value != fb::game::DIRECTION_TYPE::RIGHT && 
       value != fb::game::DIRECTION_TYPE::BOTTOM)
        return false;

    if(this->_direction == value)
        return true;

    this->_direction = value;
    this->_listener->on_direction(*this);

    return true;
}

fb::game::map* fb::game::object::map() const
{
    return this->_map;
}

bool fb::game::object::sight(const point16_t& position) const
{
    return fb::game::object::sight(this->_position, position, this->_map);
}

bool fb::game::object::sight(const fb::game::object& object) const
{
    if(this->_map == nullptr)
        return false;

    if(this->_map != object.map())
        return false;

    if(object.visible() == false)
        return false;

    return this->sight(object._position);
}

bool fb::game::object::sector(fb::game::sector* sector)
{
    if(this->_sector == sector)
        return false;

    if(this->_sector != nullptr)
        this->_sector->erase(*this);

    this->_sector = sector;
    if(sector != nullptr)
        sector->push(*this);
    return true;
}

fb::game::sector* fb::game::object::sector()
{
    return this->_sector;
}

bool fb::game::object::sight(const point16_t me, const point16_t you, const fb::game::map* map)
{
    point16_t begin, end;

    if(me.x <= fb::game::map::HALF_SCREEN_WIDTH) // 최좌측
    {
        begin.x = 0;
        end.x   = fb::game::map::MAX_SCREEN_WIDTH;
    }
    else if(me.x >= map->width() - fb::game::map::HALF_SCREEN_WIDTH) // 최우측
    {
        begin.x = std::max(int32_t(0), int32_t(map->width() - fb::game::map::MAX_SCREEN_WIDTH - 1));
        end.x   = std::max(int32_t(0), int32_t(map->width() - 1));
    }
    else
    {
        begin.x = std::max(int32_t(0), int32_t(me.x - fb::game::map::HALF_SCREEN_WIDTH - 1));
        end.x   = std::max(int32_t(0), int32_t(me.x + fb::game::map::HALF_SCREEN_WIDTH + 1));
    }


    if(me.y <= fb::game::map::HALF_SCREEN_HEIGHT) // 최상단
    {
        begin.y = 0;
        end.y   = fb::game::map::MAX_SCREEN_HEIGHT;
    }
    else if(me.y >= map->height() - fb::game::map::HALF_SCREEN_HEIGHT) // 최하단
    {
        begin.y = std::max(int32_t(0), int32_t(map->height() - fb::game::map::MAX_SCREEN_HEIGHT - 1));
        end.y   = std::max(int32_t(0), map->height() - 1);
    }
    else
    {
        begin.y = std::max(int32_t(0), int32_t(me.y - fb::game::map::HALF_SCREEN_HEIGHT - 1));
        end.y   = std::max(int32_t(0), int32_t(me.y + fb::game::map::HALF_SCREEN_HEIGHT + 1));
    }

    return begin.x <= you.x && end.x >= you.x &&
        begin.y <= you.y && end.y >= you.y;
}

fb::task<bool> fb::game::object::__map(fb::game::map* map, const point16_t position, fb::awaitable<bool>* awaitable)
{
    try
    {
        if(this->_map_lock)
            co_return false;

        if(map == nullptr)
        {
            if(this->_map != nullptr)
            {
                this->_map->objects.remove(*this);
                for (auto x : this->_map->nears(this->_position))
                {
                    if (x != this)
                        this->_listener->on_hide(*x, *this);
                }

                this->_map = nullptr;
            }
            this->_position = point16_t(1, 1); // 가상계 위치
            this->sector(nullptr);
            auto result = true;
            if(awaitable != nullptr)
            {
                awaitable->result = &result;
                awaitable->handler.resume();
            }
            co_return result;
        }

        if(this->_map == map)
        {
            this->position(position, true);
            auto result = true;
            if(awaitable != nullptr)
            {
                awaitable->result = &result;
                awaitable->handler.resume();
            }
            co_return result;
        }

        if(map->active == false)
        {
            auto result = false;
            if(awaitable != nullptr)
            {
                awaitable->result = &result;
                awaitable->handler.resume();
            }
            co_return result;
        }

        auto before = this->_map;

        if(this->_map != nullptr)
        {
            co_await this->co_map(nullptr);
        }
        this->_map_lock = true;

        auto thread = this->context.thread(map);
        if(thread != nullptr && thread != this->context.current_thread())
            co_await thread->precedence();

        this->_map = map;
        this->_position = position;
        this->_map->update(*this);
        this->_map->objects.add(*this);
        this->_listener->on_map_changed(*this, before, map);

        for(auto x : map->nears(this->_position))
        {
            if(x == this)
                continue;

            this->_listener->on_show(*this, *x, false);
            this->_listener->on_show(*x, *this, false);
        }

        auto result = true;
        if(awaitable != nullptr)
        {
            awaitable->result = &result;
            awaitable->handler.resume();
        }
        this->_map_lock = false;
        co_return result;
    }
    catch(std::exception& e)
    {

    }
    catch(...)
    {
        
    }
}

fb::awaitable<bool> fb::game::object::co_map(fb::game::map* map, const point16_t& position)
{
    return fb::awaitable<bool>([this, map, position](auto& awaitable)
    {
        this->__map(map, position, &awaitable);
    });
}

fb::awaitable<bool> fb::game::object::co_map(fb::game::map* map)
{
    return this->co_map(map, point16_t(0, 0));
}

bool fb::game::object::map(fb::game::map* map, const point16_t& position)
{
    auto task = this->__map(map, position);
    if (task.done())
        return task.value();

    return true;
}

bool fb::game::object::map(fb::game::map* map)
{
    return this->map(map, point16_t(0, 0));
}

fb::game::object* fb::game::object::side(fb::game::DIRECTION_TYPE direction, fb::game::object::types type) const
{
    fb::game::map* map = this->_map;
    if(map == nullptr)
        return nullptr;


    point16_t front = this->position();
    switch(direction)
    {
    case fb::game::DIRECTION_TYPE::TOP:
        front.y--;
        break;

    case fb::game::DIRECTION_TYPE::BOTTOM:
        front.y++;
        break;

    case fb::game::DIRECTION_TYPE::LEFT:
        front.x--;
        break;

    case fb::game::DIRECTION_TYPE::RIGHT:
        front.x++;
        break;
    }

    if(map->existable(front) == false)
        return nullptr;

    auto nears = map->nears(this->_position, type);
    auto found = std::find_if
    (
        nears.begin(), nears.end(), 
        [&front] (auto x)
        {
            return x->position() == front;
        }
    );

    return found != nears.end() ?
        *found : nullptr;
}

std::vector<fb::game::object*> fb::game::object::sides(fb::game::DIRECTION_TYPE direction, fb::game::object::types type) const
{
    auto result = std::vector<fb::game::object*>();
    try
    {
        auto map = this->_map;
        if(map == nullptr)
            throw std::exception();

        auto front = this->position();
        switch(direction)
        {
        case fb::game::DIRECTION_TYPE::TOP:
            front.y--;
            break;

        case fb::game::DIRECTION_TYPE::BOTTOM:
            front.y++;
            break;

        case fb::game::DIRECTION_TYPE::LEFT:
            front.x--;
            break;

        case fb::game::DIRECTION_TYPE::RIGHT:
            front.x++;
            break;
        }

        if(map->existable(front) == false)
            throw std::exception();

        auto nears = map->nears(this->_position, type);
        std::copy_if
        (
            nears.begin(), nears.end(), std::back_inserter(result),
            [&front] (auto x)
            {
                return x->position() == front;
            }
        );
    }
    catch(std::exception&)
    { }

    return std::move(result);
}

fb::game::object* fb::game::object::forward(fb::game::object::types type) const
{
    return this->side(this->_direction, type);
}

std::vector<fb::game::object*> fb::game::object::forwards(fb::game::object::types type) const
{
    return this->sides(this->_direction, type);
}

std::vector<fb::game::object*> fb::game::object::showns(object::types type) const
{
    if(this->_map == nullptr)
        return std::vector<fb::game::object*> { };

    return this->showns(this->_map->nears(this->_position), this->_position, type);
}

std::vector<fb::game::object*> fb::game::object::showns(const std::vector<object*>& source, const point16_t& position, object::types type) const
{
    auto                    objects = std::vector<fb::game::object*>();
    if(this->_map == nullptr)
        return objects;

    std::copy_if
    (
        source.begin(), source.end(), std::back_inserter(objects), 
        [&](auto x)
        {
            if(this == x)
                return false;

            if(x->visible() == false)
                return false;

            if(type != object::types::UNKNOWN && x->is(type) == false)
                return false;

            return sight(position, x->_position, this->_map);
        }
    );

    return std::move(objects);
}

std::vector<object*> fb::game::object::showings(object::types type) const
{
    if(this->_map == nullptr)
        return std::vector<object*> { };
    else
        return this->showings(this->_map->nears(this->_position), this->_position, type);
}

std::vector<object*> fb::game::object::showings(const std::vector<object*>& source, const point16_t& position, object::types type) const
{
    auto                    objects = std::vector<fb::game::object*>();
    if(this->_map == nullptr)
        return objects;

    std::copy_if
    (
        source.begin(), source.end(), std::back_inserter(objects), 
        [&](auto x)
        {
            if(x == this)
                return false;

            if(x->visible() == false)
                return false;

            if(type != object::types::UNKNOWN && x->is(type) == false)
                return false;

            return x->sight(position);
        }
    );

    return std::move(objects);
}

bool fb::game::object::visible() const
{
    return this->_visible;
}

void fb::game::object::visible(bool value)
{
    if(this->_visible == value)
        return;

    this->_visible = value;
}

double fb::game::object::distance(const object& right) const
{
    return std::sqrt(this->distance_sqrt(right));
}

uint32_t fb::game::object::distance_sqrt(const object& right) const
{
    return (uint32_t)std::pow(this->_position.x - right._position.x, 2) + 
        (uint32_t)std::pow(this->_position.y - right._position.y, 2);
}

bool fb::game::object::operator==(const object& right) const
{
    return this->_map == right._map &&
        this->sequence() == right.sequence();
}

bool fb::game::object::operator!=(const object& right) const
{
    return !((*this) == right);
}