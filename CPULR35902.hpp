#pragma once

#include <cstdint>

class Bus;

enum Flag {
    Z = 0,
    N,
    H,
    C
};

class CPULR35902 {
public:
    CPULR35902(Bus* bus);
    ~CPULR35902();
    void reset(); 

    void fetchDecodeExecute();

    private:
    
    void clearFlags();
    void setFlag(Flag flag);
    bool getFlag(Flag flag);
    
    void move();

    union Register {
        uint16_t w;
	struct {
#if defined(BIG_ENDIAN)
	    uint8_t msb; 
	    uint8_t lsb;
#else
            uint8_t lsb;
	    uint8_t msb;
#endif
	};
    };
    Register AF;
    Register BC;
    Register DE;
    Register HL;
    Register SP;
    Register PC;

    Bus* bus;
};

