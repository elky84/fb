#include <fb/game/item.h>

fb::game::weapon::model::model(const fb::game::weapon::model::config& config) : 
    fb::game::equipment::model(config),
    damage_range(config.small, config.large),
    sound(config.sound),
    spell(config.spell)
{ }


fb::game::weapon::model::~model()
{ }

fb::game::item::ATTRIBUTE fb::game::weapon::model::attr() const
{
    return item::ATTRIBUTE::WEAPON;
}

fb::game::weapon::types fb::game::weapon::model::weapon_type() const
{
    switch(this->look / 10000)
    {
    case 0:
        return fb::game::weapon::types::NORMAL;

    case 1:
        return fb::game::weapon::types::SPEAR;

    case 2:
        return fb::game::weapon::types::BOW;

    case 3:
        return fb::game::weapon::types::FAN;

    default:
        return fb::game::weapon::types::UNKNOWN;
    }
}


fb::game::weapon::weapon(fb::game::context& context, const model* model) : 
    equipment(context, model)
{ }

fb::game::weapon::weapon(const weapon& right) : 
    equipment(right)
{ }

fb::game::weapon::~weapon()
{ }

std::string fb::game::weapon::mid_message() const
{
    std::stringstream       sstream;
    auto                    model = this->based<fb::game::weapon>();
    
    sstream << "파괴력: 　　 S:　" << std::to_string(model->damage_range.small.min) << 'm' << std::to_string(model->damage_range.small.max) << std::endl;
    sstream << "　　　  　 　L:　" << std::to_string(model->damage_range.large.min) << 'm' << std::to_string(model->damage_range.large.max) << std::endl;
    return sstream.str();
}