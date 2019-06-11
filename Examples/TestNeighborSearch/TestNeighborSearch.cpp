/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   =========================================================================*/


#include "imstkMath.h"

#include "imstkSpatialHashTableSeparateChaining.h"
#include "imstkGridBasedNeighborSearch.h"

#include <fstream>
#include <string>
#include <iostream>
using namespace imstk;

#define SPHERE_CENTER   Vec3d(0, 0, 0)
#define PARTICLE_RADIUS 0.05
#define ITERATIONS      100
//#define PRINT_NEIGHBORS true
#define PRINT_NEIGHBORS false

//#define  DISABLE_GRID_BASED_SEARCH
//#define  DISABLE_SPATIAL_HASHING

#define STEP 1.02

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
template<class DataVec>
void writeFile(const DataVec& data, const std::string& fileName)
{
    std::ofstream file(fileName, std::ios::out);
    if(!file.is_open())
    {
        return;
    }

    for(auto& x : data)
    {
        file << "v " << x[0] << " " << x[1] << " " << x[2] << "\n";
    }

    file.close();
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
#include <chrono>
#include <string>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <functional>

class Timer {
protected:
    using Clock = std::chrono::high_resolution_clock;

public:
    Timer()          = default;
    virtual ~Timer() = default;

    void tick()
    {
        assert(!m_TimerTicked);
        m_StartTime   = Clock::now();
        m_TimerTicked = true;
    }

    double tock()
    {
        assert(m_TimerTicked);
        m_EndTime     = Clock::now();
        m_TimerTicked = false;
        m_ElapsedTime = std::chrono::duration<double, std::milli>(m_EndTime - m_StartTime).count();

        return m_ElapsedTime;
    }

    std::string getRunTime()
    {
        if(m_TimerTicked)
        {
            tock();
        }
        m_Str.clear();
        m_Str += std::to_string(m_ElapsedTime);
        m_Str += std::string("ms");
        return m_Str;
    }

    std::string getRunTime(const std::string& caption)
    {
        if(m_TimerTicked)
        {
            tock();
        }

        m_Str.clear();
        m_Str += caption;
        m_Str += std::string(": ");
        m_Str += std::to_string(m_ElapsedTime);
        m_Str += std::string("ms");

        return m_Str;
    }

    template<class Function>
    static std::string getRunTime(const std::string& caption, const Function& function)
    {
        Timer timer;
        ////////////////////////////////////////////////////////////////////////////////
        timer.tick();
        function();
        timer.tock();
        ////////////////////////////////////////////////////////////////////////////////
        return timer.getRunTime(caption);
    }

private:
    Clock::time_point m_StartTime;
    Clock::time_point m_EndTime;
    std::string m_Str;

    double m_ElapsedTime { 0.0 };
    bool m_TimerTicked { false };
};

class ScopeTimer : public Timer {
public:
    ScopeTimer(const std::function<void(const std::string&)>& exitFunc) : m_ExitFunc(exitFunc), m_bDefaultCaption(false) { this->tick(); }
    ScopeTimer(const std::string& caption) : m_Caption(caption), m_bDefaultCaption(true) { this->tick(); }
    ~ScopeTimer()
    {
        if(m_bDefaultCaption)
        {
            printf("%s\n", this->getRunTime(m_Caption).c_str());
        }
        else
        {
            m_ExitFunc(this->getRunTime());
        }
    }

protected:
    std::function<void(const std::string&)> m_ExitFunc;
    std::string m_Caption;
    bool m_bDefaultCaption;
};

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
/*
 * Author:  David Robert Nadeau
 * Site:    http://NadeauSoftware.com/
 * License: Creative Commons Attribution 3.0 Unported License
 *          http://creativecommons.org/licenses/by/3.0/deed.en_US
 */

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/resource.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
#include <fcntl.h>
#include <procfs.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#include <stdio.h>

#endif

#else
#error "Cannot define getPeakRSS( ) or getCurrentRSS( ) for an unknown OS."
#endif

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
class MemoryUsage {
public:
    MemoryUsage() = delete;
    static std::string getMemoryUsageInBytes()
    {
        std::string str;
        str += std::string("Memory usage: ");
        str += std::to_string(static_cast<double>(getCurrentRSS()));
        str += std::string(" MB(s). Peak: ");
        str += std::to_string(static_cast<double>(getPeakRSS())) + " byte(s).";
        return str;
    }

    static std::string getMemoryUsageInMB()
    {
        std::string str;
        str += std::string("Memory usage: ");
        str += std::to_string(static_cast<double>(getCurrentRSS()) / 1048576.0);
        str += std::string(" MB(s). Peak: ");
        str += std::to_string(static_cast<double>(getPeakRSS()) / 1048576.0) + " MB(s).";
        return str;
    }

private:
    //-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /**
     * Returns the peak (maximum so far) resident set size (physical
     * memory use) measured in bytes, or zero if the value cannot be
     * determined on this OS.
     */
    static size_t getPeakRSS()
    {
#if defined(_WIN32)
        /* Windows -------------------------------------------------- */
        PROCESS_MEMORY_COUNTERS info;
        GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
        return (size_t)info.PeakWorkingSetSize;
#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
        /* AIX and Solaris ------------------------------------------ */
        struct psinfo psinfo;
        int fd = -1;

        if((fd = open("/proc/self/psinfo", O_RDONLY)) == -1)
        {
            return (size_t)0L; /* Can't open? */
        }

        if(read(fd, &psinfo, sizeof(psinfo)) != sizeof(psinfo))
        {
            close(fd);
            return (size_t)0L; /* Can't read? */
        }

        close(fd);
        return (size_t)(psinfo.pr_rssize * 1024L);
#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
        /* BSD, Linux, and OSX -------------------------------------- */
        struct rusage rusage;
        getrusage(RUSAGE_SELF, &rusage);
#if defined(__APPLE__) && defined(__MACH__)
        return (size_t)rusage.ru_maxrss;
#else
        return (size_t)(rusage.ru_maxrss * 1024L);
#endif
#else
        /* Unknown OS ----------------------------------------------- */
        return (size_t)0L; /* Unsupported. */
#endif
    }

    //-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    /**
     * Returns the current resident set size (physical memory use) measured
     * in bytes, or zero if the value cannot be determined on this OS.
     */
    static size_t getCurrentRSS()
    {
#if defined(_WIN32)
        /* Windows -------------------------------------------------- */
        PROCESS_MEMORY_COUNTERS info;
        GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
        return (size_t)info.WorkingSetSize;
#elif defined(__APPLE__) && defined(__MACH__)
        /* OSX ------------------------------------------------------ */
        struct mach_task_basic_info info;
        mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;

        if(task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
            (task_info_t)&info, &infoCount) != KERN_SUCCESS)
        {
            return (size_t)0L; /* Can't access? */
        }

        return (size_t)info.resident_size;
#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
        /* Linux ---------------------------------------------------- */
        long rss = 0L;
        FILE* fp  = nullptr;

        if((fp = fopen("/proc/self/statm", "r")) == nullptr)
        {
            return (size_t)0L; /* Can't open? */
        }

        if(fscanf(fp, "%*s%ld", &rss) != 1)
        {
            fclose(fp);
            return (size_t)0L; /* Can't read? */
        }

        fclose(fp);
        return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE);
#else
        /* AIX, BSD, Solaris, and Unknown OS ------------------------ */
        return (size_t)0L; /* Unsupported. */
#endif
    }
}; // end MemoryUsage

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
template<class Data>
void advancePositions(Data& verts)
{
    for(auto& v: verts)
    {
        Vec3d vc  = v - SPHERE_CENTER;
        double mag = vc.norm() * STEP;
        v = SPHERE_CENTER + vc.normalized() * mag;
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
template<bool PRINT, class Data>
void neighborSearch_GridBased(Data& verts, std::vector<std::vector<size_t>>& neighbors, int iter, double& totalTime)
{
    neighbors.resize(verts.size());
    ScopeTimer* timer = nullptr;
    if(PRINT)
    {
        timer = new ScopeTimer("Neighbor search by grid-based approach:");
    }

    const double radius = 4.000000000000001 * PARTICLE_RADIUS;
    static GridBasedNeighborSearch gridSearch;
    gridSearch.setSearchRadius(radius);
    gridSearch.getNeighbors(neighbors, verts);

    if(PRINT)
    {
        std::cout << std::endl;
        std::cout << "------------------------------ ITER " << iter << " ------------------------------\n";
        totalTime += timer->tock();
        delete timer;
        std::cout << MemoryUsage::getMemoryUsageInMB() << std::endl;
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
template<bool PRINT, class Data>
void neighborSearch_SpatialHashing(Data& verts, std::vector<std::vector<size_t>>& neighbors, int iter, double& totalTime)
{
    neighbors.resize(verts.size());
    for(auto& list : neighbors)
    {
        list.resize(0);
    }

    ScopeTimer* timer = nullptr;
    if(PRINT)
    {
        timer = new ScopeTimer("Neighbor search by spatial hashing:");
    }

    const double radius = 4.000000000000001 * PARTICLE_RADIUS;
    static SpatialHashTableSeparateChaining hashTable;
    if(iter == 0)
    {
        hashTable.setCellSize(radius, radius, radius);
        hashTable.insertPoints(verts);
    }
    else
    {
        hashTable.clear();
        hashTable.insertPoints(verts);
    }

    for(size_t p = 0; p < verts.size(); ++p)
    {
        auto& v = verts[p];
        hashTable.getPointsInSphere(neighbors[p], v, radius);
    }

    if(PRINT)
    {
        std::cout << std::endl;
        std::cout << "------------------------------ ITER " << iter << " ------------------------------\n";
        totalTime += timer->tock();
        delete timer;
        std::cout << MemoryUsage::getMemoryUsageInMB() << std::endl;
    }
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
void print(const std::vector<size_t>& data)
{
    for(auto x : data)
    {
        std::cout << x << ", ";
    }
    std::cout << std::endl;
}

void verify(std::vector<std::vector<size_t>>& neighbors1,  std::vector<std::vector<size_t>>& neighbors2)
{
    if(neighbors1.size() != neighbors2.size())
    {
        throw std::runtime_error("Invalid input!");
    }
    for(size_t p = 0; p < neighbors1.size(); ++p)
    {
        auto& list1 = neighbors1[p];
        auto& list2 = neighbors2[p];
        list1.erase(std::remove(list1.begin(), list1.end(), p), list1.end());
        list2.erase(std::remove(list2.begin(), list2.end(), p), list2.end());

        std::sort(list1.begin(), list1.end());
        std::sort(list2.begin(), list2.end());

        if(list1.size() != list2.size())
        {
            std::cout << "List1: " << list1.size() << " neighbors, List2: " << list2.size() << " neighbors\n";
            print(list1);
            print(list2);
            throw std::runtime_error("Not equal number of neighbors!");
        }

        for(size_t idx = 0; idx < list1.size(); ++idx)
        {
            if(list1[idx] != list2[idx])
            {
                print(list1);
                print(list2);
                throw std::runtime_error("Not the same neighbors!");
            }
        }
    }

    std::cout << "All consistent!" << std::endl;
}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
int main()
{
    StdVectorOfVec3d verts;

    const auto particleRadius = double(   PARTICLE_RADIUS);
    const Vec3d sphereCenter   = SPHERE_CENTER;
    const auto sphereRadius   = double(1);
    const auto spacing        = double(2) * particleRadius;
    const int N = int(2 * sphereRadius / spacing);

    verts.reserve(N * N * N);
    const Vec3d corner = sphereCenter - Vec3d(sphereRadius, sphereRadius, sphereRadius);

    for(int i = 0; i < N; ++i)
    {
        for(int j = 0; j < N; ++j)
        {
            for(int k = 0; k < N; ++k)
            {
                const Vec3d ppos = corner + Vec3d(spacing * double(i), spacing * double(j), spacing * double(k));
                const Vec3d d    = ppos - sphereCenter;
                if(d.norm() < sphereRadius)
                {
                    verts.push_back(ppos);
                }
            }
        }
    }

    printf("N Particles: %zu\n", verts.size());
    // writeFile(verts, "D:\\particles.obj");
    //////////////////////////////////////////////////////////////////////////////////////////
    double totalTime1 = 0;
    double totalTime2 = 0;

    for(int iter = 0; iter < ITERATIONS; ++iter)
    {
#ifndef DISABLE_GRID_BASED_SEARCH
        static std::vector<std::vector<size_t>> neighbors1;
        neighborSearch_GridBased<true>(verts, neighbors1, iter, totalTime1);
        if(PRINT_NEIGHBORS)
        {
            size_t p = 0;
            for(auto& pneighbors : neighbors1)
            {
                printf("%zu, %zu neighbors: ", p++, pneighbors.size());
                for(auto q : pneighbors)
                {
                    printf("%zu, ", q);
                }
                printf("\n");
            }
        }

        std::cout << std::endl << "Total time grid-based approach: " << totalTime1 / 1000.0 << std::endl;
#endif

        //////////////////////////////////////////////////////////////////////////////////////////
#ifndef DISABLE_SPATIAL_HASHING
        std::cout << std::endl << std::endl << std::endl;
        static std::vector<std::vector<size_t>> neighbors2;
        neighborSearch_SpatialHashing<true>(verts, neighbors2, iter, totalTime2);
        if(PRINT_NEIGHBORS)
        {
            size_t p = 0;
            for(auto& pneighbors : neighbors2)
            {
                printf("%zu, %zu neighbors: ", p++, pneighbors.size());
                for(auto q : pneighbors)
                {
                    printf("%zu, ", q);
                }
                printf("\n");
            }
        }
        std::cout << std::endl << "Total time spatial hashing approach: " << totalTime2 / 1000.0 << std::endl;
#endif

        //////////////////////////////////////////////////////////////////////////////////////////
#if !defined(DISABLE_SPATIAL_HASHING) && !defined(DISABLE_GRID_BASED_SEARCH)
        std::cout << std::endl << std::endl << std::endl;
        try
        {
            verify(neighbors1, neighbors2);
        }
        catch(std::runtime_error& e)
        {
            std::cout << e.what() << std::endl;
        }
#endif

        advancePositions(verts);
    }
    return 0;
}
