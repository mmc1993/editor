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
            //  TODO MMC_
            ret = std::filesystem::create_directory(name);
        }
        return ret;
    }

    //  ±éÀúÄ¿Â¼
    inline void ListPath(const std::string & dir, const std::function<void (const std::string & path)> & func)
    {
        for (std::deque<std::string> list{ dir }; !list.empty(); list.pop_front())
        {
            //  TODO MMC_
            for (auto & item : std::filesystem::directory_iterator(list.front()))
            {
                auto path = Replace(item.path().string(), "\\", "/");
                if (item.is_directory()) { list.push_back(path); }
                else { func(path); }
            }
        }
    }
}