#ifndef __FB_LOGIN_H__
#define __FB_LOGIN_H__

#include <string>
#include <iostream>
#include <memory>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <regex>
#include <ctime>
#include <zlib.h>
#include <openssl/sha.h>
#include <fb/core/db.h>
#include <fb/core/socket.h>
#include <fb/login/session.h>
#include <fb/login/gateway.h>
#include <fb/protocol/login.h>
#include <fb/protocol/internal.h>
#include <fb/core/acceptor.h>
#include <fb/core/string.h>

#define MAX_NXCLUB_SIZE     14

namespace fb { namespace login {


class login_exception : public std::runtime_error
{
private:
    uint8_t                 _exc_type;

public:
    login_exception(uint8_t type, const std::string& what) : std::runtime_error(what), _exc_type(type) { }

public:
    uint8_t                 type() const { return this->_exc_type; }
};

class id_exception : public login_exception
{
public:
    id_exception(const std::string& what) : login_exception(0x0E, what) { }
};

class pw_exception : public login_exception
{
public:
    pw_exception(const std::string& what) : login_exception(0x0F, what) { }
};

class newpw_exception : public login_exception
{
public:
    newpw_exception(const std::string& what) : login_exception(0x05, what) { }
};

class btd_exception : public login_exception
{
public:
    btd_exception() : login_exception(0x1F, fb::login::message::account::INVALID_BIRTHDAY) { }
};

class context : public fb::acceptor<fb::login::session>
{
public:
    using unique_session = std::unique_ptr<fb::login::session>;

private:
    fb::protocol::login::response::agreement    _agreement = CP949(fb::config::get()["agreement"].asString(), PLATFORM::Both);
    std::vector<std::string>                    _forbiddens;
    std::vector<unique_session>                 _sessions;
    fb::db::context<session>                    _db;

public:
    context(boost::asio::io_context& context, uint16_t port);
    ~context();

private:
    static bool                 is_hangul(const std::string& str);
    bool                        is_forbidden(const std::string& str) const;
    std::string                 sha256(const std::string& data) const;
    void                        assert_account(const std::string& id, const std::string& pw) const;

    // override
protected:
    bool                        decrypt_policy(uint8_t) const final;
    fb::login::session*         handle_accepted(fb::socket<fb::login::session>&) final;
    bool                        handle_connected(fb::socket<fb::login::session>&) final;
    bool                        handle_disconnected(fb::socket<fb::login::session>&) final;
    void                        handle_internal_connected() final;

public:
    fb::task<bool>              handle_in_shutdown(fb::internal::socket<>&, const fb::protocol::internal::response::shutdown&);

public:
    fb::task<bool>              handle_agreement(fb::socket<fb::login::session>&, const fb::protocol::login::request::agreement&);
    fb::task<bool>              handle_create_account(fb::socket<fb::login::session>&, const fb::protocol::login::request::account::create&);
    fb::task<bool>              handle_account_complete(fb::socket<fb::login::session>&, const fb::protocol::login::request::account::complete&);
    fb::task<bool>              handle_login(fb::socket<fb::login::session>&, const fb::protocol::login::request::login&);
    fb::task<bool>              handle_change_password(fb::socket<fb::login::session>&, const fb::protocol::login::request::account::change_pw&);
};

} }

#endif // !__FB_LOGIN_H__
