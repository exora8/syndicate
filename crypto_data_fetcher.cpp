#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <curl/curl.h>
#include <ctime>
#include <iomanip>

// Callback for writing received data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

// Convert timestamp to readable date
std::string timestamp_to_date(long timestamp) {
    std::time_t t = timestamp;
    std::tm* tm_ptr = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm_ptr, "%Y-%m-%d");
    return oss.str();
}

// Fetch historical crypto price data from API
std::string fetch_crypto_data(const std::string& symbol, long to_timestamp) {
    std::string url = "https://min-api.cryptocompare.com/data/v2/histoday?fsym=" + symbol + "&tsym=USD&limit=20&toTs=" + std::to_string(to_timestamp);
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

// Parse price data from response
std::vector<std::pair<long, double>> parse_price_data(const std::string& data) {
    std::vector<std::pair<long, double>> prices;
    size_t time_pos = 0, price_pos = 0;
    std::string line;
    std::istringstream data_stream(data);
    
    while (std::getline(data_stream, line)) {
        size_t time_pos = line.find("\"time\":");
        size_t price_pos = line.find("\"close\":");

        if (time_pos != std::string::npos && price_pos != std::string::npos) {
            long timestamp = std::stol(line.substr(time_pos + 7, line.find(",", time_pos) - time_pos - 7));
            double price = std::stod(line.substr(price_pos + 8, line.find(",", price_pos) - price_pos - 8));
            prices.emplace_back(timestamp, price);
        }
    }
    return prices;
}

// Save historical price data to CSV file
void save_historical_to_csv(const std::string& filename, const std::string& symbol, const std::vector<std::pair<long, double>>& prices) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "Date,Symbol,Close Price" << std::endl;
        for (const auto& [timestamp, price] : prices) {
            file << timestamp_to_date(timestamp) << "," << symbol << "," << price << std::endl;
        }
        file.close();
        std::cout << "Data saved to " << filename << std::endl;
    } else {
        std::cerr << "Failed to open file: " << filename << std::endl;
    }
}

int main() {
    std::string symbol;
    std::cout << "Enter cryptocurrency symbol (e.g., BTC): ";
    std::cin >> symbol;
    std::string filename = symbol + "_3_weeks_daily_data.csv";

    std::cout << "Fetching 3 weeks daily data for " << symbol << "..." << std::endl;

    long to_timestamp = std::time(nullptr);
    std::string data = fetch_crypto_data(symbol, to_timestamp);

    if (!data.empty()) {
        auto prices = parse_price_data(data);
        if (!prices.empty()) {
            save_historical_to_csv(filename, symbol, prices);
        } else {
            std::cerr << "No price data found for " << symbol << std::endl;
        }
    }
    return 0;
}
