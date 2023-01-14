#ifndef MANIFEST_H
#define MANIFEST_H

#include <string>
#include <vector>
#include <fstream>
#include <regex>

struct Manifest_animation
{
    std::string name;
    int min_butthurt;
    int max_butthurt;
    int min_level;
    int max_level;
    int weight;
};


class Manifest
{
    public:
        std::string manifest_path;
        bool is_good = false;
        std::vector<Manifest_animation> manifest_animations;

    private:

    public:
        Manifest(std::string manifest_path);
        ~Manifest();
        void load_manifest();
};

#endif
