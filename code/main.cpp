#include "utils.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>

#include <vector>
#include <list>




class LutItem
{
	static inline constexpr uint32_t ADDBUFFER = 512;
	static constexpr uint32_t LUT_SEGMENTS = 8;

	std::array<std::vector<uint64_t>, LUT_SEGMENTS> m_mem;

	bool m_error{};

public:

	LutItem() : m_error{ true } {}

	LutItem(std::size_t size) : m_error{ false }
	{
		std::size_t memMaxSize = size / LUT_SEGMENTS + ADDBUFFER;
		for (auto& m : m_mem)
		{
			m.reserve(memMaxSize);
		}
	}

	bool isError() { return m_error; }
	void append(uint32_t mem, uint64_t val)
	{
		m_mem[mem].push_back(val);
	}

	constexpr uint32_t memCount() const { return LUT_SEGMENTS; }
	const std::vector<uint64_t>& mem(uint32_t no) const { return m_mem[no]; }

	uint8_t* memPtr(uint32_t no) { return reinterpret_cast<uint8_t*>(m_mem[no].data()); };


};


class Counter
{

	static constexpr uint32_t mmmp[2][2][2]{ {{0, 1}, {2, 3}}, {{4, 5}, {6, 7}} };

	uint32_t m_count{ 0 };
	uint32_t m_size{ 0 };
	uint32_t sizesize{ 0 };


	uint32_t column() const { return m_count % m_size; }
	uint32_t row() const { return (m_count % sizesize) / m_size; }
	uint32_t plane() const { return m_count / (sizesize); };

public:

	void setSize(uint32_t s) { m_size = s; sizesize = m_size * m_size; m_count = 0; }
	uint32_t total() const { return m_count; }
	uint32_t increment() { return ++m_count; }
	uint32_t memno() const { return mmmp[plane() % 2][row() % 2][column() % 2]; }

};


LutItem loadLut(const std::string& fname)
{

	if (!std::filesystem::exists(fname))
	{
		std::cout << "File : " << fname << " missing!\n";
		return {};
	}

	std::ifstream file(fname);

	if (!file.is_open())
	{
		std::cout << "Failed to open " << fname << "\n";
		return {};
	}

	Counter cntr;
	LutItem result;

	std::string line;
	uint32_t lineNo = 0;

	bool started{};
	uint32_t totalSize{ 0 };

	while (std::getline(file, line))
	{
		lineNo++;

		line = trim(line);

		if (line.empty())
		{
			continue;
		}

		if (line[0] == '#')
		{
			continue;
		}

		auto lineWords = split(line, ' ');

		if (lineWords.empty())
		{
			continue;
		}

		if (lineWords.front() == "TITLE")
		{
			continue;
		}

		if (lineWords.front() == "LUT_3D_SIZE")
		{
			if (started)
			{
				std::cout << "Line:" << lineNo << " duplicated LUT_3D_SIZE tag (1).\n";
				return {};
			}


			if (lineWords.size() != 2)
			{
				std::cout << "Line:" << lineNo << " Incorrect file format. LUT_3D_SIZE tag (1).\n";
				return {};
			}

			auto lutSize = text2int(lineWords[1]);
			if (!lutSize)
			{
				std::cout << "Line:" << lineNo << " Incorrect file format. LUT_3D_SIZE tag (2).\n";
				return {};
			}
			cntr.setSize(*lutSize);

			totalSize = static_cast<uint32_t>(pow(*lutSize, 3));

			started = true;
			result = LutItem(static_cast<std::size_t>(totalSize));
			continue;
		}

		if (lineWords.size() != 3)
		{
			std::cout << "Line:" << lineNo << " Incorrect file format.. data.\n";
			return {};
		}

		if (!started)
		{
			std::cout << "Line:" << lineNo << " Unrecognized data...\n";
			return {};
		}

		auto r = text2float(lineWords[0]);
		auto g = text2float(lineWords[1]);
		auto b = text2float(lineWords[2]);

		if (!r || !g || !b ||
			*r > 1.0f || *g > 1.0f || *g > 1.0f ||
			*r < 0.0f || *g < 0.0f || *g < 0.0f)
		{
			std::cout << "Line:" << lineNo << " Incorrect data.\n";
			return {};
		}

		uint64_t rgb = std::clamp(static_cast<uint64_t>((*b * 4096) + 0.5), 0ull, 4095ull);
		rgb <<= 12;
		rgb |= std::clamp(static_cast<uint64_t>((*g * 4096) + 0.5), 0ull, 4095ull);
		rgb <<= 12;
		rgb |= std::clamp(static_cast<uint64_t>((*r * 4096) + 0.5), 0ull, 4095ull);

		result.append(cntr.memno(), rgb);

		if (cntr.increment() > totalSize)
		{
			break;
		}
	}

	return result;

}

void process(const LutItem& lut)
{
	for (uint32_t inx = 0; inx < lut.memCount(); inx++)
	{
		const auto& mem = lut.mem(inx);

		std::cout << "\nMEM: " << inx << " ------------------------------------------------------------------------\n";


		for (const auto [i, memElem64] : mem | std::views::enumerate)
		{


			if (i % 4 == 0)
			{
				std::cout << "\n";
			}

			std::cout << std::format("{:16x} ", memElem64);
		}
		std::cout << "\n";
	}

}

int main(int argc, const char* argv[])
{
	std::vector commandLine(argv, argc + argv);

	if (commandLine.size() < 2)
	{
		std::cout << "File name required...\n";
		return 0;
	}
	std::cout << "lut:\t\t" << commandLine[1] << "\n";

	auto lut = loadLut(commandLine[1]);

	if (lut.isError())
	{
		return 0;
	}

	process(lut);
	
	return 0;

}



