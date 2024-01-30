#pragma once
#include <cassert>
#include <system_error>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

using u8 = uint8_t;

using u16 = uint16_t;

using u32 = uint32_t;

using u64 = uint64_t;

using s8 = int8_t;

using s16 = int16_t;

using s32 = int32_t;

using s64 = int64_t;

using f32 = float;

using f64 = double;

constexpr f32 MAX_F32 = std::numeric_limits<f32>::max();

template <typename T>
using ArrayList = std::vector<T>;

template <typename T, typename K, typename... Args>
using HashMap = std::unordered_map<T, K, Args...>;

struct Error {
	std::error_code type;
	VkResult vkResult =
		VK_SUCCESS; // optional error value if a vulkan call failed
};

template <typename T>
class Result {

public:
	Result(const T& value) noexcept : mValue{value}, mValid{true} {}

	Result(T&& value) noexcept : mValue{std::move(value)}, mValid{true} {}

	Result(Error error) noexcept : mError{error}, mValid{false} {}

	Result(VkResult result) noexcept {
		if (result != VK_SUCCESS) {
			mError = {{}, result};
			mValid = false;
		}
	}

	Result(std::error_code error_code, VkResult result = VK_SUCCESS) noexcept :
		mError{error_code, result}, mValid{false} {}

	~Result() noexcept { destroy(); }

	Result(Result const& expected) noexcept : mValid(expected.mValid) {
		if (mValid)
			new (&mValue) T{expected.mValue};
		else
			mError = expected.mError;
	}

	Result& operator=(Result const& result) noexcept {
		mValid = result.mValid;
		if (mValid)
			new (&mValue) T{result.mValue};
		else
			mError = result.mError;
	}

	Result(Result&& expected) noexcept : mValid(expected.mValid) {
		if (mValid)
			new (&mValue) T{std::move(expected.mValue)};
		else
			mError = std::move(expected.mError);
		expected.destroy();
	}

	Result& operator=(Result&& result) noexcept {
		mValid = result.mValid;
		if (mValid)
			new (&mValue) T{std::move(result.mValue)};
		else
			mError = std::move(result.mError);
	}

	Result& operator=(const T& expect) noexcept {
		destroy();
		mValid = true;
		new (&mValue) T{expect};
		return *this;
	}

	Result& operator=(T&& expect) noexcept {
		destroy();
		mValid = true;
		new (&mValue) T{std::move(expect)};
		return *this;
	}

	Result& operator=(const Error& error) noexcept {
		destroy();
		mValid = false;
		mError = error;
		return *this;
	}

	Result& operator=(Error&& error) noexcept {
		destroy();
		mValid = false;
		mError = error;
		return *this;
	}

	Result& operator=(const VkResult& res) noexcept {
		if (res != VK_SUCCESS) {
			destroy();
			mError = {{}, res};
			mValid = false;
		}
		return *this;
	}

	Result& operator=(VkResult&& res) noexcept {
		if (res != VK_SUCCESS) {
			destroy();
			mError = {{}, res};
			mValid = false;
		}
		return *this;
	}

	// clang-format off
	const T* operator-> () const noexcept { assert (mValid); return &mValue; }
	T*       operator-> ()       noexcept { assert (mValid); return &mValue; }
	const T& operator* () const& noexcept { assert (mValid);	return mValue; }
	T&       operator* () &      noexcept { assert (mValid); return mValue; }
	T&&      operator* () &&	 noexcept { assert (mValid); return std::move (mValue); }
	const T&  value () const&    noexcept { assert (mValid); return mValue; }
	T&        value () &         noexcept { assert (mValid); return mValue; }
	const T&& value () const&&   noexcept { assert (mValid); return std::move (mValue); }
	T&&       value () &&        noexcept { assert (mValid); return std::move (mValue); }

    // std::error_code associated with the error
    std::error_code error() const { assert (!mValid); return mError.type; }

    // optional VkResult that could of been produced due to the error
    VkResult vk_result() const { assert (!mValid); return mError.vkResult; }

    // Returns the struct that holds the std::error_code and VkResult
    Error full_error() const { assert (!mValid); return mError; }
	// clang-format on

	// check if the result has an error that matches a specific error case
	template <typename E>
	bool matches_error(E error_enum_value) const {
		return !mValid &&
			static_cast<E>(mError.type.value()) == error_enum_value;
	}

	bool has_value() const { return mValid; }
	explicit operator bool() const { return mValid; }

private:
	void destroy() {
		if (mValid)
			mValue.~T();
	}
	union {
		T mValue;
		Error mError;
	};
	bool mValid;
};

enum class ObjectLifetime : u8 {
	TEMP = 0,
	OWNED = 1,
};
