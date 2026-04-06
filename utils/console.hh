#pragma once

#include <string>

inline void printProgressBar(int progress, int total, int barWidth = 50)
{
    static int lastPercent = -1;
    int percent = (int)((float)progress / total * 100);
    if (percent == lastPercent)
        return;
    lastPercent = percent;

    int filled = percent * barWidth / 100;

    std::string bar(filled, '#');
    bar = "\033[32m" + bar;
    bar += "\033[31m";
    bar += std::string(barWidth - filled, '-');
    bar += "\033[0m";

    printf("\r[%s] %d%%", bar.c_str(), percent);
    fflush(stdout);
}
