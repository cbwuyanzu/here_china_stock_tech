#define bswap16(x) (((x & 0xff00) >> 8 )| ((x & 0x00ff) << 8))
#define bswap32(x) (((x & 0xff000000) >> 24 )| ((x & 0x000000ff) << 24) | \
                    ((x & 0x00ff0000) >>  8 )| ((x & 0x0000ff00) <<  8))
#define bswap64(x) (((x & 0xff00000000000000) >> 56 )| ((x & 0x00000000000000ff) << 56) | \
                    ((x & 0x00ff000000000000) >> 40 )| ((x & 0x000000000000ff00) << 40) | \
                    ((x & 0x0000ff0000000000) >> 24 )| ((x & 0x0000000000ff0000) << 32) | \
                    ((x & 0x000000ff00000000) >>  8 )| ((x & 0x00000000ff000000) <<  8)) 

inline uint16_t htnu16(uint16_t x){
  uint16_t res = bswap16(x);
  return res;
}

inline uint32_t htnu32(uint32_t x){
  uint32_t res = bswap32(x);
  return res;
}

inline uint64_t htnu64(uint64_t x){
  uint64_t res = bswap64(x);
  return res;
}

inline int16_t htn16(int16_t x){
  int16_t res = bswap16(x);
  return res;
}

inline int32_t htn32(int32_t x){
  int32_t res = bswap32(x);
  return res;
}

inline int64_t htn64(int64_t x){
  int64_t res = bswap64(x);
  return res;
}
