#ifndef __BASE64_INCLUDE__

#include <string>

// Base64 编码函数
std::string base64_encode(const unsigned char *bytes_to_encode, int in_len);

// Base64 解码函数
std::string base64_decode(std::string const &encoded_string);


#define __BASE64_INCLUDE__
#endif /* __BASE64_INCLUDE__ */