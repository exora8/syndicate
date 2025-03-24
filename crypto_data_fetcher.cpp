#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <curl/curl.h>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <thread>

// Callback buat nerima data dari API
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

// Ubah timestamp ke format tanggal biasa
std::string timestamp_to_date(long timestamp) {
    std::time_t t = timestamp;
    std::tm* tm_ptr = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm_ptr, "%Y-%m-%d");
    return oss.str();
}

// Fetch data harga historis dari API
std::string fetch_crypto_data(const std::string& symbol, long to_timestamp) {
    std::string url = "https://min-api.cryptocompare.com/data/v2/histoday?fsym=" + symbol + "&tsym=USD&limit=2000&toTs=" + std::to_string(to_timestamp);
    CURL* curl = curl_easy_init();
    std::string response;

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");  // Pakai gzip buat compress biar lebih cepat
        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "Gagal fetch data: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
    }
    return response;
}

// Parsing harga dari response API lebih cepat
std::vector<std::pair<long, double>> parse_price_data(const std::string& data) {
    std::vector<std::pair<long, double>> prices;
    size_t time_pos = 0, price_pos = 0;
    std::string line;
    size_t start = 0;

    while ((time_pos = data.find("\"time\":", start)) != std::string::npos &&
           (price_pos = data.find("\"close\":", start)) != std::string::npos) {

        long timestamp = std::stol(data.substr(time_pos + 7, data.find(",", time_pos) - time_pos - 7));
        double price = std::stod(data.substr(price_pos + 8, data.find(",", price_pos) - price_pos - 8));

        prices.emplace_back(timestamp, price);
        start = price_pos + 1; // Lanjut parsing baris berikutnya
    }

    return prices;
}

// Save data harga ke file CSV
void save_historical_to_csv(const std::string& filename, const std::string& symbol, const std::vector<std::pair<long, double>>& prices) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "Date,Symbol,Close Price" << std::endl;
        for (const auto& [timestamp, price] : prices) {
            file << timestamp_to_date(timestamp) << "," << symbol << "," << price << std::endl;
        }
        file.close();
        std::cout << "Data berhasil disimpan ke " << filename << std::endl;
    } else {
        std::cerr << "Gagal buka file: " << filename << std::endl;
    }
}

int main() {
    std::string symbol;
    std::cout << "Masukkan simbol cryptocurrency (contoh: BTC): ";
    std::cin >> symbol;
    std::string filename = symbol + "_full_historical_data.csv";
    std::vector<std::pair<long, double>> all_prices;
    long to_timestamp = std::time(nullptr);

    std::cout << "Fetching full historical data untuk " << symbol << "..." << std::endl;

    while (true) {
        std::string data = fetch_crypto_data(symbol, to_timestamp);
        if (data.empty()) break;

        auto prices = parse_price_data(data);
        if (prices.empty()) break;

        all_prices.insert(all_prices.end(), prices.begin(), prices.end());
        to_timestamp = prices.back().first - 1; // Mundur 1 detik biar nggak double data

        // Delay biar nggak kena limit API
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (!all_prices.empty()) {
        save_historical_to_csv(filename, symbol, all_prices);
    } else {
        std::cerr << "Gagal fetch data untuk " << symbol << std::endl;
    }

    return 0;
}

