#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>

class DynamicBitset {
private:
    std::vector<uint64_t> bits;
    size_t bitSize;
public:
    DynamicBitset(size_t n) : bitSize(n) {
        bits.resize((n + 63) / 64, 0ULL);
    }
    
    void reset() {
        std::fill(bits.begin(), bits.end(), 0ULL);
    }
    
    void set(size_t pos) {
        assert(pos < bitSize);
        bits[pos / 64] |= (1ULL << (pos % 64));
    }
    
    bool test(size_t pos) const {
        assert(pos < bitSize);
        return (bits[pos / 64] & (1ULL << (pos % 64))) != 0;
    }

    DynamicBitset& operator|=(const DynamicBitset &other) {
        assert(bits.size() == other.bits.size());
        for (size_t i = 0; i < bits.size(); i++) {
            bits[i] |= other.bits[i];
        }
        return *this;
    }

    DynamicBitset& operator&=(const DynamicBitset &other) {
        assert(bits.size() == other.bits.size());
        for (size_t i = 0; i < bits.size(); i++) {
            bits[i] &= other.bits[i];
        }
        return *this;
    }

    DynamicBitset operator~() const {
        DynamicBitset res(*this);
        for (auto &w : res.bits) {
            w = ~w;
        }
        size_t extra = bits.size() * 64 - bitSize;
        if (extra > 0) {
            res.bits.back() &= ~((~0ULL) << (64 - extra));
        }
        return res;
    }
    bool none() const {
        for (auto w : bits) {
            if (w != 0ULL)
                return false;
        }
        return true;
    }
    size_t size() const {
        return bitSize;
    }
};
