#include "register.h"

string hashPassword(const string &password)
{
    char hashed[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(hashed,
        password.c_str(),
        password.size(),
        crypto_pwhash_OPSLIMIT_MODERATE,
        crypto_pwhash_MEMLIMIT_MODERATE) != 0)
    {
        throw runtime_error("Out of memory while hashing");
    }

    return string(hashed);
}

bool verifyPassword(const string &password, const string &hashed)
{
    if (crypto_pwhash_str_verify(hashed.c_str(), password.c_str(), password.size()) == 0) {
        return true;
    }

    return false;
}
