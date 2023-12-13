/*
THIS IS ALTERED EXPIRATION CACHE
GCC HAD A PROBLEM WITH FRIEND CLASS SO I SLIGHTLY ALTERED IT
YOU CAN GET THE ORIGINAL CODE AT https://github.com/zapredelom/expirationCache/tree/master
*/


#pragma once
#include <chrono>
#include <map>
#include <list>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <algorithm>

class Synchrophasotron
{
    template <typename KeyType, typename ValueType, int TTL>
    friend class ExpirationCache;

private:
    Synchrophasotron()
    {
    }

    virtual int getTTL() const = 0;
    virtual void ClearAll() = 0;
    virtual int Count() const = 0;

    static void RemoveExpiredElements()
    {
        int count = 1;
        while (!endExecution())
        {
            count++;
            for (auto &i : caches())
            {
                if (count % i->getTTL() == 0)
                {
                    i->ClearAll();
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    virtual ~Synchrophasotron()
    {
        if (caches().size() == 0)
        {
            endExecution() = true;
            removeingThread().join();
        }
    }

protected:
    static std::thread &removeingThread() { static std::thread t; return t; }
    static std::list<Synchrophasotron *> &caches() { static std::list<Synchrophasotron *> t; return t; }
    static bool &endExecution() { static bool t = false; return t; }
};

template <typename _KeyType, typename _ValueType, int TTL>
class ExpirationCache : public Synchrophasotron
{
public:
    typedef _KeyType KeyType;
    typedef _ValueType ValueType;

public:
    ExpirationCache()
    {
        caches().push_back(this);

        if (caches().size() == 1)
        {
            endExecution() = false;
            removeingThread() = std::thread(&Synchrophasotron::RemoveExpiredElements);
        }
    };

    ValueType Get(const KeyType &ky) const
    {
        std::lock_guard<std::mutex> lock(mt_);
        return map_.at(ky);
    }

    void Put(const KeyType &ky, const ValueType &val)
    {
        std::lock_guard<std::mutex> lock(mt_);
        map_[ky] = val;
    }

    void Put(const KeyType &ky, ValueType &&val)
    {
        std::lock_guard<std::mutex> lock(mt_);
        map_[ky] = std::move(val);
    }

    ~ExpirationCache()
    {
        auto ret = std::find(caches().begin(), caches().end(), this);
        if (ret != caches().end())
        {
            (*ret)->ClearAll();
            caches().erase(ret);
        }
    }

    virtual int Count() const override
    {
        std::lock_guard<std::mutex> lock(mt_);
        return map_.size();
    }

private:
    virtual int getTTL() const override { return TTL; }
    virtual void ClearAll() override { std::lock_guard<std::mutex> lock(mt_); map_.clear(); }

private:
    std::map<KeyType, ValueType> map_;
    mutable std::mutex mt_;
};
