#include <iostream>
#include <ncurses.h>
#include <vector>
#include <curl/curl.h>
#include <json/json.h>
#include <unistd.h>

using namespace std;

// Function to fetch price data from CryptoCompare
string fetch_price(const string& symbol) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        string url = "https://min-api.cryptocompare.com/data/price?fsym=" + symbol + "&tsyms=USD";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](char* ptr, size_t size, size_t nmemb, string* data) -> size_t {
            data->append(ptr, size * nmemb);
            return size * nmemb;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    Json::Value jsonData;
    Json::CharReaderBuilder reader;
    string errs;
    if (Json::parseFromStream(reader, readBuffer, &jsonData, &errs)) {
        return jsonData["USD"].asString();
    }
    return "0";
}

// Function to draw a basic price chart
void draw_chart(const vector<int>& prices) {
    clear();
    int max_height = LINES - 5;
    int max_width = COLS - 5;

    for (int i = 0; i < max_width; ++i) {
        mvprintw(max_height, i, "-");
    }

    for (size_t i = 0; i < prices.size() && i < (size_t)max_width; ++i) {
        int y_pos = max_height - (prices[i] * max_height / 100);
        mvprintw(y_pos, i, "#");
    }

    mvprintw(LINES - 2, 0, "[1] Settings  [2] WiFi  [3] Chart  [Q] Quit");
    refresh();
}

int main() {
    initscr();
    noecho();
    curs_set(0);

    vector<int> prices;

    while (true) {
        string price_str = fetch_price("BTC");
        int price = stoi(price_str) / 100;
        prices.push_back(price);
        if (prices.size() > COLS - 5) prices.erase(prices.begin());

        draw_chart(prices);
        char ch = getch();

        if (ch == 'q' || ch == 'Q') {
            break;
        } else if (ch == '1') {
            mvprintw(LINES - 4, 0, "[Settings Menu] - Coming soon...");
        } else if (ch == '2') {
            mvprintw(LINES - 4, 0, "[WiFi Menu] - Coming soon...");
        } else if (ch == '3') {
            mvprintw(LINES - 4, 0, "[Chart Menu] - Coming soon...");
        }
        refresh();
        sleep(15);
    }

    endwin();
    return 0;
}
