#ifndef fileIO_helper
#define fileIO_helper

#include <bitset>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <cassert>
#include <cstring>

// size of the buffers used to read from file
static constexpr std::size_t buf_size = 255;

// Prototypes
template <typename T> static inline void write_vector_to_file(const std::string &filename, const std::vector<T> &vec);

template <typename T> static inline void write_array_to_file(const std::string &filename, T *arr, size_t size);

template <typename T> static inline void write_value_to_file(const std::string &filename, T val);
template <> inline void write_value_to_file<const char *>(const std::string &filename, const char *val);

template <typename T> static inline void append_value_to_file(const std::string &filename, T val);

static inline std::string read_line_from_file(const std::string &filename);
template <typename T> static inline std::vector<T> read_lines_from_file(const std::string &filename);

template <typename T> static inline T string_to_T(const std::string &s, std::size_t &done);
// template <> inline unsigned long string_to_T<unsigned long>(const std::string &s, std::size_t &done);
template <> inline int string_to_T<int>(const std::string &s, std::size_t &done);

template <std::size_t N>
static inline void write_bitset_to_file(const std::string &filename, const std::bitset<N> &bits);

template <typename T> static inline void write_vector_to_file(const std::string &filename, const std::vector<T> &vec) {
	write_array_to_file(filename, &vec[0], vec.size());
}

template <typename T> static inline void write_array_to_file(const std::string &filename, T *arr, size_t size) {
	assert(size > 0);
	assert(filename != "");

	std::string str;
	for (size_t i = 0; i < size; ++i) {
		str.append(std::to_string(arr[i]));
		str.append(",");
	}

	write_value_to_file(filename, str.c_str());
}

template <typename T> static inline void append_value_to_file(const std::string &filename, T val) {
	assert(filename != "");

	FILE *f = fopen(filename.c_str(), "a+");
	if (f == nullptr) {
		throw std::runtime_error(strerror(errno));
	}
	std::string str = std::to_string(val);

	if (fputs(str.c_str(), f) == EOF && ferror(f) != 0) {
		// try to close the file, but return the old error
		auto err = errno;
		fclose(f);

		throw std::runtime_error(strerror(err));
	}
	if (fclose(f) != 0) {
		throw std::runtime_error(strerror(errno));
	}
}

template <std::size_t N>
static inline void write_bitset_to_file(const std::string &filename, const std::bitset<N> &bits) {
	static_assert(N < 65, "Maximum bitset size is currently 64");
	auto val = bits.to_ullong();
	std::stringstream sstream;
	sstream << std::hex << val;
	std::string str(sstream.str());
	write_value_to_file(filename, str);
}

template <typename T> static inline void write_value_to_file(const std::string &filename, T val) {
	write_value_to_file(filename, std::to_string(val).c_str());
}

template <> inline void write_value_to_file(const std::string &filename, std::string val) {
	write_value_to_file(filename, val.c_str());
}

template <> void write_value_to_file<const char *>(const std::string &filename, const char *val) {
	assert(filename != "");

	FILE *file = fopen(filename.c_str(), "w+");

	if (file == nullptr) {
		throw std::runtime_error(strerror(errno));
	}

	int status = fputs(val, file);
	if (status <= 0 || ferror(file) != 0) {
		// try to close the file, but return the old error
		auto err = errno;
		fclose(file);

		throw std::runtime_error(strerror(err));
	}

	if (fclose(file) != 0) {
		// removed error handling for fclose for now, as resfs always reports an error
		// for writes to the schema, even though the write was success
		// throw std::runtime_error(strerror(errno));
	}
}

static inline std::string read_line_from_file(const std::string &filename) {
	assert(filename != "");

	FILE *file = fopen(filename.c_str(), "r");

	if (file == nullptr) {
		throw std::runtime_error(strerror(errno));
	}

	char temp[buf_size];
	if (fgets(temp, buf_size, file) == nullptr && (feof(file) == 0)) {
		// something is wrong. let's try to close the file and go home
		fclose(file);
		throw std::runtime_error("Error while reading file in libponci. Buffer to small?");
	}

	if (feof(file) != 0) {
		memset(temp, 0, buf_size);
	}

	if (fclose(file) != 0) {
		throw std::runtime_error(strerror(errno));
	}

	return std::string(temp);
}

template <typename T> static inline std::vector<T> read_lines_from_file(const std::string &filename) {
	assert(filename != "");

	FILE *file = fopen(filename.c_str(), "r");

	if (file == nullptr) {
		throw std::runtime_error(strerror(errno));
	}

	std::vector<T> ret;

	char temp[buf_size];
	while (true) {
		if (fgets(temp, buf_size, file) == nullptr && !feof(file)) {
			// something is wrong. let's try to close the file and go home
			fclose(file);
			throw std::runtime_error("Error while reading file in libponci. Buffer to small?");
		}
		if (feof(file)) break;
		std::size_t done = 0;
		T i = string_to_T<T>(std::string(temp), done);
		if (done != 0) ret.push_back(i);
	}

	if (fclose(file) != 0) {
		throw std::runtime_error(strerror(errno));
	}

	return ret;
}

template <> int string_to_T<int>(const std::string &s, std::size_t &done) { return stoi(s, &done); }

/*template <> unsigned long string_to_T<unsigned long>(const std::string &s, std::size_t &done) {
	return stoul(s, &done);
}*/

#endif /* end of include guard: fileIO_helper */
