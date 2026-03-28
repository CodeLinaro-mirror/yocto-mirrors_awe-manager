#pragma once
#include <unordered_set>
#include <string>
#include <iostream>

class CmdArgs {
public:
    // Add a flag (e.g., "-help", "-file")
    void add(const std::string& flag) {
        options_.insert(flag);
    }

    // Parse command-line arguments, any key not in the known list will cause failure
    bool parse(int argc, char* argv[]) {

        for (int i = 1; i < argc; ++i) {
            if (argv[i][0] == '-') { // starts with '-'
                if (! has(argv[i])) {
                    return false;
                }
            }
        }
        return true;
    }

    // Check if a flag was provided
    bool has(const std::string flag) const {
        auto it = options_.find(flag);
        return it != options_.end();
    }

    void clear() {
        options_.clear();
    }

private:
    std::unordered_set<std::string> options_;
};

