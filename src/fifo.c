struct FIFOPixelEntry
{
    unsigned char color;
    unsigned char palette;
    unsigned char bg_priority;
};

struct FIFO
{
    struct FIFOPixelEntry pixels[16];
    int length;
};
