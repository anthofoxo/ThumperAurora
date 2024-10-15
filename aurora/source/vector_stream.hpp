#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>
#include <span>
#include <filesystem>
#include <fstream>
#include <optional>

namespace aurora {
	class VectorStream final {
	public:
		static std::optional<VectorStream> from_file(std::filesystem::path const& path) {
			std::ifstream file(path, std::ios::in | std::ios::binary);
			if (!file) return std::nullopt;

			VectorStream stream;
			file.seekg(0, std::ios::end);
			stream.mData.resize(file.tellg());
			file.seekg(0, std::ios::beg);
			file.read(reinterpret_cast<char*>(stream.mData.data()), stream.mData.size());
			return stream;
		}

		void to_file(std::filesystem::path const& path) const {
			std::ofstream stream(path, std::ios::out | std::ios::binary);
			stream.write(reinterpret_cast<char const*>(mData.data()), mData.size());
		}

		std::span<std::byte> read_bytes(size_t count) {
			std::span<std::byte> v = std::span(mData.data() + mMark, count);
			mMark += count;
			return v;
		}

		uint16_t read_u16() {
			uint16_t v = *reinterpret_cast<uint16_t*>(mData.data() + mMark);
			mMark += sizeof(v);
			return v;
		}

		uint32_t read_u32() {
			uint32_t v = *reinterpret_cast<uint32_t*>(mData.data() + mMark);
			mMark += sizeof(v);
			return v;
		}

		void write_bytes(std::span<std::byte const> v) {
			mData.append_range(v);
		}

		void write_u16(uint16_t v) {
			std::span<std::byte> view = std::span(reinterpret_cast<std::byte*>(&v), sizeof(v));
			mData.append_range(view);
		}

		void write_u32(uint32_t v) {
			std::span<std::byte> view = std::span(reinterpret_cast<std::byte*>(&v), sizeof(v));
			mData.append_range(view);
		}
	private:
		size_t mMark = 0;
		std::vector<std::byte> mData;
	};
}