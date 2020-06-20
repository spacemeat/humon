#include <humon.hpp>
#include <iostream>

using namespace std;
using namespace std::literals;

template <int NumVersionValus>
struct Version
{
    Version(std::string_view verString)
    {
    }

    Version()
    {
    }

    Version(std::initializer_list<int> args)
    {

    }

    std::array<int, NumVersionValus> nums;

    bool operator<(Version const & rhs)
    {
        return true;
    }
};


int main()
{
    if (auto trove = hu::Trove::fromFile("src/samples/materials.hu");
        trove)
    {
        auto extentsNode = trove.getNode("/assets/brick-diffuse/importData/extents"sv);
        tuple xyExtents = { extentsNode / 0 / hu::value<int>{}, 
                            extentsNode / 1 / hu::value<int>{} };
        std::cout << "Extents: (" << get<0>(xyExtents) << ", " << get<1>(xyExtents) << ")\n";
    }


    if (auto trove = hu::Trove::fromFile("src/samples/hudo.hu");
        trove)
    {
        if (auto annoValue = trove.getAnnotationWithKey("app");
            annoValue != "hudo"sv)
            { throw runtime_error("File is not a hudo file."); }

        auto versionString = trove.getAnnotationWithKey("hudo-version");
        auto version = Version<3> { versionString };
        if (version < Version<3> { 0, 1, 0 }) {  }
        else if (version < Version<3> { 0, 2, 0 }) {  }
        //else ...
    }

    return 0;
}
