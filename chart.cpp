#include <iostream>
#include <vector>
#include <string>
#include <ncurses.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string fetchCryptoPrice(const std::string& symbol) {
    std::string url = "https://min-api.cryptocompare.com/data/price?fsym=" + symbol + "&tsyms=USD";
    auto response = cpr::Get(cpr::Url{url});
    if (response.status_code == 200) {
        auto jsonData = json::parse(response.text);
        return "$" + std::to_string(jsonData["USD"]);
    } else {
        return "Error fetching price";
    }
}

void displayMenu(const std::vector<std::string>& cryptos) {
    int choice = 0;
    int ch;

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    while (true) {
        clear();
        mvprintw(0, 10, "Crypto CLI Dashboard");

        for (int i = 0; i < cryptos.size(); ++i) {
            if (i == choice) {
                attron(A_REVERSE);
            }
            mvprintw(i + 2, 5, cryptos[i].c_str());
            if (i == choice) {
                attroff(A_REVERSE);
            }
        }

        mvprintw(cryptos.size() + 4, 5, "Use arrow keys to navigate. Press Enter to select.");
        ch = getch();

        switch (ch) {
            case KEY_UP:
                choice = (choice == 0) ? cryptos.size() - 1 : choice - 1;
                break;
            case KEY_DOWN:
                choice = (choice == cryptos.size() - 1) ? 0 : choice + 1;
                break;
            case 10: // Enter key
                clear();
                mvprintw(0, 10, ("Fetching " + cryptos[choice] + " price...").c_str());
                std::string price = fetchCryptoPrice(cryptos[choice]);
                mvprintw(2, 10, (cryptos[choice] + " price: " + price).c_str());
                mvprintw(4, 10, "Press any key to go back.");
                getch();
                break;
            default:
                break;
        }

        if (ch == 'q') break;
    }

    endwin();
}

int main() {
    std::vector<std::string> cryptos = {"BTC", "ETH", "XRP", "DOGE", "ADA"};
    displayMenu(cryptos);
    return 0;
}
