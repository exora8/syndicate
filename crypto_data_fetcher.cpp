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

// Fetch historical crypto price data from API
std::string fetch_historical_data(const std::string& symbol, const std::string& timeframe) {
    std::string url = "https://min-api.cryptocompare.com/data/v2/histo" + timeframe + "?fsym=" + symbol + "&tsym=USD&limit=2000";
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
void save_to_csv(const std::string& filename, const std::string& symbol, const std::string& data) {
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        file << "time,price" << std::endl;
        size_t pos = 0;
        std::string token;

        while ((pos = data.find("},{")) != std::string::npos) {
            token = data.substr(0, pos);
            size_t time_pos = token.find("time":);
            size_t price_pos = token.find("close":);

            if (time_pos != std::string::npos && price_pos != std::string::npos) {
                std::string timestamp = token.substr(time_pos + 6, 10);
                std::string price = token.substr(price_pos + 7, token.find(',', price_pos) - price_pos - 7);

                file << timestamp << "," << price << std::endl;
            }
            data.erase(0, pos + 2);
        }
        file.close();
    } else {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }
}

int main() {
    std::string symbol, timeframe;
    std::cout << "Enter cryptocurrency symbol (e.g., BTC): ";
    std::cin >> symbol;

    std::cout << "Enter timeframe (day/hour/minute): ";
    std::cin >> timeframe;

    std::string filename = symbol + "_historical_data.csv";

    std::cout << "Fetching historical data for " << symbol << " with " << timeframe << " timeframe..." << std::endl;
    
    std::string data = fetch_historical_data(symbol, timeframe);
    if (!data.empty()) {
        save_to_csv(filename, symbol, data);
        std::cout << "Data saved to " << filename << std::endl;
    }

    return 0;
}
