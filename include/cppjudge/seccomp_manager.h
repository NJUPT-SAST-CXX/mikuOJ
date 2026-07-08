#pragma once
#include "cppjudge/language.h"  
#include <string>
#include <vector>

namespace cppjudge::seccomp {

class Manager {
    private:
    static const std::vector<int>&get_allowlist(SeccompProfile profile);
    
    public:
    static SeccompProfile profile_for_lang(const std::string& lang);

    static bool install(SeccompProfile profile, bool is_compile);
   
    static std::string violation_to_string(int syscall_num);

    static const std::vector<int>&get_allowlist_for_testing(SeccompProfile p);
};

} 
