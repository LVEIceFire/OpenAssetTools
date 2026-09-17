#include "Zone/XBlock.h"

#include <algorithm>
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <utility>

namespace
{
    TEST_CASE("Zone block buffers remain aligned across repeated allocations", "[zone-loading][stream][stress]")
    {
        constexpr std::array<size_t, 12> sizes{1u, 7u, 15u, 16u, 17u, 31u, 32u, 33u, 4095u, 4096u, 4097u, 65537u};
        XBlock block("stress", 0u, XBlockType::BLOCK_TYPE_NORMAL);

        for (unsigned round = 0; round < 128u; round++)
        {
            for (const auto size : sizes)
            {
                block.Alloc(size);
                auto* const data = block.m_buffer.get();
                REQUIRE(data != nullptr);
                REQUIRE(reinterpret_cast<std::uintptr_t>(data) % 16u == 0u);
                REQUIRE(block.m_buffer_size == size);

                const auto value = static_cast<std::uint8_t>(round);
                std::fill_n(data, size, value);
                REQUIRE(data[0] == value);
                REQUIRE(data[size - 1u] == value);
            }

            block.Alloc(0u);
            REQUIRE(block.m_buffer == nullptr);
            REQUIRE(block.m_buffer_size == 0u);
        }

        block.Alloc(4097u);
        auto* const originalAddress = block.m_buffer.get();
        XBlock moved(std::move(block));
        REQUIRE(moved.m_buffer.get() == originalAddress);
        REQUIRE(reinterpret_cast<std::uintptr_t>(moved.m_buffer.get()) % 16u == 0u);
        REQUIRE(moved.m_buffer_size == 4097u);
    }
} // namespace
