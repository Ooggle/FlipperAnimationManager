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
    // TODO: load Manifest file and parse it (obviously)
}
