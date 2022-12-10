#include "SPL/Utils.hpp"


namespace sp{ // BEGINING OF NAMESPACE //////////////////////////////////////////////////////////////////


class GeneralStorage{
public:
	inline constexpr GeneralStorage() noexcept;
	inline GeneralStorage(const uint32_t requiredCapacity) noexcept;
	inline ~GeneralStorage();
	
	GeneralStorage(const GeneralStorage &) = delete;
	GeneralStorage(GeneralStorage &&) = delete;
	GeneralStorage &operator =(const GeneralStorage &) = delete;
	GeneralStorage &operator =(GeneralStorage &&) = delete;
	
	inline void resize(const uint32_t requiredSize) noexcept;
	inline void reallocate(const uint32_t requiredSize) noexcept;

	template<class T>
	inline constexpr T &at(const uint32_t index) noexcept;

	template<class T>
	inline constexpr T *data() noexcept;

	inline constexpr void *data() noexcept;

private:
	void *dataPtr;
	uint32_t capacity;
};



#define CLS GeneralStorage

inline constexpr CLS::GeneralStorage() noexcept : dataPtr{nullptr}, capacity{0} {}

inline CLS::GeneralStorage(const uint32_t requiredCapacity) noexcept : dataPtr{malloc(requiredCapacity)}, capacity{requiredCapacity} {}
	
inline CLS::~GeneralStorage(){
	if (dataPtr)
		free(dataPtr);
}

inline void CLS::resize(const uint32_t requiredSize) noexcept{
	if (requiredSize > capacity){
		if (dataPtr)
			free(dataPtr);
		dataPtr = malloc(requiredSize);
		capacity = requiredSize;
	}
}

inline void CLS::reallocate(const uint32_t requiredSize) noexcept{
	if (dataPtr)
		free(dataPtr);
	dataPtr = malloc(requiredSize);
	capacity = requiredSize;
}

template<class T>
inline constexpr T &CLS::at(const uint32_t index) noexcept{
	return ((T *)dataPtr)[index];
}

template<class T>
inline constexpr T *CLS::data() noexcept{
	return (T *)dataPtr;
}

inline constexpr void *CLS::data() noexcept{
	return dataPtr;
}


#undef CLS


}	// END OF NAMESPACE	///////////////////////////////////////////////////////////////////