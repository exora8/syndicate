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
    oss << std::put_time(tm_ptr, "%Y-%m-%d");
    return oss.str();
}

// Fetch historical crypto price data from API
std::string fetch_crypto_data(const std::string& symbol, int limit) {
    std::string url = "https://min-api.cryptocompare.com/data/v2/histoday?fsym=" + symbol + "&tsym=USD&limit=" + std::to_string(limit);
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

// Calculate Simple Moving Average (SMA)
double calculate_sma(const std::vector<double>& prices, int period, int index) {
    if (index < period - 1) return 0;
    double sum = 0;
    for (int i = index - period + 1; i <= index; ++i) {
        sum += prices[i];
    }
    return sum / period;
}

// Calculate Exponential Moving Average (EMA)
double calculate_ema(const std::vector<double>& prices, int period, int index, double prev_ema) {
    double multiplier = 2.0 / (period + 1);
    return (prices[index] - prev_ema) * multiplier + prev_ema;
}

// Calculate RSI (Relative Strength Index)
double calculate_rsi(const std::vector<double>& prices, int period, int index) {
    if (index < period) return 0;
    double gain = 0, loss = 0;
    for (int i = index - period + 1; i <= index; ++i) {
        double change = prices[i] - prices[i - 1];
        if (change > 0) gain += change;
        else loss += std::abs(change);
    }
    double rs = (loss == 0) ? 100 : gain / loss;
    return 100 - (100 / (1 + rs));
}

// Save historical price data with indicators to CSV file
void save_historical_to_csv(const std::string& filename, const std::string& symbol, const std::string& data) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "Date,Symbol,Close Price,SMA,EMA,RSI" << std::endl;

        std::vector<double> prices;

        size_t data_start = data.find("[{"), data_end = data.find("}]");
        if (data_start == std::string::npos || data_end == std::string::npos) {
            std::cerr << "Error parsing data format!" << std::endl;
            return;
        }

        std::string json_data = data.substr(data_start + 1, data_end - data_start);
        std::istringstream data_stream(json_data);
        std::string line;

        while (std::getline(data_stream, line, '{')) {
            size_t time_pos = line.find("\"time\":");
            size_t price_pos = line.find("\"close\":");

            if (time_pos != std::string::npos && price_pos != std::string::npos) {
                try {
                    long timestamp = std::stol(line.substr(time_pos + 7, line.find(',', time_pos) - time_pos - 7));
                    double price = std::stod(line.substr(price_pos + 8, line.find(',', price_pos) - price_pos - 8));

                    prices.push_back(price);
                    std::string date = timestamp_to_date(timestamp);

                    int index = prices.size() - 1;
                    double sma = calculate_sma(prices, 14, index);
                    double ema = (index == 0) ? price : calculate_ema(prices, 14, index, prices[index - 1]);
                    double rsi = calculate_rsi(prices, 14, index);

                    file << date << "," << symbol << "," << price << "," << sma << "," << ema << "," << rsi << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Parsing error: " << e.what() << "\nLine: " << line << std::endl;
                }
            }
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
    int limit = 200;

    std::string filename = symbol + "_historical_data.csv";

    std::cout << "Fetching historical data for " << symbol << "..." << std::endl;

    std::string data = fetch_crypto_data(symbol, limit);
    if (!data.empty()) {
        save_historical_to_csv(filename, symbol, data);
    }

    return 0;
}
