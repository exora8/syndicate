#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <curl/curl.h>
#include <chrono>

// Helper to write API response to a string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Convert timestamp to readable date
std::string timestamp_to_date(long timestamp) {
    time_t rawTime = timestamp;
    struct tm* timeInfo = localtime(&rawTime);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", timeInfo);
    return std::string(buffer);
}

// Fetch historical data from API
std::string fetch_price_data(const std::string& symbol, const std::string& timeframe) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    std::string url = "https://min-api.cryptocompare.com/data/v2/histoday?fsym=" + symbol + "&tsym=USD&limit=2000";

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return readBuffer;
}

// Parse price data from JSON response
std::vector<std::pair<long, double>> parse_price_data(const std::string& data) {
    std::vector<std::pair<long, double>> prices;
    std::istringstream stream(data);
    std::string line;

    while (std::getline(stream, line)) {
        size_t time_pos = line.find("\"time\":");
        size_t close_pos = line.find("\"close\":");

        if (time_pos != std::string::npos && close_pos != std::string::npos) {
            try {
                long timestamp = std::stol(line.substr(time_pos + 7, line.find(",", time_pos) - time_pos - 7));
                double price = std::stod(line.substr(close_pos + 8, line.find(",", close_pos) - close_pos - 8));
                prices.emplace_back(timestamp, price);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing line: " << line << " (" << e.what() << ")\n";
            }
        }
    }
    return prices;
}

// Save data to CSV file
void save_historical_to_csv(const std::string& filename, const std::string& symbol, const std::vector<std::pair<long, double>>& prices) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "Date,Symbol,Close Price\n";
        for (const auto& [timestamp, price] : prices) {
            file << timestamp_to_date(timestamp) << "," << symbol << "," << price << "\n";
        }
        file.close();
        std::cout << "✅ Data saved to " << filename << std::endl;
    } else {
        std::cerr << "❌ Failed to open file: " << filename << std::endl;
    }
}

int main() {
    std::string symbol;
    std::cout << "Enter cryptocurrency symbol (e.g., BTC, ETH): ";
    std::cin >> symbol;

    auto start_time = std::chrono::steady_clock::now();

    std::string data = fetch_price_data(symbol, "daily");
    std::vector<std::pair<long, double>> prices = parse_price_data(data);
    save_historical_to_csv(symbol + "_prices.csv", symbol, prices);

    auto end_time = std::chrono::steady_clock::now();
    double elapsed_time = std::chrono::duration<double>(end_time - start_time).count();
    std::cout << "Fetch completed in " << elapsed_time << " seconds.\n";

    return 0;
}
