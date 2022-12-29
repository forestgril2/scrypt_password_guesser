#include <array>
#include <algorithm>
#include <wchar.h>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <locale>
#include <codecvt>
#include <set>
#include <string>
#include <vector>

#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

using StringSet = std::set<std::wstring, std::greater<std::wstring>>;

const int NUM_THREADS = std::thread::hardware_concurrency();

std::mutex var_mutex;
std::mutex cout_mutex;
std::condition_variable var_cv;
std::vector<std::thread> threads;

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

std::map<wchar_t, wchar_t> diacritics = {
    {L'a', L'ą'},
    {L'c', L'ć'},
    {L'e', L'ę'},
    {L'l', L'ł'},
    {L'n', L'ń'},
    {L'o', L'ó'},
    {L's', L'ś'},
    {L'u', L'€'},
    {L'x', L'ź'},
    {L'z', L'ż'},
    {L'A', L'Ą'},
    {L'C', L'Ć'},
    {L'E', L'Ę'},
    {L'L', L'Ł'},
    {L'N', L'Ń'},
    {L'O', L'Ó'},
    {L'S', L'Ś'},
    {L'U', L'€'},
    {L'X', L'Ź'},
    {L'Z', L'Ż'},
};

std::set<std::wint_t> shuffledSpecials = {};

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


StringSet generateVariantsWithOneMod(std::wstring&& base, std::function<wint_t(wint_t)> modify)
{
    StringSet variants({base});
    for(int i=0; i<base.length(); ++i)
    {
        std::wstring modifiedBase(base);
        modifiedBase[i] = modify(modifiedBase[i]);
        variants.insert(modifiedBase);
    }

    return variants;
}

// Get variants with a shuffled special character
StringSet generateVariantsWithOneShuffledSpecial(std::wstring&& base)
{
    auto alternativeSpecials = [](wint_t c) {
        if (shuffledSpecials.contains(c))
        {
            auto shuffledSpecialsCopy = shuffledSpecials;
            shuffledSpecialsCopy.erase(c);
            return shuffledSpecialsCopy;
        }
        return std::set<wint_t>({c});
    };

    StringSet variants({base});
    for(int i=0; i<base.length(); ++i)
    {
        std::wstring modifiedBase(base);
        for (auto c : alternativeSpecials(modifiedBase[i]))
        {
            modifiedBase[i] = c;
            variants.insert(modifiedBase);
        }
    }

    return variants;
}

// Get variants with one shift
StringSet generateVariantsWithOneShift(std::wstring&& base)
{
    auto shift = [](wint_t c) {
        //  std::wcout << "c: " << c << " iswupper: " << ::iswupper(c) << " iswlower: " << ::iswlower(c) << std::endl;
         if (::iswupper(c))
             return ::towlower(c);
         else if (::iswlower(c))
         return ::towupper(c);
         else if (::iswdigit(c) || special.contains(c))
             return (wint_t)special.at(c);
        return c;
    };

    return generateVariantsWithOneMod(std::move(base), shift);
}

//Generate variants with a misplaced alt key
StringSet generateVariantsWithOneAlt(std::wstring&& base)
{
    auto alt = [](wint_t c) {
        if (diacritics.contains(c))
            return (wint_t)diacritics.at(c);
        return c;
    };
    
    return generateVariantsWithOneMod(std::move(base), alt);
}

StringSet multiplyVariantsWithMod(StringSet&& variants, std::function<StringSet(std::wstring&&)> modify)
{
    StringSet addedMods;
    for(auto v : variants)
    {
        auto v2 = modify(std::move(v));
        addedMods.insert(v2.begin(), v2.end());
    }
    //Merge them
    variants.insert(addedMods.begin(), addedMods.end());
    return addedMods;
}

StringSet generateVariants(std::wstring&& base)
{
    auto variants = generateVariantsWithOneShuffledSpecial(std::move(base));

    // Get variants with one alt
    variants = multiplyVariantsWithMod(std::move(variants), generateVariantsWithOneAlt);
    // Get variants with second alt
    variants = multiplyVariantsWithMod(std::move(variants), generateVariantsWithOneAlt);
    // // Get variants with one more alt (3)
    // multiplyVariantsWithMod(std::move(variants), generateVariantsWithOneAlt);
    
    // Get variants with one shift
    variants = multiplyVariantsWithMod(std::move(variants), generateVariantsWithOneShift);
    // Get variants with second shift
    variants = multiplyVariantsWithMod(std::move(variants), generateVariantsWithOneShift);
    // // Get variants with one more shift (3)
    // multiplyVariantsWithMod(std::move(variants), generateVariantsWithOneShift);
    
    // append the original as well
    variants.insert(base);
    
    // Get variants with starting caps-lock - n signs
    // Get variants with one alt
    // Get variants with two alts
    // Get variants with double letter
    // Switch special sign with two alts
    return variants;
};

