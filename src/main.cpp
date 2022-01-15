#include "libchunkedhashtable.h" 
#include <iostream>
#include <string>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>
#include <fstream>

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
}

int main(int argc, char *argv[]){
    if(argc<3) {
        std::cout << "Usage: " << std::filesystem::path(argv[0]).filename().string() << " [make|check|fix] FILE HASHFILE [URL]\n";
        return EXIT_FAILURE;
    }
    std::string command(argv[1]);
	if(command == "make"){
		std::string path(argv[2]);
		std::cout << "Path: " << path << '\n';
		ChunkedHashTable HT(path, 4096);
		HT.save(argv[3]);
	}else if(command == "check"){
		std::string path(argv[2]);
		ChunkedHashTable HT(path, 4096);
		auto res = HT.check(argv[3]);
        std::cout << "Different Chunks:\n";
        if(!res.empty())
            std::cout << "None!\n";
        else
		    for(auto item: res)
			    std::cout << item << '\n';
    }else if(command == "fix"){
        std::string path(argv[2]);
        std::string hashfile(argv[3]);
        std::string url(argv[4]);

        ChunkedHashTable HT(path, 4096);
        auto res = HT.check(argv[3]);
        if(!res.empty())
            std::cout << "No corruption!\n";
        else {
            std::cout << "Fixing ...\n";

            try {
                curlpp::Cleanup cleaner;
                curlpp::Easy request;

                // Setting the URL to retrive.
                request.setOpt(new curlpp::options::Url(url));
                request.setOpt(new curlpp::options::FollowLocation(true));
                request.setOpt(new curlpp::options::NoBody (true));
                request.setOpt(new curlpp::options::Header (true));
                request.setOpt(new curlpp::options::WriteFunction ([] (char* buffer, size_t size, size_t items) -> size_t{return size * items;}));
                bool range_support{false};
                request.setOpt(new curlpp::options::HeaderFunction(
                        [&range_support] (char* buffer, size_t size, size_t items) -> size_t {
                            std::string s(buffer, size * items); // buffer is not null terminated
                            std::transform(s.begin(), s.end(), s.begin(),[](unsigned char c){ return std::tolower(c); });
                            if(s.rfind("accept-ranges", 0) == 0){
                                std::string accept_range{trim_copy(s.substr(s.find(':')+1))};
                                if(accept_range == "bytes")
                                    range_support = true;
                                std::cout << accept_range << '\n';
                            }else if(s.rfind("content-length", 0) == 0) {
                                std::cout << s ;
                            }
                            return size * items;
                        }));
                request.perform();
                if(range_support){
                    // TODO: Convert to multipart partial request
                    for (auto item: res) {
                        unsigned int start{item * 4096u}, end{(item + 1) * 4096u - 1};

                        std::list<std::string> headers;
                        std::stringstream ss;
                        ss << "Range: bytes=" << start << '-' << end;
                        headers.emplace_back(ss.str());
                        headers.emplace_back("user-agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:96.0) Gecko/20100101 Firefox/96.0");
                        request.reset();
                        request.setOpt(new curlpp::options::NoBody(false));
                        request.setOpt(new curlpp::options::Header(false));
                        request.setOpt(new curlpp::options::Url(url));
                        request.setOpt(new curlpp::options::FollowLocation(true));
                        request.setOpt(new curlpp::options::NoBody (true));
                        request.setOpt(new curlpp::options::Header (true));
                        std::ofstream out(path, std::ios::out | std::ios::binary);
                        out.seekp(start);
                        request.setOpt(new curlpp::options::WriteFunction(
                                [&out](char *buffer, size_t size, size_t items) -> size_t {
                                    out.write(buffer, size * items);
                                    return size * items;
                                }));
                        request.setOpt(new curlpp::options::HttpHeader(headers));
                        request.perform();
                    }
                }else{
                    std::cerr << "This server doesnt support partial request!\n";
                }
                return EXIT_SUCCESS;
            }
            catch ( curlpp::LogicError & e ) {
                std::cout << e.what() << std::endl;
            }
            catch ( curlpp::RuntimeError & e ) {
                std::cout << e.what() << std::endl;
            }
        }
    }else{
        std::cerr << "Bad usage\n";
    }
	return 0;
}
