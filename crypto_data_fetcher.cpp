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
std::string fetch_crypto_data(const std::string& symbol, const std::string& start_date, const std::string& end_date) {
    std::string url = "https://min-api.cryptocompare.com/data/v2/histoday?fsym=" + symbol + "&tsym=USD&limit=2000&toTs=" + end_date;
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
    std::ofstream file(filename, std::ios::app);
    if (file.is_open()) {
        size_t time_pos = data.find("time");
        size_t price_pos = data.find("close");

        if (time_pos != std::string::npos && price_pos != std::string::npos) {
            std::string clean_data = data;
            clean_data.erase(0, time_pos + 6);
            file << symbol << "," << clean_data << std::endl;
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

    std::string start_date, end_date;
    std::cout << "Enter start date (timestamp): ";
    std::cin >> start_date;
    std::cout << "Enter end date (timestamp): ";
    std::cin >> end_date;

    std::string filename = symbol + "_historical_data.csv";

    std::cout << "Fetching historical data for " << symbol << "..." << std::endl;

    std::string data = fetch_crypto_data(symbol, start_date, end_date);
    if (!data.empty()) {
        save_historical_to_csv(filename, symbol, data);
        std::cout << "Data saved to " << filename << std::endl;
    }

    return 0;
}
