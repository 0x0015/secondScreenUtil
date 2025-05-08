#include "util.hpp"
#include <fstream>
#include <filesystem>

std::string readFileIntoString(std::string_view filename){
	if(!std::filesystem::exists(filename))
		return "";

	std::ifstream file((std::string)filename);
	auto size = std::filesystem::file_size(filename);
	if(size != 0){
		std::string output;
		output.resize(size);
		file.read(output.data(), size);
		return output;
	}else{
		//some weird file (eg. proc entry) doesn't have size
		return std::string(std::istreambuf_iterator<char>{file}, {});
	}
}

std::vector<std::string_view> splitString(std::string_view s, std::string_view delimiter){
	size_t pos_start = 0, pos_end, delim_len = delimiter.length();
	std::string_view token;
	std::vector<std::string_view> res;

	while((pos_end = s.find(delimiter, pos_start)) != std::string_view::npos){
		token = s.substr (pos_start, pos_end - pos_start);
		pos_start = pos_end + delim_len;
		if(!token.empty())
			res.push_back (token);
	}

	if(!s.substr(pos_start).empty())
		res.push_back (s.substr (pos_start));
	return res;
}

