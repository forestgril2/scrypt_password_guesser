#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

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

int main()
{
#ifdef _WIN32
    const std::string command = "python D:/Projects/decrypt-ethereum-keyfile/main.py D:/Projects/scrypt_password_guesser/UTC--2022-01-22T23-28-24.373Z--013efef911d47e09b85f9df956e6565d0f828622 ";
#else
    const std::string command = "python3 /home/space/projects/decrypt-ethereum-keyfile/main.py /home/space/projects/scrypt_password_guesser/UTC--2022-01-22T23-28-24.373Z--013efef911d47e09b85f9df956e6565d0f828622 ";
#endif

    const std::string pass = "dupa88ąść";

    const auto result = exec((command + pass).c_str());

    if (result.starts_with("Password verified.\n"))
        std::cout << "PASS" << std::endl;
    else
        std::cout << result << std::endl;

    return 0;
}
