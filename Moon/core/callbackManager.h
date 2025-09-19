#pragma once

#include <utility>
#include <atomic>
#include <functional>
#include <mutex>
#include <type_traits>
#include <vector>
#include <assert.h>
namespace MOON {

    class CallBackManager {
    public:
        class CallBack;
        using CallBackFunc = void(*)(void*, CallBack*);
        class alignas(64) CallBack {
        public:
            CallBack() noexcept {}
            //CallBack(const CallBack&) = delete;
            //CallBack(CallBack&&) = delete;
            

        private:
            friend CallBackManager;
            void* storage[8];
            CallBackFunc function;
        };
        static CallBackManager& instance();
        void exectue();
        CallBack& create(CallBackFunc func);
       
        template<typename T,void(T::*method)(CallBack*)>
        CallBack& createCallBack(T*data) {
            CallBack& call = create(+[](void* storage, CallBack* callback) {
                T* const that = static_cast<T*>(static_cast<void**>(storage)[0]);
                (that->*method)( callback);
                });
            call.storage[0] = data;
            return call;
        }
        
        template<typename T, void(T::* method)(CallBack*)>
        CallBack& createCallBack(T data) {
            CallBack& call = create(+[](void* storage, CallBack* callback) {
                T* const that = static_cast<T*>(static_cast<void**>(storage)[0]);
                (that->*method)(callback);
                });
            new (call.storage) T(std::move(data));
            return call;
        }

        template<typename T, void(T::* method)(CallBack*), typename ... ARGS>
        CallBack& emplaceCallBack( ARGS&& ... args) noexcept {
            static_assert(sizeof(T) <= sizeof(CallBack::storage), "user data too large");
            CallBack& call = create([](void* storage, CallBack* call){
                T* const that = static_cast<T*>(storage);
                (that->*method)( call);
                that->~T();
                });
            new(call.storage) T(std::forward<ARGS>(args)...);
            return call;
        }
      
        template<typename T>
        CallBack* createCallBack( T functor) noexcept {
            static_assert(sizeof(functor) <= sizeof(CallBack::storage), "functor too large");
            CallBack& call = create( [](void* storage, CallBack* call) {
                T* const that = static_cast<T*>(storage);
                that->operator()( call);
                that->~T();
                });
             new(call.storage) T(std::move(functor));
             return call;
        }


        template<typename T, typename ... ARGS>
        CallBack& emplaceCallBack( ARGS&& ... args) noexcept {
            static_assert(sizeof(T) <= sizeof(CallBack::storage), "functor too large");
            CallBack& call = create( [](void* storage, CallBack* call) {
                T* const that = static_cast<T*>(storage);
                that->operator()( call);
                that->~T();
                });
            new(call.storage) T(std::forward<ARGS>(args)...);
            return call;
        }

    private:
        CallBackManager();
    private:
        std::vector<CallBack>mCallBacks;
    };


    template<typename CALLABLE, typename ... ARGS>
    CallBackManager::CallBack& createCallBack(CallBackManager& manager,
        CALLABLE&& func, ARGS&&... args) noexcept {
        struct Data {
            explicit Data(std::function<void()> f) noexcept : f(std::move(f)) {}
            std::function<void()> f;
            void gob(CallBackManager::CallBack*) noexcept { f(); }
        };
        return manager.emplaceCallBack<Data,&Data::gob>(std::bind(std::forward<CALLABLE>(func), std::forward<ARGS>(args)...));
    }

    template<typename CALLABLE, typename T, typename ... ARGS,typename = std::enable_if_t<std::is_member_function_pointer_v<std::remove_reference_t<CALLABLE>>>>
    CallBackManager::CallBack* createCallBack(CallBackManager& manager,
        CALLABLE&& func, T&& o, ARGS&&... args) noexcept {
        struct Data {
            explicit Data(std::function<void()> f) noexcept : f(std::move(f)) {}
            std::function<void()> f;
           
            void gob(CallBackManager::CallBack*) noexcept { f(); }
        };
        return manager.emplaceCallBack<Data, &Data::gob>(std::bind(std::forward<CALLABLE>(func), std::forward<T>(o), std::forward<ARGS>(args)...));
    }
}