#include "auth.service.h"

fb::login::service::auth::auth()
{
    const auto& config = fb::config();

    this->_connection = new connection("localhost", "root", "tmdgus12", "fb");

    // Parse agreement
    for(auto x : config["login"]["account option"]["forbidden"])
        this->_forbiddens.push_back(x.asString());
}

fb::login::service::auth::~auth()
{
    delete this->_connection;
}

bool fb::login::service::auth::is_hangul(const std::string& str)
{
    auto size = (uint8_t)MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), nullptr, 0) + 1;
    auto wide = new wchar_t[size];
    memset(wide, 0, sizeof(wchar_t) * size);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), wide, sizeof(wchar_t) * size);

    std::wregex re(L"^.+[가-힣]$");
    auto success = std::regex_search(wide, re);
    delete[] wide;

    return success;
}

bool fb::login::service::auth::is_forbidden(const char* str) const
{
    return std::any_of
    (
        this->_forbiddens.cbegin(), 
        this->_forbiddens.cend(), 
        [str](const std::string& x)
        {
            return x == str;
        }
    );
}

std::string fb::login::service::auth::sha256(const std::string& data) const
{
    auto buffer = new char[data.size()];
    memcpy(buffer, data.c_str(), data.size());

    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, buffer, data.size());

    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &ctx);
    
    std::stringstream sstream;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sstream << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return sstream.str();
}

bool fb::login::service::auth::exists(const std::string& name) const
{
    return this->_connection->query
    (
        "SELECT COUNT(*) FROM user WHERE name LIKE '%s'",
        name.c_str()
    ).get_value<int>() > 0;
}

void fb::login::service::auth::assert_client(fb::login::session& session)
{
    auto&                   istream = session.in_stream();
    auto                    cmd     = istream.read_u8();
    auto                    version = istream.read_u16();
    if(version != 0x0226)
        throw std::exception("클라이언트 버전이 맞지 않습니다.");

    auto                    national_key    = istream.read_u8();
    if(national_key != 0xD7)
        throw std::exception("국가가 올바르지 않습니다.");
}

void fb::login::service::auth::assert_account(const std::string& id, const std::string& pw) const
{
    if(id.length() < MIN_NAME_SIZE || id.length() > MAX_NAME_SIZE)
        throw id_exception(message::INVALID_NAME);

    // Name must be full-hangul characters
    if(fb::config()["login"]["account option"]["allow other language"].asBool() == false && this->is_hangul(id) == false)
        throw id_exception(message::INVALID_NAME);

    // Name cannot contains subcharacters in forbidden list
    if(this->is_forbidden(id.c_str()))
        throw id_exception(message::INVALID_NAME);


    // Read character's password
    if(pw.length() < MIN_PASSWORD_SIZE || pw.length() > MAX_PASSWORD_SIZE)
        throw pw_exception(message::PASSWORD_SIZE);
}

void fb::login::service::auth::create_account(const std::string& id, const std::string& pw)
{
    this->assert_account(id, pw);

    if(this->exists(id))
        throw id_exception(message::ALREADY_EXISTS);

    std::srand(std::time(nullptr));
    auto hp = 45 + std::rand() % 10;
    auto mp = 45 + std::rand() % 10;
    this->_connection->exec
    (
        "INSERT INTO user(name, pw, hp, base_hp, mp, base_mp) VALUES('%s', '%s', %d, %d, %d, %d)",
        id.c_str(), this->sha256(pw).c_str(), hp, hp, mp, mp
    );
    auto index = this->_connection->last_insert_id();

    auto result = this->_connection->query("SELECT id, name FROM user")
        .each([](int id, std::string name) 
            {
                std::cout << id << ' ' << name << std::endl;
                return true;
            });
}

void fb::login::service::auth::init_account(const std::string& id, uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature)
{
    this->_connection->exec
    (
        "UPDATE user SET look=%d, sex=%d, nation=%d, creature=%d WHERE name LIKE '%s'",
        hair, sex, nation, creature, id.c_str()
    );
}

void fb::login::service::auth::login(const std::string& id, const std::string& pw)
{
    this->assert_account(id, pw);
}

void fb::login::service::auth::change_pw(const std::string& id, const std::string& pw, const std::string& new_pw, uint32_t birthday)
{
    if(id.length() < MIN_NAME_SIZE || id.length() > MAX_NAME_SIZE)
        throw id_exception(message::INVALID_NAME);

    // Name must be full-hangul characters
    if(fb::config()["login"]["account option"]["allow other language"].asBool() == false && this->is_hangul(id.c_str()) == false)
        throw id_exception(message::INVALID_NAME);

    // Name cannot contains subcharacters in forbidden list
    if(this->is_forbidden(id.c_str()))
        throw id_exception(message::INVALID_NAME);

    if(pw.length() < MIN_PASSWORD_SIZE || pw.length() > MAX_PASSWORD_SIZE)
        throw pw_exception(message::PASSWORD_SIZE);

    auto found = this->_connection->query
    (
        "SELECT pw, birth FROM user WHERE name LIKE '%s'",
        id.c_str()
    );

    if(found.count() == 0)
        throw id_exception(message::NOT_FOUND_NAME);

    auto found_pw = found.get_value<std::string>(0);
    auto found_birth = found.get_value<uint32_t>(1);

    // TODO : 올바른 비밀번호인지 체크
    if(this->sha256(pw) != found_pw)
        throw pw_exception(message::INVALID_PASSWORD);

    if(new_pw.length() < MIN_PASSWORD_SIZE || new_pw.length() > MAX_PASSWORD_SIZE)
        throw newpw_exception(message::PASSWORD_SIZE);
    
    // TODO : 너무 쉬운 비밀번호인지 체크

    if(pw == new_pw)
        throw newpw_exception(message::NEW_PW_EQUALIZATION);

    if(birthday != found_birth)
        throw btd_exception();

    this->_connection->exec
    (
        "UPDATE user SET pw='%s' WHERE name LIKE '%s'",
        this->sha256(new_pw).c_str(), id.c_str()
    );
}