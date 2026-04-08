#pragma once

#include <string>
#include <ctime>

time_t g_start;
bool first = true;

inline void printProgressBar(int progress, int total, int barWidth = 50)
{
    static int lastPercent = -1;
    int percent = (int)((float)progress / total * 100);
    if (percent == lastPercent)
        return;
    lastPercent = percent;

    int filled = percent * barWidth / 100;
    std::string bar(filled, '#');
    bar = "\033[32m" + bar + "\033[31m";
    bar += std::string(barWidth - filled, '-');
    bar += "\033[0m";

    std::string timeStr = "Estimated time remaining: ";
    if (first) {
        timeStr += "-";
        g_start = std::time(nullptr);
        first = false;
    } else {
        time_t now = std::time(nullptr);
        double elapsed = difftime(now, g_start);
        double estimated = (percent > 0)
            ? (elapsed / percent) * (100 - percent)
            : 0;
        timeStr += std::to_string((int)estimated);
    }
    timeStr += "s";

    if (!first)
        printf("\033[2A");

    printf("\033[2K%s\n", timeStr.c_str());
    printf("\033[2K[%s] %d%%\n", bar.c_str(), percent);
    fflush(stdout);
}
