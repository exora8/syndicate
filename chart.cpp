#include <iostream>
#include <ncurses.h>
#include <vector>
#include <string>
#include <algorithm>

struct Candle {
    double open, high, low, close;
};

std::vector<Candle> fetchCandleData() {
    return {
        {42.1, 42.5, 42.0, 42.4},
        {42.4, 42.6, 42.3, 42.3},
        {42.3, 42.7, 42.2, 42.6},
        {42.6, 42.8, 42.5, 42.7},
        {42.7, 42.9, 42.4, 42.6},
        {42.6, 43.0, 42.3, 42.8},
        {42.8, 43.2, 42.7, 43.0}
    };
}

void drawMenu(int highlight) {
    const char* options[] = {"1. Tampilkan Chart", "2. Refresh Data", "3. Keluar"};
    int menu_size = sizeof(options) / sizeof(options[0]);
    for (int i = 0; i < menu_size; ++i) {
        if (i == highlight) {
            attron(A_REVERSE);
        }
        mvprintw(2 + i, 2, options[i]);
        attroff(A_REVERSE);
    }
    refresh();
}

void drawCandleChart(const std::vector<Candle>& candles) {
    clear();
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    int chart_height = max_y - 6;
    int candle_width = max_x / candles.size();

    double highest_price = 0, lowest_price = 999999;
    for (const auto& c : candles) {
        highest_price = std::max(highest_price, c.high);
        lowest_price = std::min(lowest_price, c.low);
    }

    double price_range = highest_price - lowest_price;

    for (size_t i = 0; i < candles.size(); ++i) {
        const Candle& c = candles[i];
        int x_pos = i * candle_width + candle_width / 2;

        int high_pos = chart_height - static_cast<int>(((c.high - lowest_price) / price_range) * chart_height);
        int low_pos = chart_height - static_cast<int>(((c.low - lowest_price) / price_range) * chart_height);
        int open_pos = chart_height - static_cast<int>(((c.open - lowest_price) / price_range) * chart_height);
        int close_pos = chart_height - static_cast<int>(((c.close - lowest_price) / price_range) * chart_height);

        for (int y = high_pos; y <= low_pos; ++y) {
            mvprintw(y, x_pos, "│");
        }

        if (c.close >= c.open) {
            attron(COLOR_PAIR(1));
        } else {
            attron(COLOR_PAIR(2));
        }

        int body_top = std::min(open_pos, close_pos);
        int body_bottom = std::max(open_pos, close_pos);
        for (int y = body_top; y <= body_bottom; ++y) {
            mvprintw(y, x_pos - 1, "██");
        }

        attroff(COLOR_PAIR(1));
        attroff(COLOR_PAIR(2));
    }
    mvprintw(max_y - 2, 2, "Tekan [B] untuk kembali ke menu");
    refresh();
}

void mainMenu() {
    initscr();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    int highlight = 0;
    int choice;
    std::vector<Candle> candles = fetchCandleData();

    while (true) {
        clear();
        mvprintw(0, 2, "=== Crypto Market CLI ===");
        drawMenu(highlight);
        choice = getch();

        switch (choice) {
            case KEY_UP:
                highlight = (highlight == 0) ? 2 : highlight - 1;
                break;
            case KEY_DOWN:
                highlight = (highlight == 2) ? 0 : highlight + 1;
                break;
            case 10:
                if (highlight == 0) {
                    drawCandleChart(candles);
                    while (getch() != 'b');
                } else if (highlight == 1) {
                    candles = fetchCandleData();
                } else if (highlight == 2) {
                    endwin();
                    return;
                }
                break;
        }
    }
}

int main() {
    mainMenu();
    return 0;
}
