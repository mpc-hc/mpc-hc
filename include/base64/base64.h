//  //
// ####   ###     ##                -= Base64 library =-                 //
// #   # #       # # Base64.h - Base64 encoder/decoder                   //
// ####  ####   #  #                                                     //
// #   # #   # ##### Encodes and decodes base64 strings                  //
// #   # #   #     # Ideas taken from work done by Bob Withers           //
// ####   ###      # R1                      2002-05-07 by Markus Ewald  //
//  //
#ifndef B64_BASE64_H
#define B64_BASE64_H

#include <string>

namespace Base64 {

  /// Encode string to base64
  inline std::string encode(const std::string &sString);
  /// Encode base64 into string
  inline std::string decode(const std::string &sString);

}; // namespace Base64

// ####################################################################### //
// # Base64::encode()                                                    # //
// ####################################################################### //
/** Encodes the specified string to base64

    @param  sString  String to encode
    @return Base64 encoded string
*/
inline std::string Base64::encode(const std::string &sString) {
   static const char sBase64Table[64] =
    { 'A','B','C','D','E','F','G','H',
        'I','J','K','L','M','N','O','P',
        'Q','R','S','T','U','V','W','X',
        'Y','Z','a','b','c','d','e','f',
        'g','h','i','j','k','l','m','n',
        'o','p','q','r','s','t','u','v',
        'w','x','y','z','0','1','2','3',
        '4','5','6','7','8','9','+','/' };
  const char cFillChar = '=';
  const std::string::size_type   nLength = sString.length();
  std::string              sResult;

  // Allocate memory for the converted string
  sResult.reserve(nLength * 8 / 6 + 1);

  for(std::string::size_type nPos = 0; nPos < nLength; nPos++) {
    char cCode;
  
    // Encode the first 6 bits
    cCode = (sString[nPos] >> 2) & 0x3f;
    sResult.append(1, sBase64Table[cCode]);

    // Encode the remaining 2 bits with the next 4 bits (if present)
    cCode = (sString[nPos] << 4) & 0x3f;
    if(++nPos < nLength)
      cCode |= (sString[nPos] >> 4) & 0x0f;
    sResult.append(1, sBase64Table[cCode]);

    if(nPos < nLength) {
      cCode = (sString[nPos] << 2) & 0x3f;
      if(++nPos < nLength)
        cCode |= (sString[nPos] >> 6) & 0x03;

      sResult.append(1, sBase64Table[cCode]);
    } else {
      ++nPos;
      sResult.append(1, cFillChar);
    }

    if(nPos < nLength) {
      cCode = sString[nPos] & 0x3f;
      sResult.append(1, sBase64Table[cCode]);
    } else {
      sResult.append(1, cFillChar);
    }
  }

  return sResult;
}

// ####################################################################### //
// # Base64::decode()                                                    # //
// ####################################################################### //
/** Decodes the specified base64 string

    @param  sString  Base64 string to decode
    @return Decoded string
*/
inline std::string Base64::decode(const std::string &sString) {
  static const std::string::size_type np = std::string::npos;
  static const std::string::size_type DecodeTable[] = {
  // 0   1   2   3   4   5   6   7   8   9 
    np, np, np, np, np, np, np, np, np, np,  //   0 -   9
    np, np, np, np, np, np, np, np, np, np,  //  10 -  19
    np, np, np, np, np, np, np, np, np, np,  //  20 -  29
    np, np, np, np, np, np, np, np, np, np,  //  30 -  39
    np, np, np, 62, np, np, np, 63, 52, 53,  //  40 -  49
    54, 55, 56, 57, 58, 59, 60, 61, np, np,  //  50 -  59
    np, np, np, np, np,  0,  1,  2,  3,  4,  //  60 -  69
     5,  6,  7,  8,  9, 10, 11, 12, 13, 14,  //  70 -  79
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  //  80 -  89
    25, np, np, np, np, np, np, 26, 27, 28,  //  90 -  99
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38,  // 100 - 109
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48,  // 110 - 119
    49, 50, 51, np, np, np, np, np, np, np,  // 120 - 129
    np, np, np, np, np, np, np, np, np, np,  // 130 - 139
    np, np, np, np, np, np, np, np, np, np,  // 140 - 149
    np, np, np, np, np, np, np, np, np, np,  // 150 - 159
    np, np, np, np, np, np, np, np, np, np,  // 160 - 169
    np, np, np, np, np, np, np, np, np, np,  // 170 - 179
    np, np, np, np, np, np, np, np, np, np,  // 180 - 189
    np, np, np, np, np, np, np, np, np, np,  // 190 - 199
    np, np, np, np, np, np, np, np, np, np,  // 200 - 209
    np, np, np, np, np, np, np, np, np, np,  // 210 - 219
    np, np, np, np, np, np, np, np, np, np,  // 220 - 229
    np, np, np, np, np, np, np, np, np, np,  // 230 - 239
    np, np, np, np, np, np, np, np, np, np,  // 240 - 249
    np, np, np, np, np, np                   // 250 - 256
  };
  static const char cFillChar = '=';

  std::string::size_type nLength = sString.length();
  std::string            sResult;

  sResult.reserve(nLength);

  for(std::string::size_type nPos = 0; nPos < nLength; nPos++) {
    unsigned char c, c1;

    c = (char) DecodeTable[(unsigned char)sString[nPos]];
    nPos++;
    c1 = (char) DecodeTable[(unsigned char)sString[nPos]];
    c = (c << 2) | ((c1 >> 4) & 0x3);
    sResult.append(1, c);

    if(++nPos < nLength) {
      c = sString[nPos];
      if(cFillChar == c)
        break;

      c = (char) DecodeTable[(unsigned char)sString[nPos]];
      c1 = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
      sResult.append(1, c1);
    }

    if(++nPos < nLength) {
      c1 = sString[nPos];
      if(cFillChar == c1)
        break;

      c1 = (char) DecodeTable[(unsigned char)sString[nPos]];
      c = ((c << 6) & 0xc0) | c1;
      sResult.append(1, c);
    }
  }

  return sResult;
}

#endif // B64_BASE64_H
