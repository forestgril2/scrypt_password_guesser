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
std::set<std::string> variantsOneShift(std::string&& base)
{
    auto shift = [](char c) {
        if (::islower(c))
            return (char)::toupper(c);
        if (::isupper(c))
            return (char)::tolower(c);
        if (::isdigit(c) || special.contains(c))
            return (char)special.at(c);

        return (char)'+';
    };

    std::set<std::string> out({base});
    for(int i=0; i<base.length(); ++i)
    {
        std::string mod(base);
        mod[i] = shift(mod[i]);
        out.insert(mod);
    }

    return out;
}

std::set<std::string> variants(std::string&& base)
{
    // Get variants with one shift
    return variantsOneShift(std::move(base));
    // Get variants with starting caps-lock - n signs
    // Get variants with one alt
    // Get variants with two alts
    // Get variants with double letter
    // Switch special sign with two alts
};

std::set<std::string> variants(const std::string& base)
{
    return variants(std::string(base));
};

int main(int argc, char *argv[])
{
 // Set the current locale to UTF-8
  std::setlocale(LC_ALL, "en_US.UTF-8");

#ifdef _WIN32
//    const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-01-22T23-28-24.373Z--013efef911d47e09b85f9df956e6565d0f828622 "; //dupa88ąść
//    const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-01-29T09-15-31.463Z--a07877a94b37d8cdc60d94432db84fc3affdf907 "; //dupa88ąść
// const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-02-05T12-55-32.284Z--7db20550c30c76620b3b9f14945eb691ceea551d "; //dupa88ąśćę#
//const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-01-29T10-04-59.453Z--1537ecb79590c6de59a8bbbc9322c9755796c1b8 "; //dupa88asc
const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-10-04T08-04-22.071Z--54278f2fe320bec308cefc4640e50479e9ab1791 "; //oaeóąę#$*
#else
    const std::string command = "python3 /home/space/projects/decrypt-ethereum-keyfile/main.py /home/space/projects/scrypt_password_guesser/UTC--2022-01-22T23-28-24.373Z--013efef911d47e09b85f9df956e6565d0f828622 ";
#endif

    // const std::string pass = std::string("dupa88ąśćę#$");
    const std::string pass = std::string("oaeóąę#$*");
    std::cout << "Init pass: " << pass << std::endl;

    //get a wide char version of the pass
    std::wstring wpass;
    wpass.assign(pass.begin(), pass.end());
    std::wcout << "wpass: " << wpass << std::endl;
    
    //get a copy of pass to modify
    std::string passCopy(pass);
    wchar_t wc;
    std::mbstowcs(&wc, &passCopy[3], 1);  // convert to wide character
    wc = towupper(wc);                    // convert to upper case
    // Print wc now
    std::wcout << "wc: " << wc << std::endl;
    // Convert the uppercase wide character back to UTF-8
    char utf8_char_upper[5];
    std::wcstombs(utf8_char_upper, &wc, 5);  // convert to UTF-8

    // Replace the 2nd character in the string with the uppercase version
    passCopy[3] = utf8_char_upper[1];
    std::cout << "Init passCopy: " << passCopy << std::endl;

    const auto result = exec((command + pass).c_str());

    if (result.starts_with("Password verified.\n"))
        std::cout << "PASS" << std::endl;
    else
        std::cout << result << std::endl;

    auto var = variants(pass);
    for (auto v : var)
    {
        const std::string s{std::begin(v), std::end(v)};
        std::cout << s << std::endl;
    }

    return 0;
}
