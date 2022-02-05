#include <array>
#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <locale>
#include <codecvt>
#include <set>
#include <string>
#include <vector>

static const std::map<int, int> special { {'1','!'}, {'2','@'}, {'3','#'}, {'4','$'}, {'5','%'}, {'6','^'}, {'7','&'}, {'8','*'}, {'9','('}, {'0',')'} };

//std::string exec(const wchar_t* cmd) {
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;

#ifdef _WIN32
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
#else
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
#endif

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Get variants with one shift
std::set<std::wstring> variantsOneShift(std::wstring&& base)
{
    auto shift = [](int c) {
        if (::iswlower(c))
            return (int)::towupper(c);
        if (::iswupper(c))
            return (int)::tolower(c);
        if (::iswdigit(c))
            return (int)special.at(c);
        return (int)'+';
    };

    std::set<std::wstring> out({base});
    for(int i=0; i<base.length(); ++i)
    {
        std::wstring mod(base);
        mod[i] = shift(mod[i]);
        out.insert(mod);
    }

    return out;
}

std::set<std::wstring> variants(std::wstring&& base)
{
    // Get variants with one shift
    return variantsOneShift(std::move(base));
    // Get variants with starting caps-lock - n signs
    // Get variants with one alt
    // Get variants with two alts
    // Get variants with double letter
    // Switch special sign with two alts
};

std::set<std::wstring> variants(const std::wstring& base)
{
    return variants(std::wstring(base));
};

int main()
{
#ifdef _WIN32
//    const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-01-22T23-28-24.373Z--013efef911d47e09b85f9df956e6565d0f828622 "; //dupa88ąść
//    const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-01-29T09-15-31.463Z--a07877a94b37d8cdc60d94432db84fc3affdf907 "; //dupa88ąść
const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-02-05T12-55-32.284Z--7db20550c30c76620b3b9f14945eb691ceea551d "; //dupa88ąśćę#
//const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-01-29T10-04-59.453Z--1537ecb79590c6de59a8bbbc9322c9755796c1b8 "; //dupa88asc
#else
    const std::string command = "python3 /home/space/projects/decrypt-ethereum-keyfile/main.py /home/space/projects/scrypt_password_guesser/UTC--2022-01-22T23-28-24.373Z--013efef911d47e09b85f9df956e6565d0f828622 ";
#endif

    const std::string pass = std::string("dupa88ąśćę#$");

    const std::wstring wPass{std::begin(pass), std::end(pass)};
    const std::wstring wCommand{std::begin(command), std::end(command)};

//    const auto result = exec((wCommand + wPass).c_str());
    const auto result = exec((command + pass).c_str());

    if (result.starts_with("Password verified.\n"))
        std::cout << "PASS" << std::endl;
    else
        std::cout << result << std::endl;

    auto var = variants(wPass);
    for (auto v : var)
    {
        const std::string s{std::begin(v), std::end(v)};
        std::cout << s << std::endl;
    }

    return 0;
}
