// #include "../trie.h"
#include "../zmq_server.h"
#include "glad.h"
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <bitset>
#include <map>

#define CACHE_THRESHOLD_SIZE 9 // 1 BYTE CMD AND 8 BYTE HASH
#define CACHE_KEY_SIZE sizeof(size_t) * __CHAR_BIT__ + sizeof(unsigned char) * __CHAR_BIT__

typedef std::string cache_key;
typedef std::pair<std::pair<cache_key, std::size_t>, bool> cache_check;
// typedef trie::trie_map<std::string, std::string> CMD_SEQ_BUFFER;