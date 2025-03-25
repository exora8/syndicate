#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <curl/curl.h>
#include <ctime>
#include <iomanip>
#include <cmath>

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
    oss << std::put_time(tm_ptr, "%Y-%m-%d %H:%M");
    return oss.str();
}

// Fetch historical crypto price data from API
std::string fetch_crypto_data(const std::string& symbol, long to_timestamp) {
    std::string url = "https://min-api.cryptocompare.com/data/v2/histominute?fsym=" + symbol + "&tsym=USD&limit=2000&toTs=" + std::to_string(to_timestamp);
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

// Display progress bar
void show_progress(int current, int total, double elapsed_time) {
    int width = 50; // Progress bar width
    float progress = (float)current / total;
    int pos = width * progress;

    std::cout << "[";
    for (int i = 0; i < width; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    double remaining_time = (elapsed_time / current) * (total - current);
    std::cout << "] " << int(progress * 100.0) << "% | ETA: " << std::fixed << std::setprecision(1) << remaining_time << "s\r";
    std::cout.flush();
}

int main() {
    std::string symbol;
    std::cout << "Enter cryptocurrency symbol (e.g., BTC): ";
    std::cin >> symbol;
    std::string filename = symbol + "_shortterm_data.csv";
    std::vector<std::pair<long, double>> all_prices;
    long to_timestamp = std::time(nullptr);
    int batch = 0;
    int max_batches = 20;
    auto start_time = std::chrono::steady_clock::now();

    std::cout << "Fetching short-term data for " << symbol << "..." << std::endl;

    while (batch < max_batches) {
        std::string data = fetch_crypto_data(symbol, to_timestamp);
        if (data.empty()) break;

        auto prices = parse_price_data(data);
        if (prices.empty()) break;

        all_prices.insert(all_prices.end(), prices.begin(), prices.end());
        to_timestamp = prices.back().first - 1;
        batch++;

        auto current_time = std::chrono::steady_clock::now();
        double elapsed_time = std::chrono::duration<double>(current_time - start_time).count();
        show_progress(batch, max_batches, elapsed_time);
    }

    if (!all_prices.empty()) {
        save_historical_to_csv(filename, symbol, all_prices);
    } else {
        std::cerr << "No data fetched for " << symbol << std::endl;
    }

    std::cout << "\nFetch complete!" << std::endl;
    return 0;
}
