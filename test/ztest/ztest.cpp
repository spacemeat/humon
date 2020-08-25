#include "ztest.hpp"


void addToFilteredTests(SourceMap & filteredTests,
    std::string_view source, std::string_view group, std::string_view test)
{
    auto sit = filteredTests.find(std::string(source));
    if (sit == filteredTests.end())
    {
        auto [nit, did] = filteredTests.emplace(source, GroupMap {});
        if (did)
            { sit = nit; }
    }
    if (sit == filteredTests.end())
        { return; }

    auto git = sit->second.find(std::string(group));
    if (git == sit->second.end())
    {
        auto [nit, did] = sit->second.emplace(group, std::vector<std::string> {});
        if (did)
            { git = nit; }
    }
    if (git == sit->second.end())
        { return; }

    git->second.push_back(std::string(test));
}


bool passesFilters(
    InexOpsVector const & inexOps, 
    std::string_view source, std::string_view group, std::string_view test)
{
    bool srcPasses = true;
    bool grpPasses = true;
    bool tstPasses = true;

    for (auto [op, arg] : inexOps)
    {
        switch (op)
        {
        case InexOperation::excludeSource:
            srcPasses = source.find(arg) == std::string::npos;
            break;
        case InexOperation::excludeGroup:
            grpPasses = group.find(arg) == std::string::npos;
            break;
        case InexOperation::excludeTest:
            tstPasses = test.find(arg) == std::string::npos;
            break;
        case InexOperation::includeSource:
            srcPasses = source.find(arg) != std::string::npos;
            break;
        case InexOperation::includeGroup:
            grpPasses = group.find(arg) != std::string::npos;
            break;
        case InexOperation::includeTest:
            tstPasses = test.find(arg) != std::string::npos;
            break;
        }
    }

    return srcPasses && grpPasses && tstPasses;
}
