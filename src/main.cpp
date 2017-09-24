#include <iostream>
#include <thread>
#include <sstream>
#include <unistd.h>
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>
#include <g3sinks/LogRotate.h>
#include <g3log/std2_make_unique.hpp>

//#include "Stopwatch.hpp"

#include <chrono>
#include <iostream>


template <class resolution> class Stopwatch_res
{
    std::chrono::time_point<std::chrono::steady_clock> stopped_;
    std::chrono::time_point<std::chrono::steady_clock> started_;

public:
    Stopwatch_res()
    {
        reset();
    }
    void reset()
    {
        started_ = std::chrono::steady_clock::now();
        stopped_ = started_;
    }
    void stop()
    {
        stopped_ = std::chrono::steady_clock::now();
    }
    std::chrono::time_point<std::chrono::steady_clock> now()
    {
        return std::chrono::steady_clock::now();
    }
    int64_t elapsed(bool stop_now)
    {
        if (stop_now) {
            stop();
        }
        return std::chrono::duration_cast<resolution>(stopped_ - started_).count();
    }

    int64_t elapsed()
    {
        return elapsed(true);
    }

    static void describe(std::ostream& out)
    {
        out << "High Resolution Clock has " <<  std::chrono::high_resolution_clock::period::den << " ticks per second" << std::endl;
        auto freq = 1.0 / std::chrono::high_resolution_clock::period::den;
        out << "High Resolution Clock Frequency " << freq << std::endl;
        out << "Steady Clock has " <<  std::chrono::steady_clock::period::den << " ticks per second" << std::endl;
        freq = 1.0 / std::chrono::steady_clock::period::den;
        out << "Steady Clock Frequency " << freq << std::endl;
        out << "This implementation uses std::chrono::steady_clock" << std::endl;
    }
    static const std::string& units();
    static double freq();
};

// Template specialization for slightly easier use by clients
typedef Stopwatch_res<std::chrono::nanoseconds> Stopwatch_ns;
typedef Stopwatch_res<std::chrono::microseconds> Stopwatch_us;
typedef Stopwatch_res<std::chrono::milliseconds> Stopwatch_ms;
typedef Stopwatch_res<std::chrono::seconds> Stopwatch_s;
// default to the highest resolution
typedef Stopwatch_res<std::chrono::nanoseconds> Stopwatch;
// unit string for logging
template<> const std::string& Stopwatch_ns::units()
{
    static std::string unit_string(" ns");
    return unit_string;
}
template<> const std::string& Stopwatch_us::units()
{
    static std::string unit_string(" µs");
    return unit_string;
}
template<> const std::string& Stopwatch_ms::units()
{
    static std::string unit_string(" ms");
    return unit_string;
}
template<> const std::string& Stopwatch_s::units()
{
    static std::string unit_string(" s");
    return unit_string;
}
// Return frequency to scale to seconds
template<> double Stopwatch_ns::freq()
{
    return double(1.0)/1e9;
}
template<> double Stopwatch_us::freq()
{
    return double(1.0)/1e6;
}
template<> double Stopwatch_ms::freq()
{
    return double(1.0)/1e3;
}
template<> double Stopwatch_s::freq()
{
    return 1;
}



void thread_func(int my_tid)
{
    static const unsigned int num_logs_per_thread = 10;
    LOG(INFO) << "Launching thread " << my_tid;
    for (unsigned int i = 0; i < num_logs_per_thread; ++i) {
        LOG(INFO) << "Log msg " << i << " from thread " << my_tid;
        usleep(10); // 10 micro * rand? would that add anything
    }

    LOG(INFO) << "Finishing thread " << my_tid;
    std::cout << "Finishing thread" << std::endl;
}


int main(int argc, char* argv[])
{
    int ret_val = 0;
    std::cout << "g3log_explore" << std::endl;

    using namespace g3;

    std::unique_ptr<LogWorker> logworker{ LogWorker::createLogWorker() };

    auto sinkHandle = logworker->addSink(std2::make_unique<LogRotate>("g3log_explore", "log_dir"),
                                         &LogRotate::save);

    // initialize the logger before it can receive LOG calls
    initializeLogging(logworker.get());
    // You can call in a thread safe manner public functions on the logrotate sink
    // The call is asynchronously executed on your custom sink.
    const int k10MBInBytes = 10 * 1024 * 1024;
    std::future<void> received = sinkHandle->call(&LogRotate::setMaxLogSize, k10MBInBytes);

    // Run the main part of the application. This can be anything of course, in this example
    // we'll call it "RunApplication". Once this call exits we are in shutdown mode
    for (auto i = 0; i < 50; ++i) {
        LOG(INFO)  << "Info Logging message " << ++i;
        LOG(WARNING) << "Warning Logging message " << ++i;
        LOG(DEBUG) << "Logging message " << i;
    }

    static const int num_threads = 15;
    std::thread t[num_threads];

    LOG(INFO) << "Launching " << num_threads << " threads from main" << std::endl;
    //Launch a group of threads
    for (int i = 0; i < num_threads; ++i) {
        t[i] = std::thread(thread_func, i);
    }

    for (int i = 0; i < num_threads; ++i) {
        t[i].join();
    }

 
    std::ostringstream ss;
    Stopwatch::describe(ss);
    LOG(INFO) << ss.str();

    Stopwatch timer;
    timer.reset();
    for (unsigned int i = 0; i < 1e5; i++) {
        LOG(INFO) << "Logging";
    }
    auto elapsed = timer.elapsed();
    auto average = elapsed / 1e5;

    LOG(INFO) << 1e5 << " log messages took  took: " << elapsed << " nanoseconds";
    LOG(INFO) << "Average " << average << "ns";

    timer.reset();
    auto delta =  timer.elapsed();
    LOG(INFO) << "Successive calls to now took " << delta << " nanoseconds";

    timer.reset();
    usleep(2000);
    auto sleepTime = timer.elapsed();

    LOG(INFO) << "usleep(2000) took: " << sleepTime << " nanoseconds";
    LOG(INFO) << "usleep(2000) took: " << sleepTime / 1e3 << " microseconds";
    LOG(INFO) << "usleep(2000) took: " << sleepTime / 1e6  << " milliseconds";
    LOG(INFO) << "usleep(2000) took: " << sleepTime / 1e9  << " seconds";

    timer.reset();
    sleep(2);
    sleepTime = timer.elapsed();
    LOG(INFO) << "sleep(2) took: " << sleepTime << " nanoseconds";
    LOG(INFO) << "sleep(2) took: " << sleepTime / 1e3 << " microseconds";
    LOG(INFO) << "sleep(2) took: " << sleepTime / 1e6 << " milliseconds";
    LOG(INFO) << "sleep(2) took: " << sleepTime / 1e9 << " seconds";
    return ret_val;
}