struct JsonValue {
  uint32_t unsignedIntValue = 0;
  bool boolValue = false;  //isNegative for int value
} jsonValue;

//start from "
bool isNextJsonString(const String& str, int16_t startPos = 0) {
  if (startPos < 0 || startPos >= str.length()) return false;
  //Serial.println((int)str[startPos]);
  return str[startPos] == '"';
}

//start with t or f
bool isNextJsonBool(const String& str, int16_t startPos = 0) {
  if (startPos < 0 || startPos >= str.length()) return false;
  return str[startPos] == 't' || str[startPos] == 'f';
}

//start with - or digit
bool isNextJsonInteger(const String& str, int16_t startPos = 0) {
  if (startPos < 0 || startPos >= str.length()) return false;
  return str[startPos] == '-' || isDigit(str[startPos]);
}

bool isNextJsonArray(const String& str, int16_t startPos = 0) {
  if (startPos < 0 || startPos >= str.length()) return false;
  return str[startPos] == '[';
}

bool isNextJsonNode(const String& str, int16_t startPos = 0) {
  if (startPos < 0 || startPos >= str.length()) return false;
  return str[startPos] == '{';
}

//return endPos - position of close "
uint16_t endJsonStringPos(const String& str, int16_t startPos = 0) {
  if (str[startPos] != '"') return startPos;
  bool escaped = false;
  startPos++;
  while (startPos < str.length()) {
    if (!escaped && str[startPos] == '\\') escaped = true;
    else if (!escaped && str[startPos] == '"') return startPos;
    else if (escaped) escaped = false;
    startPos++;
  }
  Serial.println("Close \" not found");
  return startPos;
}

uint16_t parseJsonString(const String& str, String& out, int16_t startPos = 0) {
  int16_t endPos = endJsonStringPos(str, startPos);
  if(startPos + 1 < endPos) out = str.substring(startPos + 1, endPos);
  return endPos+1;
}

//return endPos - position of last letter + 1
uint16_t parseJsonBool(const String& str, String& out, int16_t startPos = 0) {
  if (str[startPos] == 't' && str.substring(startPos, startPos + 4) == "true") {
    jsonValue.boolValue = true;
    return (uint16_t)(startPos + 4);
  } else if (str[startPos] == 'f' && str.substring(startPos, startPos + 5) == "false") {
    jsonValue.boolValue = false;
    return (uint16_t)(startPos + 5);
  }
  Serial.print("Can't parse bool str[");
  Serial.print(startPos);
  Serial.print("]=");
  Serial.println(str[startPos]);
  return (uint16_t)startPos;
}

//return endPos - position of last digit + 1
uint16_t parseJsonInteger(const String& str, int16_t startPos = 0) {
  if (str[startPos] == '-') {
    jsonValue.boolValue = true;
    startPos++;
  } else jsonValue.boolValue = false;
  jsonValue.unsignedIntValue = 0;
  while (startPos < str.length() && isDigit(str[startPos])) {
    jsonValue.unsignedIntValue = jsonValue.unsignedIntValue * 10 + (str[startPos] - '0');
    startPos++;
  }
  return startPos;
}

uint16_t endJsonIntegerPos(const String& str, int16_t startPos = 0) {
  if (str[startPos] == '-') 
    startPos++;
  while (startPos < str.length() && isDigit(str[startPos]))
    startPos++;
  return startPos;
}

//return endPos - position of this array end position + 1
uint16_t endJsonArrayPos(const String& str, int16_t startPos = 0) {
  bool insideString = false;
  bool escaped = false;
  if (str[startPos] != '[') return startPos;
  uint16_t counter = 1;
  startPos++;
  while (startPos < str.length()) {
    if (str[startPos] == '[' && !insideString) counter++;
    else if (str[startPos] == ']' && !insideString) counter--;
    else if (!escaped && str[startPos] == '\\') escaped = true;
    else if (!escaped && str[startPos] == '"') insideString = !insideString;
    else if (escaped) escaped = false;
    startPos++;
    if (counter == 0) return startPos;
  }
}

//return endPos - position of this node end position + 1
uint16_t endJsonNodePos(const String& str, int16_t startPos = 0) {
  bool insideString = false;
  bool escaped = false;
  if (str[startPos] != '{') return startPos;
  uint16_t counter = 1;
  startPos++;
  while (startPos < str.length()) {
    if (str[startPos] == '{' && !insideString) counter++;
    else if (str[startPos] == '}' && !insideString) counter--;
    else if (!escaped && str[startPos] == '\\') escaped = true;
    else if (!escaped && str[startPos] == '"') insideString = !insideString;
    else if (escaped) escaped = false;
    startPos++;
    if (counter == 0) return startPos;
  }
}

//return new startPos from valuable symbol
uint16_t skipSymbolsJson(const String& str, int16_t startPos = 0, char alsoSkip = ' ') {
  while (startPos < str.length() && (str[startPos] < '!' || str[startPos] == alsoSkip)) startPos++;
  return startPos;
}

//return new startPos after json value
uint16_t skipJsonValue(const String& str, int16_t startPos = 0, char alsoSkip = ' ') {
  if (isNextJsonString(str, startPos)) {
    return (uint16_t) (endJsonStringPos(str, startPos) + 1);
  } else if (isNextJsonBool(str, startPos)) {
    if (str[startPos] == 't' && str.substring(startPos, startPos + 4) == "true") {
      return (uint16_t)(startPos + 5);
    } else if (str[startPos] == 'f' && str.substring(startPos, startPos + 5) == "false") {
      return (uint16_t)(startPos + 6);
    }
    Serial.print("Can't parse bool str[");
    Serial.print(startPos);
    Serial.print("]=");
    Serial.println(str[startPos]);
    return (uint16_t)startPos;
  } else if (isNextJsonInteger(str, startPos)) {
    if (str[startPos] == '-') startPos++;
    while (startPos < str.length() && isDigit(str[startPos])) startPos++;
    return startPos + 1;
  } else if (isNextJsonArray(str, startPos)) {
    return endJsonArrayPos(str, startPos);
  } else if (isNextJsonNode(str, startPos)) {
    return endJsonNodePos(str, startPos);
  }
  Serial.print("Unknown field value: ");
  Serial.println(str.substring(startPos));
  return startPos;
}