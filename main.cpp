#include <array>
#include <algorithm>
#include <wchar.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <locale>
#include <codecvt>
#include <set>
#include <string>
#include <vector>

std::map<wchar_t, wchar_t> special = {
    {L'0', L')'},
    {L'1', L'!'},
    {L'2', L'@'},
    {L'3', L'#'},
    {L'4', L'$'},
    {L'5', L'%'},
    {L'6', L'^'},
    {L'7', L'&'},
    {L'8', L'*'},
    {L'9', L'('},
};

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
    auto shift = [](wint_t c) {
        //  std::wcout << "c: " << c << " iswupper: " << ::iswupper(c) << " iswlower: " << ::iswlower(c) << std::endl;
         if (::iswupper(c))
             return ::towlower(c);
         else if (::iswlower(c))
         return ::towupper(c);
         else if (::iswdigit(c) || special.contains(c))
             return (wint_t)special.at(c);
        return (wint_t)L'+';
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
    auto variants = variantsOneShift(std::move(base));
    // Now get variants with two shifts
    std::set<std::wstring> twoShifts;
    for(auto v : variants)
    {
        auto v2 = variantsOneShift(std::move(v));
        twoShifts.insert(v2.begin(), v2.end());
    }
    //Merge them
    variants.insert(twoShifts.begin(), twoShifts.end());
    
    // Get variants with starting caps-lock - n signs
    // Get variants with one alt
    // Get variants with two alts
    // Get variants with double letter
    // Switch special sign with two alts
    return variants;
};

std::set<std::wstring> variants(const std::wstring& base)
{
    return variants(std::wstring(base));
};

std::wstring normal_string_to_wide_string(const std::string& str)
{
    std::wstring wstr;
    int len = std::strlen(str.c_str());
    for (int i = 0; i < len;)
    {
        wchar_t wc;
        int bytes_consumed = std::mbtowc(&wc, str.c_str() + i, len - i);
        if (bytes_consumed < 0)
        {
            // Error occurred
            break;
        }
        i += bytes_consumed;
        wstr += wc;
    }
    return wstr;
}

std::string wide_string_to_normal_string(const std::wstring& wstr)
{
    std::string str;
    for (wchar_t wc : wstr)
    {
        char mb[MB_LEN_MAX];
        int bytes_consumed = std::wctomb(mb, wc);
        if (bytes_consumed < 0)
        {
            // Error occurred
            break;
        }
        str += mb;
    }
    return str;
}

int main(int argc, char *argv[])
{
 // Set the current locale to UTF-8
  std::setlocale(LC_ALL, "en_US.UTF-8");

    // invert special mapping and attach it to the main map
    std::map<wchar_t, wchar_t> special_inv; // = special;
    for (auto& [k, v] : special)
        special_inv[v] = k;
    special.insert(special_inv.begin(), special_inv.end()); 
    
#ifdef _WIN32
const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-10-04T08-04-22.071Z--54278f2fe320bec308cefc4640e50479e9ab1791 "; //oaeóąę#$*
#else
    const std::string command = "python3 /home/space/projects/decrypt-ethereum-keyfile/main.py /home/space/projects/scrypt_password_guesser/UTC--2022-01-22T23-28-24.373Z--013efef911d47e09b85f9df956e6565d0f828622 ";
#endif

    // const std::string pass = std::string("dupa88ąśćę#$");
    const std::string pass = std::string("oaeóąę#$*");
    std::cout << "Init pass: " << pass << std::endl;

    //Print a wide char version of the pass
    std::wstring wpass = normal_string_to_wide_string(pass);
    std::wcout << "wpass: " << wpass    << std::endl;

    const auto result = exec((command + pass).c_str());

    if (result.starts_with("Password verified.\n"))
        std::cout << "PASS" << std::endl;
    else
        std::cout << result << std::endl;

    auto var = variants(wpass);
    for (auto v : var)
    {
        std::wcout << v << std::endl;
    }

    return 0;
}
