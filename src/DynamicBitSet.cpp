#ifndef DYNAMIC_BITSET_H
#define DYNAMIC_BITSET_H

#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>

class DynamicBitset {
private:
    std::vector<uint64_t> bits;
    size_t bitSize;

public:
    // Constructor: allocate enough 64-bit words for n bits.
    inline DynamicBitset(size_t n) : bitSize(n) {
        bits.resize((n + 63) / 64, 0ULL);
    }
    
    // Reset all bits to 0.
    inline void reset() {
        std::fill(bits.begin(), bits.end(), 0ULL);
    }
    
    // Set the bit at position 'pos' to 1.
    inline void set(size_t pos) {
        assert(pos < bitSize);
        bits[pos / 64] |= (1ULL << (pos % 64));
    }
    
    // Test whether the bit at position 'pos' is set.
    inline bool test(size_t pos) const {
        assert(pos < bitSize);
        return (bits[pos / 64] & (1ULL << (pos % 64))) != 0;
    }
    
    // In-place bitwise OR with another DynamicBitset.
    inline DynamicBitset& operator|=(const DynamicBitset &other) {
        assert(bits.size() == other.bits.size());
        for (size_t i = 0; i < bits.size(); i++) {
            bits[i] |= other.bits[i];
        }
        return *this;
    }
    
    // In-place bitwise AND with another DynamicBitset.
    inline DynamicBitset& operator&=(const DynamicBitset &other) {
        assert(bits.size() == other.bits.size());
        for (size_t i = 0; i < bits.size(); i++) {
            bits[i] &= other.bits[i];
        }
        return *this;
    }
    
    // Bitwise NOT operator: returns a new DynamicBitset with all bits flipped.
    inline DynamicBitset operator~() const {
        DynamicBitset res(*this);
        for (auto &w : res.bits) {
            w = ~w;
        }
        // Clear any extra bits in the last word that exceed bitSize.
        size_t totalBits = bits.size() * 64;
        size_t extra = totalBits - bitSize;
        if (extra > 0) {
            res.bits.back() &= ((1ULL << (64 - extra)) - 1);
        }
        return res;
    }
    
    // Check if no bits are set.
    inline bool none() const {
        for (auto w : bits) {
            if (w != 0ULL)
                return false;
        }
        return true;
    }
    
    // Return the number of bits in the bitset.
    inline size_t size() const {
        return bitSize;
    }
};

#endif // DYNAMIC_BITSET_H