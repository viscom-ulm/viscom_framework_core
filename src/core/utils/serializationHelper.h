/**
* @file   serializationHelper.h
* @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
* @date   2016.02.17
*
* @brief  Definition of helper functions for serializing objects.
*/

#pragma once

#include <istream>
#include <ostream>
#include <vector>
#ifndef __APPLE_CC__
#include <experimental/filesystem>
#endif

namespace viscom::serializeHelper {

    inline unsigned int tag(char a, char b, char c, char d) {
        return (static_cast<unsigned int>(a) << 24 | static_cast<unsigned int>(b) << 16 | static_cast<unsigned int>(c) << 8 | static_cast<unsigned int>(d));
    }

    template<class T> void write(std::ostream& ofs, const T& value) { ofs.write(reinterpret_cast<const char*>(&value), sizeof(T)); }
    template<> inline void write<std::string>(std::ostream& ofs, const std::string& value) { write(ofs, static_cast<uint64_t>(value.size())); ofs.write(value.data(), value.size()); }

    template<class T> void writeV(std::ostream& ofs, const std::vector<T>& value)
    {
        write(ofs, static_cast<uint64_t>(value.size()));
        ofs.write(reinterpret_cast<const char*>(value.data()), value.size() * sizeof(T));
    }
    template<> inline void writeV<std::string>(std::ostream& ofs, const std::vector<std::string>& value) { write(ofs, static_cast<uint64_t>(value.size())); for (const auto& str : value) write(ofs, str); }

    template<class T> void writeVV(std::ostream& ofs, const std::vector<std::vector<T>>& value) { write(ofs, static_cast<uint64_t>(value.size())); for (const auto& str : value) writeV(ofs, str); }


    template<class T> void read(std::istream& ifs, T& value) { ifs.read(reinterpret_cast<char*>(&value), sizeof(T)); }
    template<> inline void read<std::string>(std::istream& ifs, std::string& value) {
        uint64_t strLength; ifs.read(reinterpret_cast<char*>(&strLength), sizeof(strLength));
        value.resize(strLength); if (strLength != 0) ifs.read(const_cast<char*>(value.c_str()), strLength);
    }

    template<class T> void readV(std::istream& ifs, std::vector<T>& value) {
        uint64_t vecLength; ifs.read(reinterpret_cast<char*>(&vecLength), sizeof(vecLength));
        value.resize(vecLength); if (vecLength != 0) ifs.read(reinterpret_cast<char*>(value.data()), vecLength * sizeof(T));
    }
    template<> inline void readV<std::string>(std::istream& ifs, std::vector<std::string>& value) {
        uint64_t vecLength; ifs.read(reinterpret_cast<char*>(&vecLength), sizeof(vecLength));
        value.resize(vecLength); for (auto& str : value) read(ifs, str);
    }

    template<class T> void readVV(std::istream& ifs, std::vector<std::vector<T>>& value) {
        uint64_t vecLength; ifs.read(reinterpret_cast<char*>(&vecLength), sizeof(vecLength));
        value.resize(vecLength); for (auto& str : value) readV(ifs, str);
    }

    /**
     *  Helper class to inherit from to make your class easier serializable.
     *  @tparam T0 first part of the tag identifying the class.
     *  @tparam T1 second part of the tag identifying the class.
     *  @tparam T2 third part of the tag identifying the class.
     *  @tparam T3 fourth part of the tag identifying the class.
     *  @tparam V version of the class.
     */
    template<char T0, char T1, char T2, char T3, unsigned int V> struct VersionableSerializer
    {
    private:
        using VersionableSerializerType = VersionableSerializer<T0, T1, T2, T3, V>;
        static const unsigned int VERSION = V;

    public:
#ifndef __APPLE_CC__
        /**
         *  Checks if the original file is older than the binary file.
         *  @param filename the name of the original file.
         *  @param binFilename the name of the binary file.
         *  @return true if the original file is older.
         */
        static bool checkFileDate(const std::string& filename, const std::string& binFilename)
        {
            auto fileLastWrite = std::experimental::filesystem::last_write_time(filename);
            auto binLastWrite = std::experimental::filesystem::last_write_time(binFilename);
            return (binLastWrite > fileLastWrite);
        }
#endif

        /**
         *  Checks if the file header of a serialized file matches in tag and version.
         *  @param ifs the stream to check the file header.
         *  @return a tuple containing whether tag and version of the file match and the actual file version.
         */
        static std::tuple<bool, unsigned int> checkHeader(std::istream& ifs)
        {
            const auto TAG = tag(T0, T1, T2, T3);
            unsigned int ftag, fversion;
            read(ifs, ftag);
            read(ifs, fversion);
            if (ftag == TAG) return std::make_tuple(fversion == V, fversion);
            else return std::make_tuple(false, 0);
        }

        /**
         *  Writes the file header with tag and version. 
         *  @param ofs the stream to write the header to.
         */
        static void writeHeader(std::ostream& ofs)
        {
            const auto TAG = tag(T0, T1, T2, T3);
            write(ofs, TAG);
            write(ofs, V);
        }
    };
}
