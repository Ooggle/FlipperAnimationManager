#include "Manifest.hpp"

Manifest::Manifest(std::string manifest_path)
{
    this->manifest_path = manifest_path;
    this->load_manifest();
}

Manifest::~Manifest()
{

}

void Manifest::load_manifest()
{
    std::string line;
    std::ifstream manifest_file;
    manifest_file.open(this->manifest_path.c_str());

    if(!manifest_file.is_open()) {
        perror("Error: open manifest.txt");
        return;
    }
    this->good_path = true;
    
    // check filetype and version
    if(!getline(manifest_file, line)) {manifest_file.close(); return;}
    if(line.compare("Filetype: Flipper Animation Manifest") < 0) {manifest_file.close(); return;}
    if(!getline(manifest_file, line)) {manifest_file.close(); return;}
    if(line.compare("Version: 1") < 0) {manifest_file.close(); return;}
    if(!getline(manifest_file, line)) {manifest_file.close(); return;}
    if(line != std::string("")) {manifest_file.close(); return;}

    bool error = false;
    bool one_more_time  = true;
    if(!getline(manifest_file, line)) {manifest_file.close(); return;}
    // check each animations one by one
    while(one_more_time)
    {
        Manifest_animation man_anim;
        // Name
        if(line.compare("Name: ") < 0) {error = true; break;}
        man_anim.name = line.substr(6, line.length() - 6);
        // Min butthurt
        if(!getline(manifest_file, line)) {error = true; break;}
        if(line.compare("Min butthurt: ") < 0) {error = true; break;}
        man_anim.min_butthurt = std::stoi(line.substr(14, line.length() - 14));
        // Max butthurt
        if(!getline(manifest_file, line)) {error = true; break;}
        if(line.compare("Max butthurt: ") < 0) {error = true; break;}
        man_anim.max_butthurt = std::stoi(line.substr(14, line.length() - 14));
        // Min level
        if(!getline(manifest_file, line)) {error = true; break;}
        if(line.compare("Min level: ") < 0) {error = true; break;}
        man_anim.min_level = std::stoi(line.substr(11, line.length() - 11));
        // Max level
        if(!getline(manifest_file, line)) {error = true; break;}
        if(line.compare("Max level: ") < 0) {error = true; break;}
        man_anim.max_level = std::stoi(line.substr(11, line.length() - 11));
        // Weight
        if(!getline(manifest_file, line)) {error = true; break;}
        if(line.compare("Weight: ") < 0) {error = true; break;}
        man_anim.weight = std::stoi(line.substr(8, line.length() - 8));

        this->manifest_animations.push_back(man_anim);

        if(!getline(manifest_file, line)) break;
        while(1)
        {
            if(!getline(manifest_file, line)) {one_more_time = false; break;};
            if(line != std::string("")) break;
        }

    }

    manifest_file.close();

    if(error)
        this->is_good = false;
    else
        this->is_good = true;
}

bool Manifest::update_manifest(std::string file_content)
{
    if(this->good_path == false)
        return false;

    std::ofstream manifest_file;
    manifest_file.open(this->manifest_path.c_str());

    if(!manifest_file.is_open()) {
        perror("Error: open manifest.txt");
        return false;
    }

    manifest_file.write(file_content.c_str(), file_content.size());

    manifest_file.close();
    return true;
}
