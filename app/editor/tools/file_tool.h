#pragma once

#ifdef CreateDirectory
#undef CreateDirectory
#endif

namespace tools {
    inline size_t GetFileLength(std::ifstream & istream)
    {
        auto p = istream.tellg();
        istream.seekg(0, std::ios::end);
        auto r = istream.tellg();
        istream.seekg(p, std::ios::beg);
        return (size_t)r;
    }

    inline size_t GetFileLength(std::ofstream & ostream)
    {
        auto p = ostream.tellp();
        ostream.seekp(0, std::ios::end);
        auto r = ostream.tellp();
        ostream.seekp(p, std::ios::beg);
        return (size_t)r;
    }

    inline bool IsFileExists(const std::string & path)
    {
        std::ifstream is(path);
        bool ret = !!is;
        is.close();
        return ret;
    }

    inline bool CreateDirectory(const std::string & path)
    {
        auto split = tools::Split(path, "/");
        split.pop_back();
        auto ret = false;
        for (auto & name : split)
        {
            ret = std::filesystem::create_directory(name);
        }
        return ret;
    }
}