#include "config_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

namespace morpheus {

ConfigParser::ConfigParser() {}

bool ConfigParser::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << filename << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return loadFromString(buffer.str());
}

bool ConfigParser::loadFromString(const std::string& json_str) {
    config_map_.clear();
    parseJSON(json_str);
    return true;
}

void ConfigParser::parseJSON(const std::string& json_str) {
    // Simple JSON parser for key-value pairs
    // This is a simplified version - in production, use a proper JSON library
    
    size_t pos = 0;
    std::string key, value;
    bool in_key = false, in_value = false, in_string = false;
    char quote_char = '\0';
    
    while (pos < json_str.size()) {
        char c = json_str[pos];
        
        if (c == '"' || c == '\'') {
            if (!in_string) {
                in_string = true;
                quote_char = c;
            } else if (c == quote_char) {
                in_string = false;
            }
        } else if (!in_string) {
            if (c == ':') {
                in_key = false;
                in_value = true;
                key = trim(key);
            } else if (c == ',' || c == '}') {
                if (in_value && !key.empty()) {
                    value = trim(value);
                    config_map_[key] = value;
                    key.clear();
                    value.clear();
                }
                in_value = false;
            } else if (c == '{') {
                // Start of object, skip for now
            }
        }
        
        if (in_string) {
            if (in_key) {
                key += c;
            } else if (in_value) {
                value += c;
            }
        }
        
        pos++;
    }
    
    // Handle last key-value pair
    if (!key.empty() && !value.empty()) {
        config_map_[key] = value;
    }
}

std::string ConfigParser::getString(const std::string& key, const std::string& default_val) const {
    auto it = config_map_.find(key);
    if (it != config_map_.end()) {
        // Remove quotes if present
        std::string value = it->second;
        if (value.size() >= 2 && 
            ((value.front() == '"' && value.back() == '"') ||
             (value.front() == '\'' && value.back() == '\''))) {
            return value.substr(1, value.size() - 2);
        }
        return value;
    }
    return default_val;
}

int ConfigParser::getInt(const std::string& key, int default_val) const {
    std::string value = getString(key);
    if (value.empty()) return default_val;
    
    try {
        return std::stoi(value);
    } catch (const std::exception& e) {
        return default_val;
    }
}

double ConfigParser::getDouble(const std::string& key, double default_val) const {
    std::string value = getString(key);
    if (value.empty()) return default_val;
    
    try {
        return std::stod(value);
    } catch (const std::exception& e) {
        return default_val;
    }
}

bool ConfigParser::getBool(const std::string& key, bool default_val) const {
    std::string value = getString(key);
    if (value.empty()) return default_val;
    
    std::string lower_value = value;
    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
    
    if (lower_value == "true" || lower_value == "1" || lower_value == "yes") {
        return true;
    } else if (lower_value == "false" || lower_value == "0" || lower_value == "no") {
        return false;
    }
    
    return default_val;
}

std::vector<std::string> ConfigParser::getStringArray(const std::string& key) const {
    std::vector<std::string> result;
    // Simplified array parsing
    std::string value = getString(key);
    if (value.empty()) return result;
    
    // Remove brackets and split by commas
    if (value.front() == '[' && value.back() == ']') {
        value = value.substr(1, value.size() - 2);
    }
    
    size_t start = 0, end = 0;
    while ((end = value.find(',', start)) != std::string::npos) {
        std::string element = value.substr(start, end - start);
        result.push_back(trim(element));
        start = end + 1;
    }
    result.push_back(trim(value.substr(start)));
    
    return result;
}

std::vector<int> ConfigParser::getIntArray(const std::string& key) const {
    std::vector<int> result;
    auto string_array = getStringArray(key);
    
    for (const auto& str : string_array) {
        try {
            result.push_back(std::stoi(str));
        } catch (const std::exception& e) {
            // Skip invalid entries
        }
    }
    
    return result;
}

std::vector<double> ConfigParser::getDoubleArray(const std::string& key) const {
    std::vector<double> result;
    auto string_array = getStringArray(key);
    
    for (const auto& str : string_array) {
        try {
            result.push_back(std::stod(str));
        } catch (const std::exception& e) {
            // Skip invalid entries
        }
    }
    
    return result;
}

bool ConfigParser::hasKey(const std::string& key) const {
    return config_map_.find(key) != config_map_.end();
}

std::string ConfigParser::trim(const std::string& str) const {
    size_t start = str.find_first_not_of(" \t\n\r\"'");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r\"'");
    return str.substr(start, end - start + 1);
}

} // namespace morpheus