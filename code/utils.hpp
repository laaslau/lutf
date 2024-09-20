#pragma once

#include <string>
#include <optional>
#include <regex>
#include <ranges>
#include <algorithm>

std::string trim(const std::string& str)
{
	return std::regex_replace(str, std::regex("^\\s+|\\s+$"), "");
}

auto split(const auto& data, const auto delim)
{
	auto splitView = data | std::views::split(delim) | std::views::filter([](const auto& word) {return !word.empty(); });
	std::vector<std::string> result;
	for (const auto& v : splitView)
	{
		result.emplace_back(v.begin(), v.end());
	}
	return result;
}

std::optional<int> text2int(const std::string& num)
{
	try
	{
		return { std::stoi(num) };
	}
	catch (const std::exception&)
	{
	}
	return std::nullopt;

}

std::optional<float> text2float(const std::string& num)
{
	try
	{
		return { std::stof(num) };
	}
	catch (const std::exception&)
	{
	}
	return std::nullopt;
}

