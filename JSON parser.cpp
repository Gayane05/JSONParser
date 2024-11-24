// JSON parser.cpp

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>
#include <variant>
#include <unordered_map>
#include "InvalidJSONFileFormat.h"

using namespace std::filesystem;

enum class ExpectedTokenType
{
   key,
   value,
   quotation,
   colon,
   comma,
   bracket,
   bracketorComma
};

using primitiveValue_type = std::variant<long long, std::string, bool, nullptr_t, float, double>;
using arraysType = std::variant < std::vector<std::vector<primitiveValue_type>>, std::vector<primitiveValue_type>>;

using jsonValueOrArrayType = std::variant <primitiveValue_type, arraysType >;

using jsonObjectType = std::variant< jsonValueOrArrayType, long long, std::string, bool, nullptr_t, float, double,
   std::vector<std::vector<primitiveValue_type>>, std::vector<primitiveValue_type>>;

using nestedJsonObjectType = std::unordered_map<std::string, jsonObjectType>;

//template <typename T>
struct ElementPrinterVisitor
{
   // Primitive Types
   void operator ()(long long num)
   {
      std::cout << num << " type is long long.";
   }
   void operator ()(const std::string& val)
   {
      std::cout << val << " type is string.";
   }
   void operator ()(nullptr_t /*nu*/)
   {
      std::cout << "null" << " type is null.";
   }
   void operator ()(bool val)
   {
      std::cout << std::boolalpha<< val << " type is boolean.";
   }
   void operator ()(float num)
   {
      std::cout << num << " type is float.";
   }
   void operator ()(double num)
   {
      std::cout << num << " type is double.";
   }

   // Array(Vector) types
   void operator ()(std::vector<primitiveValue_type> oneDimVector)
   {
      std::cout << "array elements are:[";

      for (const auto& el : oneDimVector)
      {
         std::visit(ElementPrinterVisitor{}, el);
         std::cout << " ";
      }
      std::cout << "]";
   }

   void operator ()(std::vector<std::vector<primitiveValue_type>> twoDimVector)
   {
      for (const auto& vector : twoDimVector)
      {
         for (const auto& el : vector)
         {
            std::visit(ElementPrinterVisitor{}, el);
         }
      }
   }

   void operator ()(jsonValueOrArrayType valueOrArray)
   {
      if (const auto* pval = std::get_if<primitiveValue_type>(&valueOrArray))
      {
         primitiveValue_type value = std::get<0>(valueOrArray); // getting value
         std::visit(ElementPrinterVisitor{}, value);
      }
      else
      {
         auto array = std::get<1>(valueOrArray); // getting array
         std::visit(ElementPrinterVisitor{}, array);
      }

   }

   void operator () (arraysType arrays)
   {
      if (const auto* pval = std::get_if<std::vector<primitiveValue_type>>(&arrays))
      {
         auto& vec = std::get<1>(arrays);
         for (const auto& el : vec)
         {
            std::visit(ElementPrinterVisitor{}, el);
         }
      }
      else
      {
         auto& twoDimVector = std::get<0>(arrays);

         for (const auto& vector : twoDimVector)
         {
            for (const auto& el : vector)
            {
               std::visit(ElementPrinterVisitor{}, el);
            }
         }
      }
   }

   void operator () (nestedJsonObjectType nestedObj)
   {
      std::cout << " Nested object { ";
      for (auto el : nestedObj)
      {
         std::cout << "Key:" << el.first << " " << "Value:";

         std::visit(ElementPrinterVisitor{}, el.second);
      }
      std::cout << "}";
   }
};

static std::vector<std::vector<primitiveValue_type>> baseArray;

std::unordered_map<std::string, arraysType > arrayElements;

ExpectedTokenType nextExpectedTokenType;

std::unordered_map<
   std::string,
   std::variant<
      jsonValueOrArrayType,
      long long,
      std::string,
      bool,
      nullptr_t,
      float,
      double,
      std::vector<std::vector<primitiveValue_type>>,
      std::vector<primitiveValue_type>,
      nestedJsonObjectType>
>  jsonGlobalElements;


bool ValidateCurrentState(ExpectedTokenType expectedTokenType, ExpectedTokenType currentTokenType)
{
   return (nextExpectedTokenType != currentTokenType);
}

