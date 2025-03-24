#include <iostream>
#include <fstream>
#include <vector>
#include <curl/curl.h>

// Callback function to handle API response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Function to fetch crypto data from CryptoCompare API
std::string fetch_crypto_data(const std::string& symbol, const std::string& currency = "USD") {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    std::string url = "https://min-api.cryptocompare.com/data/v2/histoday?fsym=" + symbol + "&tsym=" + currency + "&limit=2000";

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    if (res != CURLE_OK) {
        std::cerr << "Failed to fetch data: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    return readBuffer;
}

// Function to save data to a CSV file
void save_to_csv(const std::string& filename, const std::string& data) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << data;
        file.close();
        std::cout << "Data saved to " << filename << std::endl;
    } else {
        std::cerr << "Failed to open file." << std::endl;
    }
}

int main() {
    std::string symbol;
    std::cout << "Enter cryptocurrency symbol (e.g., BTC): ";
    std::cin >> symbol;

    std::string data = fetch_crypto_data(symbol);
    if (!data.empty()) {
        save_to_csv(symbol + "_data.csv", data);
    }

    return 0;
}
