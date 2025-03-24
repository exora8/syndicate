#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <curl/curl.h>

// Callback for writing received data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

// Fetch crypto price data from API
std::string fetch_crypto_data(const std::string& symbol) {
    std::string url = "https://min-api.cryptocompare.com/data/price?fsym=" + symbol + "&tsyms=USD";
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            std::cerr << "Failed to fetch data: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
    }
    return response;
}

// Save price data to CSV file
void save_to_csv(const std::string& filename, const std::string& symbol, float price) {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        file << symbol << "," << price << "," << std::ctime(&time_t_now);
        file.close();
    } else {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }
}

int main() {
    std::string symbol;
    std::cout << "Enter cryptocurrency symbol (e.g., BTC): ";
    std::cin >> symbol;
    
    std::string filename = symbol + "_data.csv";
    int interval;
    std::cout << "Enter fetch interval (seconds): ";
    std::cin >> interval;

    std::cout << "Fetching data for " << symbol << " every " << interval << " seconds. Press Ctrl+C to stop." << std::endl;
    
    while (true) {
        std::string data = fetch_crypto_data(symbol);
        if (!data.empty()) {
            size_t pos = data.find_last_of(':');
            float price = std::stof(data.substr(pos + 1));
            std::cout << "Price: " << price << " USD" << std::endl;
            save_to_csv(filename, symbol, price);
        }

        std::this_thread::sleep_for(std::chrono::seconds(interval));
    }

    return 0;
}
