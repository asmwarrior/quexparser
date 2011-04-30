#ifndef TIMER_H
#define TIMER_H

#ifdef _WIN32

#include<Windows.h>


class CTimer
{
private:

    LARGE_INTEGER m_base;
    LARGE_INTEGER m_temp;
    float m_resolution;

public:

    CTimer()
    {
        LARGE_INTEGER t_freq;
        QueryPerformanceFrequency(&t_freq);
        m_resolution = (float) (1.0f / (double) t_freq.QuadPart);
        reset();
    }

    void reset()
    {
        QueryPerformanceCounter(&m_base);
    }

    inline float GetCurrentTime()
    {
        QueryPerformanceCounter(&m_temp);
        return (m_temp.QuadPart - m_base.QuadPart) * m_resolution * 1000.0f;
    }

    void SleepTime(float ms_val)
    {
        float ms_st = GetCurrentTime();
        while (GetCurrentTime()-ms_st < ms_val)
            continue;
    }

};
#else
class CTimer
{
private:
    unsigned long m_base;
public:
    CTimer()
    {
        reset();
    }

    void reset()
    {
        timeval t;
        gettimeofday(&t, NULL);
        m_base = t.tv_sec;
    }

    float GetCurrentTime()
    {
        timeval t;
        gettimeofday(&t, NULL);
        return 1000 * (t.tv_sec - m_base) + t.tv_usec * 0.001f;
    }
};
#endif

#endif