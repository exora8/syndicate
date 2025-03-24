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
std::string fetch_historical_data(const std::string& symbol) {
    std::string url = "https://min-api.cryptocompare.com/data/v2/histoday?fsym=" + symbol + "&tsym=USD&limit=2000";
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

// Save historical price data to CSV file
void save_historical_to_csv(const std::string& filename, const std::string& symbol, const std::string& data) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "Date,Symbol,Price(USD)\n";
        size_t pos = 0;
        while ((pos = data.find("{")) != std::string::npos) {
            size_t time_pos = data.find("\"time\":", pos);
            size_t close_pos = data.find("\"close\":", pos);
            
            if (time_pos != std::string::npos && close_pos != std::string::npos) {
                long timestamp = std::stol(data.substr(time_pos + 7, data.find(',', time_pos) - time_pos - 7));
                float price = std::stof(data.substr(close_pos + 8, data.find(',', close_pos) - close_pos - 8));
                
                std::time_t time = timestamp;
                file << std::ctime(&time) << "," << symbol << "," << price << "\n";
            }
            data.erase(0, pos + 1);
        }
        file.close();
    } else {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }
}

int main() {
    std::string symbol;
    std::cout << "Enter cryptocurrency symbol (e.g., BTC): ";
    std::cin >> symbol;

    std::string filename = symbol + "_historical_data.csv";
    std::cout << "Fetching full historical data for " << symbol << "..." << std::endl;

    std::string data = fetch_historical_data(symbol);
    if (!data.empty()) {
        save_historical_to_csv(filename, symbol, data);
        std::cout << "Data saved to " << filename << "\n";
    }

    return 0;
}
