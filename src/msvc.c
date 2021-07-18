
int _fltused = 0x9875;

#pragma function(memset)
void *memset(void *_Dst, int _Val, size_t _Size)
{
    unsigned char Val = *(unsigned char *)&_Val;
    unsigned char *At = (unsigned char *)_Dst;
    while(_Size--)
    {
        *At++ = Val;
    }
    return(_Dst);
}
