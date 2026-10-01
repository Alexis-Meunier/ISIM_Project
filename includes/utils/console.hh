#pragma once

#include <string>
#include <ctime>

time_t g_start;
bool first = true;

inline std::string getTime(const double& elapsed)
{
    std::string timeStr = "";

    auto timeTaken = static_cast<int>(elapsed);

    int hour = timeTaken / 3600;
    if (hour > 0)
    {
        timeStr += std::to_string(hour) + "h";
        timeTaken = timeTaken % 3600;
    }

    int minute = timeTaken / 60;
    if (minute > 0)
    {
        timeStr += std::to_string(minute) + "m";
        timeTaken = timeTaken % 60;
    }

    timeStr += std::to_string(timeTaken) + "s";

    return timeStr;
}

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
    std::string timeSinceBegin = "Running since ";
    if (first) {
        timeStr += "- s";
        g_start = std::time(nullptr);
    } else {
        time_t now = std::time(nullptr);
        double elapsed = difftime(now, g_start);
        double estimated = (percent > 0)
            ? (elapsed / percent) * (100 - percent)
            : 0;
        timeStr += getTime(estimated);
        timeSinceBegin += getTime(difftime(now, g_start));
    }

    if (!first)
        printf("\033[3A");
    else
        first = false;


    printf("\033[2K%s\n", timeStr.c_str());
    printf("\033[2K[%s] %d%%\n", bar.c_str(), percent);
    printf("\033[2K%s\n", timeSinceBegin.c_str());
    fflush(stdout);
}

inline void printTimeTaken(const time_t& start, const time_t& end, const std::string& op)
{
    auto timeStr = getTime(difftime(end, start));
    printf("Took %s %s\n", timeStr.c_str(), op.c_str());
}
