#include <iostream>
#include <string>
#include <ncurses.h>
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>
#include <unistd.h>

// Fungsi untuk menangani response dari CURL
size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output) {
    size_t totalSize = size * nmemb;
    output->append((char *)contents, totalSize);
    return totalSize;
}

// Fungsi fetch harga dari CryptoCompare
std::string fetch_price(const std::string &crypto) {
    std::string url = "https://min-api.cryptocompare.com/data/price?fsym=" + crypto + "&tsyms=USD";
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return "Error fetching data";
        }

        Json::Value jsonData;
        Json::CharReaderBuilder reader;
        std::string errs;
        std::istringstream jsonStream(readBuffer);
        if (Json::parseFromStream(reader, jsonStream, &jsonData, &errs)) {
            return jsonData["USD"].asString();
        }
    }
    return "Error parsing data";
}

// Fungsi untuk menampilkan UI CLI
void display_chart(const std::string &crypto) {
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);

    while (true) {
        clear();
        std::string price = fetch_price(crypto);
        mvprintw(0, 0, "Crypto Market CLI Tool");
        mvprintw(2, 0, ("Crypto: " + crypto).c_str());
        mvprintw(3, 0, ("Price (USD): " + price).c_str());
        mvprintw(5, 0, "Press 'q' to quit.");

        refresh();
        if (getch() == 'q') break;

        sleep(15);
    }
    endwin();
}

int main() {
    std::string crypto;
    std::cout << "Enter cryptocurrency symbol (e.g., BTC): ";
    std::cin >> crypto;
    display_chart(crypto);

    return 0;
}
