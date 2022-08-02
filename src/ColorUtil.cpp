#include "ColorUtil.hpp"
#include <cmath>

float sRGBtoLin(float colorChannel) {
  if ( colorChannel <= 0.04045f ) {
    return colorChannel / 12.92f;
  } else {
    return std::pow((( colorChannel + 0.055f)/1.055f),2.4f);
  }
}

float YtoLstar(float Y) {
  if ( Y <= ((float)216/24389)) {       // The CIE standard states 0.008856 but 216/24389 is the intent for 0.008856451679036
    return Y * ((float)24389/27);  // The CIE standard states 903.3, but 24389/27 is the intent, making 903.296296296296296
  } else {
    return std::pow(Y,((float)1/3)) * 116 - 16;
  }
}
