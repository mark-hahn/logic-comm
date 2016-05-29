
#include <regex.h>

#define maxRxMatches 10
regmatch_t rxMatches[maxRxMatches];

const char* rxStrings[] = {
  "^\\s*//",
  "^\\s*$",
  "TRST\\s"
};

#define numRx ((int)(sizeof(rxStrings)/sizeof(char*)))

regex_t regexes[numRx];