std::string checkAndGetStringValue(std::string& content, size_t& i, bool& isClosed)
{
   std::string value;

   while (++i < content.size())
   {
      if (content[i] == '\"')
      {
         if (i + 1 < content.size())
         {
            if (content[i + 1] == ',' || content[i + 1] == '}')
            {
               ++i;
               isClosed = true;
               break;
            }
            else
            {
               throw InvalidJSONFileFormat("Invalid string format", __LINE__);
            }
         }
         else
         {
            isClosed = true;
            ++i;
            break;
         }
      }

      if (content[i] == ',' && !isClosed)
      {
         throw InvalidJSONFileFormat("Invalid string format", __LINE__);
      }
      else
      {
         value += content[i];
      }
   }

   return value;

}

// Parse content from JSON file to string.
void ReadContentFromFile(/*const std::filesystem::path& path,*/const std::string& fileName, std::string& content)
{

   //if (!std::filesystem::exists(path))
   //{
   //   throw std::runtime_error("File does not exist: " + path.string());
   //}

   std::ifstream file(fileName);
   if (!file.is_open()) {
      throw InvalidJSONFileFormat("Cannot open file", __LINE__);
   }

   content.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
   if (content.empty() || content.front() != '{' || content.back() != '}') {
      throw InvalidJSONFileFormat("File is invalid", __LINE__);
   }
   file.close();
}

// Parse JSON file to string.
//void readContentFromFile(/*const std::filesystem::path& path,*/ std::string fileName, std::string& content)
//{
//
//   //if (!std::filesystem::exists(path))
//   //{
//   //   throw std::runtime_error("File does not exist: " + path.string());
//   //}
//
//   std::fstream file(fileName, std::fstream::in);
//   if (!file)
//   {
//      std::cerr << "Cannot open file.\n";
//      return;
//   }
//
//   while (file)
//   {
//      std::string temp;
//      file >> temp;
//      content += temp;
//
//      if (file.eof())
//      {
//         break;
//      }
//   }
//   file.close();
//
//
//   if (content.size() == 0)
//   {
//      throw InvalidJSONFileFormat("File is invalid", __LINE__);
//   }
//   else
//   {
//      if (content[0] != '{')
//      {
//         throw InvalidJSONFileFormat("File is invalid", __LINE__);
//      }
//      else if (content[content.size() - 1] != '}')
//      {
//         throw InvalidJSONFileFormat("File is invalid", __LINE__);
//      }
//      else
//      {
//         std::cout << "File is valid \n";
//      }
//   }
//}


bool parseNumber(const std::string& value, primitiveValue_type& parsedValue)
{
   try
   {
      size_t firstDotPos;

      if (firstDotPos = value.find('.'); firstDotPos == std::string::npos)
      {
         parsedValue = stoi(value);
      }
      else
      {
         if (value.find('.', firstDotPos) == std::string::npos)
         {
            parsedValue = stod(value);
         }
         else
         {
            return false;
         }
      }
      return true;
   }
   catch (...)
   {
      return false;
   }
 
}


bool parseBoolean(std::string& value, primitiveValue_type& parsedValue)
{
   if (value == "true")
   {
      parsedValue = true;
      return true;
   }
   if (value == "false")
   {
      parsedValue = false;
      return true;
   }
   return false;
}


bool parseNull(std::string& value, primitiveValue_type& parsedValue)
{
   if (value == "null")
   {
      parsedValue = nullptr;
      return true;
   }
   return false;
}

bool ParseArrayValue(std::string& content, bool& notLastELement, std::vector<primitiveValue_type>& arr);

size_t global_index;


int ParseArrayHelper(std::string& content, std::vector<primitiveValue_type>& tempArr)
{
   bool notLastElement = false;
   ++global_index;
   for (; content[global_index] != ']' && global_index < content.size();)
   {
      if (content[global_index] == ',' && notLastElement && content[global_index + 1] == ']')
      {
         throw InvalidJSONFileFormat("File is invalid", __LINE__);
      }
      else if (content[global_index] == ',')
      {
         ++global_index;
         continue;
      }

      auto value = ParseArrayValue(/*j,*/ content, notLastElement, tempArr);
   }
   if (global_index == content.size() || content[global_index - 1] == ',')
   {
      throw InvalidJSONFileFormat("File is invalid", __LINE__);
   }
}

