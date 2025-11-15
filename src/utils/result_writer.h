#ifndef MORPHEUS_RESULT_WRITER_H
#define MORPHEUS_RESULT_WRITER_H

#include <string>
#include <vector>
#include <map>
#include <fstream>

namespace morpheus {

class ResultWriter {
public:
    ResultWriter();
    ~ResultWriter();
    
    // Write results to JSON file
    bool writeToJSON(const std::string& filename, 
                    const std::map<std::string, std::string>& results);
    
    // Write performance samples to CSV
    bool writeSamplesToCSV(const std::string& filename,
                          const std::vector<std::map<std::string, std::string>>& samples);
    
    // Write benchmark summary
    bool writeSummary(const std::string& filename,
                     const std::vector<std::map<std::string, std::string>>& benchmarks);

private:
    std::string escapeJSON(const std::string& str) const;
};

} // namespace morpheus

#endif // MORPHEUS_RESULT_WRITER_H