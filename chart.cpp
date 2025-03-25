#include <iostream>
#include <ncurses.h>
#include <vector>
#include <string>

struct Candle {
    double open, high, low, close;
};

void drawCandle(int x, int y, Candle candle) {
    int bodyTop = std::max(candle.open, candle.close);
    int bodyBottom = std::min(candle.open, candle.close);
    int color = (candle.close >= candle.open) ? COLOR_GREEN : COLOR_RED;

    // Wick
    attron(COLOR_PAIR(3));
    for (int i = candle.high; i > bodyTop; --i) mvprintw(y - i, x, "|");
    for (int i = bodyBottom - 1; i > candle.low; --i) mvprintw(y - i, x, "|");
    attroff(COLOR_PAIR(3));

    // Body
    attron(COLOR_PAIR(color));
    for (int i = bodyBottom; i <= bodyTop; ++i) mvprintw(y - i, x, "█");
    attroff(COLOR_PAIR(color));
}

int main() {
    initscr();
    start_color();
    curs_set(0);

    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);

    std::vector<Candle> candles = {{1.2, 1.5, 1.1, 1.4}, {1.4, 1.6, 1.3, 1.35}, {1.35, 1.8, 1.2, 1.7}};
    int x = 5;

    for (const auto &candle : candles) {
        drawCandle(x, LINES - 1, candle);
        x += 4;
    }

    mvprintw(LINES - 1, 0, "Tekan [B] untuk kembali ke menu");
    refresh();
    getch();

    endwin();
    return 0;
}