bool ParseArrayValue(std::string& content, bool& notLastELement, std::vector<primitiveValue_type>& arr)
{
   bool isClosed = true;

   if (content[global_index] == '\"')
   {
      isClosed = false;

      std::string value = checkAndGetStringValue(content, global_index, isClosed);

      if (content[global_index] == ',')
      {
         notLastELement = true;
      }

      if (isClosed)
      {
         // save the value
         arr.push_back(value);
         return 0;
      }
      else
      {
         throw InvalidJSONFileFormat("File is invalid", __LINE__);
         return 1; // Stop any processing.
      }
      --global_index;
   }

   // Value is an array.
   else if (content[global_index] == '[')
   {

      std::vector<primitiveValue_type> internalVec;
      ParseArrayHelper(content, internalVec);

      baseArray.push_back(internalVec);

      global_index += 1;
   }
   else
   {
      std::string value;
      isClosed = false;
      size_t end = 0;
      size_t begin = global_index;
      while (global_index < content.size())
      {
         if (content[global_index] == ',' || content[global_index] == '}' || content[global_index] == ']')
         {
            isClosed = true;
            end = global_index;
            break;
         }
         ++global_index;
      }

      if (content[global_index] == ',')
      {
         notLastELement = true;
      }

      value = content.substr(begin, end - begin);

      if (isClosed)
      {
         // save the value
         primitiveValue_type parsedValue;
         // find out what type value is:
         if (parseNumber(value, parsedValue))
         {
            arr.push_back(parsedValue);
            return 0;
         }
         else if (parseBoolean(value, parsedValue))
         {
            arr.push_back(parsedValue);
            return 0;
         }
         else if (parseNull(value, parsedValue))
         {
            arr.push_back(parsedValue);
            return 0;
         }
         else
         {
            // No valid type is value.
            throw InvalidJSONFileFormat("File is invalid", __LINE__);
         }
      }
      else
      {
         throw InvalidJSONFileFormat("File is invalid", __LINE__);
      }
   }
}

// Parse the array value
bool parseArrayValue(const std::string& content, size_t& index, std::vector<primitiveValue_type>& arr)
{
   bool isClosed = false;

   while (index < content.size()) {
      if (content[index] == '\"') {
         size_t start = ++index;
         while (index < content.size() && content[index] != '\"') ++index;
         arr.push_back(std::string(content.substr(start, index - start)));
         ++index;
         return true;
      }

      if (content[index] == '[') {
         ++index;  // Start of a nested array
         std::vector<primitiveValue_type> internalVec;
         parseArrayValue(content, index, internalVec);  // Recursive call to handle nested array
     //    arr.push_back(internalVec);  // Add the nested array as an element
         continue;
      }

      if (content[index] == ']' || content[index] == ',') break;  // End of the current array

      size_t start = index;
      while (index < content.size() && content[index] != ',' && content[index] != ']' && content[index] != '}') ++index;
      std::string value = content.substr(start, index - start);
      primitiveValue_type parsedValue;
      if (parseNumber(value, parsedValue) || parseBoolean(value, parsedValue) || parseNull(value, parsedValue)) {
         arr.push_back(parsedValue);
      }
      else {
         std::cout << "File is invalid.\n";
         return false;
      }
   }
   return true;
}


