#ifndef __StringUtil_hxx
#define __StringUtil_hxx

#include <iostream>
#include <sstream>
#include <vector>

namespace ctkCLI {

template <typename T> inline std::string toString(const T& in)
{
  std::ostringstream strstr;
  strstr << in;
  return strstr.str();
}

template <typename T> inline T stringTo(const std::string& in)
{
  T value;
  std::istringstream strstr(in);
  strstr >> value;
  return value;
}

template <> inline std::string toString<>(const std::string& in) { return in; }
template <> inline std::string stringTo<>(const std::string& in) { return in; }
template <> inline std::string toString<>(const bool& in) { return in ? "true" : "false"; }
template <> inline bool stringTo<>(const std::string& in)
{
  if (in=="true" || in=="yes") return true;
  if (in=="false" || in=="no") return false;
  return stringTo<int>(in)>0;
}

template <typename T> inline std::string vectortoString(const std::vector<T>& in, const std::string& delim=" ")
{
  if (in.empty()) return std::string();
  std::vector<T>::const_iterator it=in.begin();
  std::string ret=ctkCLI::toString(*it);
  for (++it;it!=in.end();++it)
    ret+=delim+ctkCLI::toString(*it);
  return ret;
}

template <typename T> inline std::vector<T> stringToVector(const std::string& in, const char delim=' ')
{
  std::string item;
  std::vector<T> ret;
  std::istringstream str(in);
  for (;std::getline(str,item,delim);str&&!str.eof())
    ret.push_back(stringTo<T>(item));
  if (item.empty()) ret.pop_back();
  return ret;
}

template <> inline std::string toString<>(const std::vector<int>& in) {return vectortoString<int>(in,",");}
template <> inline std::vector<int> stringTo<>(const std::string& in) {return stringToVector<int>(in,',');}

template <> inline std::string toString<>(const std::vector<float>& in) {return vectortoString<float>(in,",");}
template <> inline std::vector<float> stringTo<>(const std::string& in) {return stringToVector<float>(in,',');}

template <> inline std::string toString<>(const std::vector<double>& in) {return vectortoString<double>(in,",");}
template <> inline std::vector<double> stringTo<>(const std::string& in) {return stringToVector<double>(in,',');}

template <> inline std::string toString<>(const std::vector<std::string>& in) {return vectortoString<std::string>(in,",");}
template <> inline std::vector<std::string> stringTo<>(const std::string& in) {return stringToVector<std::string>(in,',');}

inline void rtrim(std::string &str , const std::string& t = " \t")
{
  str.erase(str.find_last_not_of(t)+1);
}

inline void ltrim(std::string& str, const std::string& t = " \t")
{
  str.erase(0,str.find_first_not_of(t));
}

inline void trim(std::string& str, const std::string& t = " \t")
{
  ltrim(str,t);
  rtrim(str,t);
}

} // namespace ctkCLI

#endif // __StringUtil_hxx
