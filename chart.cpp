#include <iostream>
#include <vector>
#include <ncurses.h>
#include <unistd.h>

using namespace std;

struct Candle {
    double open, high, low, close;
};

void drawCandle(int x, int y, Candle candle) {
    int bodyTop = max(candle.open, candle.close);
    int bodyBottom = min(candle.open, candle.close);
    attron(COLOR_PAIR(candle.close >= candle.open ? 1 : 2));

    // Draw wick
    for (int i = candle.high; i > bodyTop; --i) mvprintw(y - i, x, "|");
    for (int i = bodyBottom - 1; i > candle.low; --i) mvprintw(y - i, x, "|");
    
    // Draw body
    for (int i = bodyBottom; i <= bodyTop; ++i) mvprintw(y - i, x, "█");
    
    attroff(COLOR_PAIR(candle.close >= candle.open ? 1 : 2));
}

int main() {
    vector<Candle> candles = {
        {100, 110, 95, 105}, {105, 120, 102, 115}, {115, 118, 108, 112},
        {112, 125, 110, 120}, {120, 130, 115, 125}, {125, 128, 122, 123}
    };

    initscr();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    curs_set(0);

    int height, width;
    getmaxyx(stdscr, height, width);

    mvprintw(0, width / 2 - 10, "Crypto CLI Chart");
    mvprintw(height - 1, 0, "[Q] Quit");

    int xOffset = 5;
    for (size_t i = 0; i < candles.size(); ++i) {
        drawCandle(xOffset + i * 5, height / 2, candles[i]);
    }

    refresh();
    while (getch() != 'q');

    endwin();
    return 0;
}
