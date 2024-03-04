// Copyright (c) 2024 Huawei Technologies Co. All rights reserved.
// Use of this source code is governed by an MIT license
// that can be found in the LICENSE file.

#ifndef ELFIN_PARSE_COMMON_SHARED_WEAK_H
#define ELFIN_PARSE_COMMON_SHARED_WEAK_H


#include <memory>

namespace elfin {
template<typename IMPLT>
class SharedWeakPImpl{
public:
    SharedWeakPImpl(const std::shared_ptr<IMPLT> &impl):mShared(impl){}
    SharedWeakPImpl(const std::weak_ptr<IMPLT> &impl):mWeak(impl){}

    bool IsShared() const
    {
        return mShared.use_count() != 0;
    }

    bool IsWeak() const
    {
        return !mWeak.expired();
    }

    std::shared_ptr<IMPLT> GetShared() const 
    {
        if(IsShared()){
            return mShared;
        }
        if(IsWeak()){
            return mWeak.lock();
        }
        return nullptr;
    }

    std::weak_ptr<IMPLT> GetWeak() const
    {
        if(IsShared()){
            return mShared;
        }
        if(IsWeak()){
            return mWeak;
        }
        return {};
    }

    IMPLT &Get() const
    {
        if(IsShared()){
            return *mShared;
        }
        return *mWeak.lock();
    }

    operator bool() const
    {
        return IsShared() || IsWeak();
    }

    void ToWeak()
    {
        if(!IsShared()){
            return;
        }
        mWeak = mShared;
        mShared.reset();
    }

private:
    std::shared_ptr<IMPLT> mShared;
    std::weak_ptr<IMPLT> mWeak;

};
}

#endif /*ELFIN_PARSE_COMMON_SHARED_WEAK_H*/
