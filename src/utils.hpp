#pragma once

#include <array>
#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

#include <fmt/format.h>

#include <Geode/utils/general.hpp>

// TODO: remove this whenever geode's numToAbbreviatedString util gets updated
// this was pr'd to geode but as of v5.9.0, it hasn't been released, so i'm adding it to my own code
// https://github.com/geode-sdk/geode/pull/2160/
namespace stats::utils {
    template <std::integral Num> requires (!std::same_as<std::remove_cv_t<Num>, bool>)
    std::string convertNumToAbbreviatedString(Num num) {
        bool negative = false;
        uint64_t abs = 0;

        if constexpr (std::is_signed_v<Num>) {
            if (num < 0) {
                negative = true;
                abs = static_cast<uint64_t>(0) - static_cast<uint64_t>(num);
            }
            else {
                abs = static_cast<uint64_t>(num);
            }
        }

        struct Scale {
            uint64_t threshold;
            std::string_view suffix;
        };

        constexpr auto scales = std::to_array<Scale>({
            {1'000'000'000'000'000ull, "Q"},
            {1'000'000'000'000ull, "T"},
            {1'000'000'000ull, "B"},
            {1'000'000ull, "M"},
            {1'000ull, "K"},
        });

        for (auto const& [threshold, suffix] : scales) {
            if (abs < threshold) {
                continue;
            }
            uint64_t whole = abs / threshold;
            uint64_t rem = abs % threshold;
            std::string result;
            if (whole >= 100) {
                result = fmt::format("{}{}", whole, suffix);
            }
            else if (whole >= 10) {
                uint64_t frac = rem / (threshold / 10);
                result = fmt::format("{}.{}{}", whole, frac, suffix);
            }
            else {
                uint64_t frac = rem / (threshold / 100);
                result = fmt::format("{}.{:02d}{}", whole, frac, suffix);
            }
            return negative ? fmt::format("-{}", result) : result;
        }
        return geode::utils::numToString(num);
    }
}