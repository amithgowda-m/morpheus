#include "result_writer.h"

namespace morpheus {

ResultWriter::ResultWriter() {}

ResultWriter::~ResultWriter() {}

bool ResultWriter::writeToJSON(const std::string& filename,
                               const std::map<std::string, std::string>& results) {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) return false;

    ofs << "{" << std::endl;
    for (auto it = results.begin(); it != results.end(); ++it) {
        ofs << "  \"" << escapeJSON(it->first) << "\": \"" << escapeJSON(it->second) << "\"";
        auto next_it = it;
        ++next_it;
        if (next_it != results.end()) ofs << ",";
        ofs << std::endl;
    }
    ofs << "}\n";
    return true;
}

bool ResultWriter::writeSamplesToCSV(const std::string& filename,
                                     const std::vector<std::map<std::string, std::string>>& samples) {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) return false;
    if (samples.empty()) return true;

    // Determine headers from first sample
    std::vector<std::string> headers;
    for (const auto& kv : samples[0]) headers.push_back(kv.first);

    // Write header
    for (size_t i = 0; i < headers.size(); ++i) {
        if (i) ofs << ',';
        ofs << headers[i];
    }
    ofs << '\n';

    // Write rows
    for (const auto& sample : samples) {
        for (size_t i = 0; i < headers.size(); ++i) {
            if (i) ofs << ',';
            auto it = sample.find(headers[i]);
            if (it != sample.end()) ofs << it->second;
        }
        ofs << '\n';
    }
    return true;
}

bool ResultWriter::writeSummary(const std::string& filename,
                                const std::vector<std::map<std::string, std::string>>& benchmarks) {
    // Reuse JSON writer to dump an array of benchmark maps as a simple file
    std::ofstream ofs(filename);
    if (!ofs.is_open()) return false;

    ofs << "[" << std::endl;
    for (size_t i = 0; i < benchmarks.size(); ++i) {
        ofs << "  {" << std::endl;
        const auto& bm = benchmarks[i];
        size_t j = 0;
        for (const auto& kv : bm) {
            ofs << "    \"" << escapeJSON(kv.first) << "\": \"" << escapeJSON(kv.second) << "\"";
            if (++j < bm.size()) ofs << ",";
            ofs << std::endl;
        }
        ofs << "  }";
        if (i + 1 < benchmarks.size()) ofs << ",";
        ofs << std::endl;
    }
    ofs << "]\n";
    return true;
}

std::string ResultWriter::escapeJSON(const std::string& str) const {
    std::string out;
    for (char c : str) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    return out;
}

} // namespace morpheus
