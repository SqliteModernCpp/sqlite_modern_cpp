
#ifndef SQLITE_COLLECTORS_H
#define SQLITE_COLLECTORS_H

#include <sqlite_modern_cpp.h>

#if __cplusplus > 201402
#define CPP14_SUPPORTED
#endif

#ifdef CPP14_SUPPORTED
#define __COLLECTOR_RETURN_TYPE auto
#define __COLLECTOR_CTOR_OF_T(T,X)		{X}
#else
#define __COLLECTOR_RETURN_TYPE 	std::function<void(Args...)>
#define __COLLECTOR_CTOR_OF_T(T,X)		T(X)
#endif

template <typename ...Args>
struct collect
{
	template<typename T, template<class, class...> class C>
	inline static
	__COLLECTOR_RETURN_TYPE
	to(C<T> &container)
	{
		return [&container](const Args&... args)
		{
			container.push_back(__COLLECTOR_CTOR_OF_T(T,args...));
		};
	}

	template<typename T, template<class, class...> class C>
	inline static
	__COLLECTOR_RETURN_TYPE
	to(C<std::shared_ptr<T>> &container)
	{
		return [&container](const Args&... args)
		{
			container.push_back(std::make_shared<T>(args...));
		};
	}

	template<template<class, class...> class C, typename T >
	struct as
	{
		friend C<T> operator>>(sqlite::database_binder& db_binder, as<C,T>)
		{
			C<T> container;
			db_binder>>to(container);
			return container;
		}
		
		friend C<T> operator>>(sqlite::database_binder&& db_binder, as<C,T>)
		{
			return db_binder>>as<C,T>();
		}
	};
	
	template<typename T >
	struct as<std::shared_ptr,T>
	{
		friend std::shared_ptr<T> operator>>(sqlite::database_binder& db_binder, as<std::shared_ptr,T>)
		{
			std::shared_ptr<T> objP;
			db_binder>>[&objP](Args... args)
			{
				objP = std::make_shared<T>(args...);
			};
			return objP;
		}

		friend std::shared_ptr<T> operator>>(sqlite::database_binder&& db_binder, as<std::shared_ptr,T> whatever)
		{
			return db_binder>>whatever;
		}

	};
};


#endif