StringSet pass_variants(const std::wstring& base)
{
    return generateVariants(std::wstring(base));
};
std::wstring normal_to_wide(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string wide_to_normal(const std::wstring& wide_string) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  try {
    return converter.to_bytes(wide_string);
  } catch (const std::range_error&) {
    std::cerr << "Error converting wide string to normal string: invalid character encountered" << std::endl;
    return "";
  }
}

    
#ifdef _WIN32
// const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-10-04T08-04-22.071Z--54278f2fe320bec308cefc4640e50479e9ab1791 "; //oaeóąę#$*
const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-12-28T12-22-42.377Z--ab1c1f3869041caea5d2acea8076926c9206efcb "; //BardzoDługieHasło1234 
#else
    const std::string command = "python3 /home/space/projects/decrypt-ethereum-keyfile/main.py /home/space/projects/scrypt_password_guesser/UTC--2022-01-22T23-28-24.373Z--013efef911d47e09b85f9df956e6565d0f828622 ";
#endif

void worker_thread(int& variantsLeft, std::queue<std::wstring>& var, std::string& foundPass)
{
    while (foundPass.empty() && variantsLeft)
    {
        std::unique_lock lock(var_mutex);
        var_cv.wait(lock, [&var, &foundPass, &variantsLeft]{ return !foundPass.empty() || !variantsLeft || !var.empty(); });

        if (!foundPass.empty() || !variantsLeft)
        {
            return;
        }
        
        if (var.empty())
        {
            continue;
        }

        auto v = var.front();
        var.pop();
        lock.unlock();

        // Convert back to normal string
        const std::string vstr = wide_to_normal(v);

        const auto result = exec((command + vstr).c_str());
        if (result.starts_with("Password verified.\n"))
        {
            foundPass = vstr;
            {
                std::scoped_lock lock(cout_mutex);
                variantsLeft--;
                std::cout << "                                                                            \r" << std::flush;
                std::cout << "PASS   : " << vstr << " " << variantsLeft << " left\r" << std::flush;
            }
            var_cv.notify_all();
            return;
        }
        else
        {
            std::scoped_lock lock(cout_mutex);
            variantsLeft--;
            std::cout << "                                                                            \r" << std::flush;
            std::cout << "NO PASS: " << vstr << " " << variantsLeft << " left\r" << std::flush;
            if (!variantsLeft)
            {
                var_cv.notify_all();
                return;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    // Set the current locale to UTF-8
    std::setlocale(LC_ALL, "en_US.UTF-8");
    
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <password>" << std::endl;
        return 1;
    }
    // get specials to shuffle as second argument
    if (argc > 2)
    {
        auto specials = std::string(argv[2]);
        std::cout << "Specials: " << specials << std::endl;
        // insert special chars to shuffledSpecials
        shuffledSpecials.insert(specials.begin(), specials.end());
    }

    // invert digit-special mapping and attach it to the main map
    std::map<wchar_t, wchar_t> special_inv; // = special;
    for (auto& [k, v] : special)
        special_inv[v] = k;
    special.insert(special_inv.begin(), special_inv.end());
    
    // invert diacritics mapping and attach it to the main map
    std::map<wchar_t, wchar_t> diacritics_inv; // = diacritics;
    for (auto& [k, v] : diacritics)
        diacritics_inv[v] = k;
    diacritics.insert(diacritics_inv.begin(), diacritics_inv.end());
    
    // Setup password and variants
    // const std::string pass = std::string("dupa88ąśćę#$");
    // const std::string pass = std::string("oaeóąę#$*");
    // const std::string pass = std::string("oaeóąę#$*");
    const std::string pass = std::string(argv[1]);

    std::cout << "Initial pass: " << pass << std::endl;

    //Print a wide char version of the pass
    std::wstring wpass = normal_to_wide(pass);
    std::wcout << "Pass as wide string: " << wpass    << std::endl;
    // Prepare variants
    auto variants = pass_variants(wpass);
    // Print all variants
    std::cout << "Variants: " << std::endl;
    for (auto v : variants)
    {
        std::wcout << v << std::endl;
    }
    std::cout << "Number of variants: " << variants.size() << std::endl;
    int variantsLeft = variants.size();

    // Prepare the thread pool and the queue for the variants
    std::queue<std::wstring> variants_queue;
    std::string foundPass = ""; 
    for (int i = 0; i < NUM_THREADS; i++)
    {
        threads.emplace_back(worker_thread, std::ref(variantsLeft), std::ref(variants_queue), std::ref(foundPass));
    }

    std::cout << "Press enter to start" << std::endl;
    std::cin.get();    // Add items to the queue

    {
        std::lock_guard<std::mutex> lock(var_mutex);
        for (auto v : variants)
        {
            variants_queue.push(v);
        }
    }
    // Let know about the queue size and wait for user input ( enter press)
    std::cout << "Queue size: " << variants_queue.size() << std::endl;

    var_cv.notify_all();

    for (auto& t : threads)
    {
        t.join();
    }

    std::cout << std::endl;
    if (foundPass.empty())
    {
        std::cout << "No pass found" << std::endl;
        return 1;
    }
    std::cout << "Found pass: " << foundPass << std::endl;
    return 0;
}
