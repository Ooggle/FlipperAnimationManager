#ifndef MANIFEST_H
#define MANIFEST_H

#include <string>
#include <vector>
#include <fstream>
#include <regex>


class Manifest
{
    public:
        std::string manifest_path;
        bool is_good = false;

    private:

    public:
        Manifest(std::string manifest_path);
        ~Manifest();
        void load_manifest();
};

#endif
