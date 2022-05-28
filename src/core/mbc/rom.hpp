#include "core/cart.hpp"

namespace gbemu::core
{

class Rom : public Mbc
{
public:
    Rom(std::vector<u8>& rom);

    virtual void map(Memory* mem) override;
};

}
