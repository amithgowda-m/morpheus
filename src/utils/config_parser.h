#ifndef MORPHEUS_CONFIG_PARSER_H
#define MORPHEUS_CONFIG_PARSER_H

#include <string>
#include <unordered_map>
#include <vector>

namespace morpheus {

class ConfigParser {
public:
    ConfigParser();
    ~ConfigParser() = default;
    
    // Load configuration from file
    bool loadFromFile(const std::string& filename);
    
    // Load configuration from JSON string
    bool loadFromString(const std::string& json_str);
    
    // Get values
    std::string getString(const std::string& key, const std::string& default_val = "") const;
    int getInt(const std::string& key, int default_val = 0) const;
    double getDouble(const std::string& key, double default_val = 0.0) const;
    bool getBool(const std::string& key, bool default_val = false) const;
    
    // Get arrays
    std::vector<std::string> getStringArray(const std::string& key) const;
    std::vector<int> getIntArray(const std::string& key) const;
    std::vector<double> getDoubleArray(const std::string& key) const;
    
    // Check if key exists
    bool hasKey(const std::string& key) const;

private:
    std::unordered_map<std::string, std::string> config_map_;
    
    void parseJSON(const std::string& json_str);
    std::string trim(const std::string& str) const;
};

} // namespace morpheus

#endif // MORPHEUS_CONFIG_PARSER_H