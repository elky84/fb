#include "model/board/board.h"


fb::game::board::article::article(uint16_t id, const std::string& title, const std::string& content, const std::string& writer, uint8_t month, uint8_t day, uint8_t color, bool deleted) : 
    _id(id),
    _title(title),
    _content(content),
    _writer(writer),
    _month(month), _day(day),
    _color(color),
    _deleted(deleted)
{}

fb::game::board::article::~article()
{}

uint16_t fb::game::board::article::id() const
{
    return this->_id;
}

const std::string& fb::game::board::article::title() const
{
    return this->_title;
}

const std::string& fb::game::board::article::content() const
{
    return this->_content;
}

const std::string& fb::game::board::article::writer() const
{
    return this->_writer;
}

uint8_t fb::game::board::article::month() const
{
    return this->_month;
}

uint8_t fb::game::board::article::day() const
{
    return this->_day;
}

uint8_t fb::game::board::article::color() const
{
    return this->_color;
}

bool fb::game::board::article::deleted() const
{
    return this->_deleted;
}

void fb::game::board::article::deleted(bool value)
{
    this->_deleted = value;
}


//
// section methods
//
fb::game::board::section::section(const std::string & title) : 
    _title(title)
{
}

fb::game::board::section::~section()
{
    for(auto article : *this)
        delete article;
}

std::vector<fb::game::board::article*>::iterator fb::game::board::section::it(uint16_t id)
{
    for(auto i = this->begin(); i != this->end(); i++)
    {
        const auto& article = *i;
        if(article->id() == id)
            return i;
    }

    return this->end();
}

std::vector<fb::game::board::article*>::const_iterator fb::game::board::section::const_it(uint16_t id) const
{
    for(auto i = this->cbegin(); i != this->cend(); i++)
    {
        const auto& article = *i;
        if(article->id() == id)
            return i;
    }

    return this->cend();
}

const std::string& fb::game::board::section::title() const
{
    return this->_title;
}

fb::game::board::article* fb::game::board::section::add(const std::string& title, const std::string& content, const session& session, uint8_t color)
{
    return this->add(title, content, session.name(), color);
}

fb::game::board::article* fb::game::board::section::add(const std::string& title, const std::string& content, const std::string& writer, uint8_t color)
{
    auto time = std::time(0);
    auto now = std::localtime(&time);

    auto allocated = new article(uint16_t(this->size()+1), title, content, writer, now->tm_mon + 1, now->tm_mday, color);
    this->push_back(allocated);

    return allocated;
}

bool fb::game::board::section::remove(uint16_t id)
{
    auto found = this->it(id);
    if(found == this->end())
        return false;

    this->erase(found);
    return true;
}

const fb::game::board::article* fb::game::board::section::at(uint32_t index) const
{
    if(index > this->size() - 1)
        return nullptr;

    const auto article = std::vector<board::article*>::at(index);
    if(article->deleted())
        return nullptr;

    return article;
}

const fb::game::board::article* fb::game::board::section::find(uint16_t id) const
{
    auto found = this->const_it(id);
    if(found == this->cend())
        return nullptr;

    return *found;
}



//
// board methods
//
fb::game::board::board()
{
}

fb::game::board::~board()
{
    for(auto section : this->_sections)
        delete section;
}

const std::vector<fb::game::board::section*>& fb::game::board::sections() const
{
    return this->_sections;
}

fb::game::board::section* fb::game::board::at(uint32_t index) const
{
    if(index > this->_sections.size() - 1)
        return nullptr;

    return this->_sections.at(index);
}

fb::game::board::section* fb::game::board::add(const std::string& name)
{
    auto section = new board::section(name);
    this->_sections.push_back(section);

    return section;
}

void fb::game::board::remove(uint32_t index)
{
    this->_sections.erase(this->_sections.begin() + index);
}

void fb::game::board::clear()
{
    for(auto section : this->_sections)
        delete section;

    this->_sections.clear();
}

fb::game::board::section* fb::game::board::operator[](uint32_t index)
{
    return this->at(index);
}

fb::ostream fb::game::board::make_sections_stream() const
{
    ostream                 ostream;

    ostream.write_u8(0x31)
        .write_u8(0x01)
        .write_u16((uint16_t)this->_sections.size());

    for(int i = 0; i < this->_sections.size(); i++)
    {
        ostream.write_u16(i)
            .write(this->_sections[i]->title());
    }

    return ostream;
}

fb::ostream fb::game::board::make_articles_stream(uint16_t section_id, uint16_t offset) const
{
    ostream                 ostream;
    const auto              section = this->_sections[section_id];

    ostream.write_u8(0x31)
        .write_u8(0x02)
        .write_u8(button_enabled::NEXT | button_enabled::WRITE)
        .write_u16(section_id)
        .write(section->title());


    if(offset == 0x7FFF)
        offset = uint16_t(section->size() - 1);
    auto                    reverse_offset = uint16_t(section->size() - (offset + 1));

    auto                    count = std::min(size_t(20), section->size() - reverse_offset);
    ostream.write_u8((uint8_t)count);

    for(auto i = section->rbegin() + reverse_offset; i != section->rend(); i++)
    {
        const auto          article = (*i);
        if(article->deleted())
            continue;

        ostream.write_u8(0x00)
            .write_u16(article->id())
            .write(article->writer())
            .write_u8(article->month())
            .write_u8(article->day())
            .write(article->title());

        if(--count == 0)
            break;
    }

    ostream.write_u8(0x00);
    return ostream;
}

fb::ostream fb::game::board::make_article_stream(uint16_t section_id, uint16_t article_id, const session& session) const
{
    try
    {
        auto                    section = this->at(section_id);
        if(section == nullptr)
            throw board::section::not_found_exception();

        auto                    article = section->find(article_id);
        if(article == nullptr)
            throw board::article::not_found_exception();

        ostream                 ostream;

        ostream.write_u8(0x31)
            .write_u8(0x03)
            .write_u8(button_enabled::NEXT | button_enabled::WRITE)
            .write_u8(0x00)
            .write_u16(article_id)
            .write(article->writer())
            .write_u8(article->month())
            .write_u8(article->day())
            .write(article->title())
            .write(article->content(), true)
            .write_u8(0x00);

        return ostream;
    }
    catch(std::exception& e)
    {
        return this->make_message_stream(e.what(), false);
    }
}

fb::ostream fb::game::board::make_delete_stream(uint16_t section_id, uint16_t article_id, const session& session) const
{
    try
    {
        auto                    section = this->at(section_id);
        if(section == nullptr)
            throw board::section::not_found_exception();

        auto                    article = section->find(article_id);
        if(article == nullptr)
            throw board::article::not_found_exception();

        if(session.name() != article->writer())
            throw board::auth_exception();

        if(section->remove(article->id()) == false)
            throw board::article::not_found_exception();

        return this->make_message_stream(message::board::SUCCESS_DELETE, true);
    }
    catch(std::exception& e)
    {
        return this->make_message_stream(e.what(), false);
    }
}

fb::ostream fb::game::board::make_message_stream(const std::string& message, bool success, bool refresh) const
{
    ostream                 ostream;

    ostream.write_u8(0x31)
        .write_u8(refresh ? 0x06 : 0x07)     // mail 관련 0x06인 것 같다. 확인 필요
        .write_u8(success)
        .write(message)
        .write_u8(0x00);

    return ostream;
}