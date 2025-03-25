#include <iostream>
#include <string>
#include <ncurses.h>
#include <curl/curl.h>
#include <json/json.h>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <algorithm>

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

// Fungsi untuk menampilkan grafik
void display_chart(const std::string &crypto) {
    initscr();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);

    std::vector<double> prices;
    const int max_width = 50;

    while (true) {
        clear();
        std::string price_str = fetch_price(crypto);
        double price = std::stod(price_str);
        prices.push_back(price);
        if (prices.size() > max_width) prices.erase(prices.begin());

        mvprintw(0, 0, "Crypto Market CLI Tool");
        attron(COLOR_PAIR(3));
        mvprintw(2, 0, ("Crypto: " + crypto).c_str());
        mvprintw(3, 0, ("Price (USD): " + price_str).c_str());
        attroff(COLOR_PAIR(3));

        // Menampilkan grafik bar
        double max_price = *max_element(prices.begin(), prices.end());
        double min_price = *min_element(prices.begin(), prices.end());

        for (size_t i = 0; i < prices.size(); ++i) {
            int bar_height = ((prices[i] - min_price) / (max_price - min_price)) * 10;
            for (int y = 10; y > 10 - bar_height; --y) {
                if (i == prices.size() - 1) {
                    attron(prices[i] >= prices[i - 1] ? COLOR_PAIR(1) : COLOR_PAIR(2));
                }
                mvprintw(y, i * 2, "█");
                if (i == prices.size() - 1) attroff(COLOR_PAIR(1) | COLOR_PAIR(2));
            }
        }

        mvprintw(12, 0, "Press 'q' to quit.");

        refresh();
        if (getch() == 'q') break;

        sleep(5);
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
