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

#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

const int NUM_THREADS = 12;

std::mutex var_mutex;
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

std::set<std::wstring> pass_variants(const std::wstring& base)
{
    return variants(std::wstring(base));
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
const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-10-04T08-04-22.071Z--54278f2fe320bec308cefc4640e50479e9ab1791 "; //oaeóąę#$*
#else
    const std::string command = "python3 /home/space/projects/decrypt-ethereum-keyfile/main.py /home/space/projects/scrypt_password_guesser/UTC--2022-01-22T23-28-24.373Z--013efef911d47e09b85f9df956e6565d0f828622 ";
#endif

void worker_thread(std::queue<std::wstring>& var, bool& done)
{
    while (!done)
    {
        std::unique_lock lock(var_mutex);
        var_cv.wait(lock, [&var, done]{ return !var.empty() || done; });
        
        if (done)
        {
            return;
        }
        
        if (var.empty())
        {
            break;
        }

        auto v = var.front();
        var.pop();
        lock.unlock();

        // Convert back to normal string
        const std::string vstr = wide_to_normal(v);

        std::cout << "Trying: " << vstr << std::endl;
        const auto result = exec((command + vstr).c_str());
        if (result.starts_with("Password verified.\n"))
        {
            std::cout << "PASS   : " << vstr << std::endl;
            done = true;
            var_cv.notify_all();
            return;
        }
        else
        {
            std::cout << "NO PASS: " << vstr << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    // Set the current locale to UTF-8
    std::setlocale(LC_ALL, "en_US.UTF-8");

    // invert digit-special mapping and attach it to the main map
    std::map<wchar_t, wchar_t> special_inv; // = special;
    for (auto& [k, v] : special)
        special_inv[v] = k;
    special.insert(special_inv.begin(), special_inv.end()); 


    // Setup password and variants
    // const std::string pass = std::string("dupa88ąśćę#$");
    // const std::string pass = std::string("oaeóąę#$*");
    const std::string pass = std::string("oaEóąę#$8");
    std::cout << "Initial pass: " << pass << std::endl;

    //Print a wide char version of the pass
    std::wstring wpass = normal_to_wide(pass);
    std::wcout << "Pass as wide string: " << wpass    << std::endl;
    // Prepare variants
    auto variants = pass_variants(wpass);
    std::cout << "Number of variants: " << variants.size() << std::endl;

    // Prepare the thread pool and the queue for the variants
    std::queue<std::wstring> variants_queue;
    bool done = false; 
    for (int i = 0; i < NUM_THREADS; i++)
    {
        threads.emplace_back(worker_thread, std::ref(variants_queue), std::ref(done));
    }

    std::cout << "Press enter to start" << std::endl;
    std::cin.get();    // Add items to the queue

    {
        std::lock_guard<std::mutex> lock(var_mutex);
        for (auto v : variants)
        {
            variants_queue.emplace(v);
        }
    }
    // Let know about the queue size and wait for user input ( enter press)
    std::cout << "Queue size: " << variants_queue.size() << std::endl;


    var_cv.notify_all();

    for (auto& t : threads)
    {
        t.join();
    }

    return 0;
}