template <typename T>
int ParseTheContent(std::string& content, T& elements)
{
   nextExpectedTokenType = ExpectedTokenType::quotation; // Initial state

   std::string key;
   for (std::size_t i = 1; i < content.size(); ++i)
   {

      if (content[i] == '{' || content[i] == '}')
      {
         if (ExpectedTokenType::bracket != nextExpectedTokenType && ExpectedTokenType::bracketorComma != nextExpectedTokenType)
         {
            throw InvalidJSONFileFormat("File is invalid", __LINE__ );
         }

         nextExpectedTokenType = ExpectedTokenType::quotation;
      }

      // Supposedly got to the key.
      else if (content[i] == '\"')
      {
         //Read and validate the key.
         if (nextExpectedTokenType == ExpectedTokenType::quotation)
         {
           // bool isOpen = true;
            while (++i < content.size() && content[i] != '\"')
            {
               if (content[i] != '\"')
               {
                  key += content[i];
               }
            }

            if (i >= content.size())
            {
               throw InvalidJSONFileFormat("File is invalid", __LINE__);
            }
         }
         else
         {
            throw InvalidJSONFileFormat("File is invalid", __LINE__);
         }

         nextExpectedTokenType = ExpectedTokenType::colon;

      }

      // Get to the prefix of value.
      else if (content[i] == ':')
      {

         if (nextExpectedTokenType != ExpectedTokenType::colon)
         {
            throw InvalidJSONFileFormat("File is invalid", __LINE__);
         }

         // If we see : - it means next will be value.

         if (i + 1 == content.size())
         {
            throw InvalidJSONFileFormat("File is invalid", __LINE__);
         }

         bool isClosed = true; // Will help to understand if value is string and if yes, are all quotes present.
         // 0 - neutral value,  -1 - is opened.
         ++i;

         // Value is string.
         if (content[i] == '\"')
         {
            isClosed = false;

            std::string value = checkAndGetStringValue(content, i, isClosed);
            if (isClosed)
            {
               // save the value
               elements[key] = value;
               key = "";
            }
            else
            {
               throw InvalidJSONFileFormat("File is invalid", __LINE__);
            }
            --i;
         }
         // Value is an array.
         else if (content[i] == '[')
         {
            std::vector<primitiveValue_type> arr;
            global_index = i;
            ParseArrayHelper(/*i,*/ content, arr);

            if (baseArray.size() > 0)
            {
               jsonGlobalElements[key] = baseArray;
            }
            else
            {
               jsonGlobalElements[key] = arr;
            }
            key = "";
            
         }

         // Value is an object itself.
         else if (content[i] == '{')
         {

            size_t nestedContentEnd = content.find("},", i);

            if (nestedContentEnd == std::string::npos)
            {
               throw InvalidJSONFileFormat("File is invalid", __LINE__);
            }

            std::string nestedContent = content.substr(i, nestedContentEnd - i);

            std::unordered_map<
               std::string,
               std::variant<
                  jsonValueOrArrayType,
                  long long,
                  std::string,
                  bool,
                  nullptr_t,
                  float,
                  double,
                  std::vector<std::vector<primitiveValue_type>>,
                  std::vector<primitiveValue_type>>
            >nestedObjElements;

            ParseTheContent(nestedContent, nestedObjElements);
            
            i = nestedContentEnd;
            
            jsonGlobalElements[key] = nestedObjElements;

            key = "";

         }

         else
         {
            std::string value;
            isClosed = false;
            size_t end = 0;
            size_t begin = i;
            while (i < content.size())
            {
               if (content[i] == ',' || content[i] == '}')
               {
                  isClosed = true;
                  end = i;
                  break;
               }
               ++i;
            }

            value = content.substr(begin, end - begin);


            if (isClosed)
            {
               // save the value
               primitiveValue_type parsedValue;
               // find out what type value is:
               if (parseNumber(value, parsedValue) || parseBoolean(value, parsedValue) || parseNull(value, parsedValue))
               {
                  elements[key] = parsedValue;
                  key = "";
               }
               else
               {
                  // No valid type is value.
                  throw InvalidJSONFileFormat("File is invalid", __LINE__);
               }
            }
            else
            {
               throw InvalidJSONFileFormat("File is invalid", __LINE__);
            }

            --i;
         }

         nextExpectedTokenType =  ExpectedTokenType::bracketorComma;
      }
      else if (content[i] == ',')
      {
         nextExpectedTokenType = ExpectedTokenType::quotation;
      }
   }
   return 0;
}





int main()
{
   std::string content;

   std::cout << "Enter parsing file name: ";
   std::string fileName;
   std::cin >> fileName;

   try
   {
      ReadContentFromFile(fileName, content);

      ParseTheContent(content, jsonGlobalElements);

      for (auto [key, value] : jsonGlobalElements)
      {
         std::cout << "Key:" << key << " " << "Value:";
         std::visit(ElementPrinterVisitor{}, value);
         std::cout << std::endl;
      }
   }
   catch (InvalidJSONFileFormat& exc)
   {
      exc.LogError();
   }
  


   return 0;
}

