#pragma once
#include <iostream>

class InvalidJSONFileFormat : public std::exception
{
public:

   InvalidJSONFileFormat(const std::string& details, int line) :/* runtime_error(details),*/ details(details), line(line)
   {
   }

   void LogError() const
   {
      std::cout << details << " Line: " << line << "\n";
   }

   // Override the what() function to return a message
   const char* what() const noexcept override
   {
      return details.c_str();
   }

private:
   std::string details;
   int line;
};