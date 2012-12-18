/*=============================================================================
  
  Library: CTK
  
  Copyright (c) Lehrstuhl fuer Mustererkennung,
    Universitaet Erlangen-Nuernberg
    
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
  
    http://www.apache.org/licenses/LICENSE-2.0
    
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
  
=============================================================================*/

#ifndef __ctkCmdLineApplication_h
#define __ctkCmdLineApplication_h

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <typeinfo>

#include "StringUtil.hxx"

#define CTK_INSTANTIATE_CMD_LINE_APP(TITEL, DESCRIPTION) \
 ctkCmdLineApplication ctkCmdLineApplication::mainInstance(TITEL,DESCRIPTION);

/// Utility makro to access the application in a short expression
#define ctkApp ctkCmdLineApplication::Instance()

//----------------------------------------------------------------------------

// Define a type which is the same in c and the slicer XML
#define CTK_PARAM_FOR_TYPE_STR(X) if (type==#X) param=new ctkParamData<X>;
// Same as CTK_PARAM_FOR_TYPE_STR, except that you can specify a type string explicitly (when the c++ and slicer names do not match)
#define CTK_PARAM_FOR_SPECIAL_TYPE(STR,TYPE) if (type==STR) param=new TYPE;

namespace ctkCLI {

/**
 * \ingroup Command Line Module
 *
 * This templated function associates c++ types with strings.
 * Specializations provided:
 * - boolean
 * - integer
 * - float
 * - double
 * - string
 * - integer-vector
 * - float-vector
 * - double-vector
 * - string-vector
 *
 **/
template <typename T> inline std::string getTypeName() {
 return typeid(T).name();
}

#define CTK_DEFINE_TYPE_NAME_STRING(TYPE,STR)  template<> inline std::string getTypeName<TYPE>() {return STR;}
CTK_DEFINE_TYPE_NAME_STRING(bool,"boolean");
CTK_DEFINE_TYPE_NAME_STRING(int,"integer");
CTK_DEFINE_TYPE_NAME_STRING(float,"float");
CTK_DEFINE_TYPE_NAME_STRING(double,"double");
CTK_DEFINE_TYPE_NAME_STRING(std::string,"string");
CTK_DEFINE_TYPE_NAME_STRING(std::vector<int>,"integer-vector");
CTK_DEFINE_TYPE_NAME_STRING(std::vector<float>,"float-vector");
CTK_DEFINE_TYPE_NAME_STRING(std::vector<double>,"double-vector");
CTK_DEFINE_TYPE_NAME_STRING(std::vector<std::string>,"string-vector");

 /**
  * \ingroup Command Line Module
  *
  * Interface to ctkParamData classes
  *
  * Essentially, this is a variant type.
  *
  * Each parameter type supported by ctk implements its own ctkParamData-subclass.
  * For each defined parameter of a ctkCmdLineApplication, an instance of this class
  * is held in a container, referenced by its section and key strings.
  *
  * The actual value of the parameter is stored there.
  * 
  * example: ctkParameter<double> stores its value in a
  *          ctkParameterData<double> within the ctkCmdLineApplication.
  * 
  **/
 class ctkParamDataInterface
 {
 public:
  virtual std::string getType() const = 0;
  virtual void setString(const std::string& new_value) = 0;
  virtual std::string getString() const = 0;

  /// Additional information for the XML: such as "description", "label" etc.
  std::map<std::string,std::string> tags;
  /// Additional attributes to the xml node, such as "fileExtensions" or "coordinateSystem"
  std::map<std::string,std::string> attribs;
  /// Additional constraints for this parameter. Used for double's mininum maximum and step
  std::map<std::string,std::string> constraints; 
 };

 /**
  * \ingroup Command Line Module
  *
  * Templated implementation of ctkParamDataInterface for basic c++ types.
  *
  * This class allows the use of boolean/integer/float/double and string types
  * arguments through their respective c++ types.
  *
  * Vector-typed parameters integer-vector, double-vector and string-vector are
  * also supported through std::vector<int|double> and std::vector<std::string>
  **/
 template <typename T>
 class ctkParamData : public ctkParamDataInterface
 {
 public:
  T value;
  /// Returns the type of this parameter (eg. "integer", "file" or "string-vector" etc. )
  virtual std::string getType() const { return getTypeName<T>(); }
  /// Set the value of this parameter though a string (eg. set a double parameter through string "123.456")
  virtual void setString(const std::string& new_value) { value=stringTo<T>(new_value); }
  /// Retreive the current value of any parameter as string
  virtual std::string getString() const { return toString(value); }
 };

} // namespace ctkCLI

/**
 * \ingroup Command Line Module
 *
 * This is the main class representing a command line application.
 * You can define parameters of this appilcation. Example:
 *
 * ctkParam<double> behaves just like a double. You can set and get values as follows:<br>
 * ctkParam<double>("Airplane","Speed")=2.5;<br>
 * double airplaneSpeed=ctkParam<double>("Airplane","Speed"); // now is 2.5<br>
 *
 * Use parseCommandLine(...) to assign the values provided by the caller.
 * Command line syntax is:<br>
 * ./myapp --airplane-speed 123.456<br>
 *
 * There are also special ctkParam* types such as: ctkParamFile, ctkParamDirectory,
 * ctkParamImage, ctkParamDouble (allows setting min/max and step for a slider) and others.
 *
 * You can also store and retreive the values of all parameters via ini-Files load(...) and save(...)
 **/
class ctkCmdLineApplication
{
 // a few const iterator typedefs
 typedef std::map<std::string,std::string>::const_iterator MapIteratorStrStr;
 typedef std::map<std::string,ctkCLI::ctkParamDataInterface*>::const_iterator MapIteratorStrParam;
 typedef std::map<std::string,std::map<std::string,ctkCLI::ctkParamDataInterface*> >::const_iterator MapIteratorStrStrParam;

private:
 /// Holds values of all parameters referenced by section and key
 std::map<std::string, std::map<std::string, ctkCLI::ctkParamDataInterface * > > param;
 /// Holds section/key pairs for specific command line flags. Also includes "0", "1" etc. for indexed parameters
 std::map<std::string,std::pair<std::string,std::string> > commandLineFlags;

  /**
  * This is the singleton instance of ctkCmdLineApplication.
  * The user needs to define this static variable himself through the makro CTK_INSTANTIATE_CMD_LINE_APP
  **/
 static ctkCmdLineApplication mainInstance;
 /// Constructor requires and name and description of app
 ctkCmdLineApplication(const std::string& titel, const std::string& description)
 {
  tags["description"]=description;
  tags["title"]=titel;
 }

public: 
 /// Access to the singleton instance
 static ctkCmdLineApplication& Instance() { return mainInstance; }

 /// Access a parameter by section/key pair. Returns null if the parameter is unknown.
 ctkCLI::ctkParamDataInterface* getParam(const std::string& section, const std::string& key);

 /// Add or Replace a parameter by section/key pair. Note that ownership is transferred.
 void setParam(const std::string& section, const std::string& key, ctkCLI::ctkParamDataInterface*);

 /// Associate a command line flag with a parameter through its section/key pair
 void setFlag(const std::string& flag,const std::string& section,const std::string& key);

 /// parse command line argumants and set parameters accordingly. Unhandled parameters are left in argv.
 void parseCommandLine(int *argc, char ** argv);

 /// Load values of parameters from a string in ini-file format ie. <br> [Section] <br> Key = Value <br> Key2 = Another Value
 void parse(const std::string& iniStr);

 /// Load values of parameters from an ini-File
 bool load(const std::string& iniFile);

 /// Save values of parameters to an ini-File
 void save(const std::string& iniFile) const;

 /// Returns a slicer-compatible xml description of the command line parameters of this app for use as a plugin to ctk-hosts.
 std::string getXMLDescription() const;

 /// Returns a somewhat nicely formatted man page as string
 std::string getSynopsis();

 // Makro to define properties for slicer xml tags
 #define CTK_APP_DEFINE_TAG(XMLTAG,FUNCNAME) \
  void set##FUNCNAME(const std::string& str) { tags[XMLTAG]=str; } \
  std::string get##FUNCNAME() { return tags[XMLTAG]; }

 /// Additional tags in the XML file
 std::map<std::string,std::string> tags;
 CTK_APP_DEFINE_TAG("category",Category)
 CTK_APP_DEFINE_TAG("titel",Titel)
 CTK_APP_DEFINE_TAG("description",Description)
 CTK_APP_DEFINE_TAG("version",Version)
 CTK_APP_DEFINE_TAG("documentationUrl",DocumentationUrl)
 CTK_APP_DEFINE_TAG("license",License)
 CTK_APP_DEFINE_TAG("contributor",Contributor)
 CTK_APP_DEFINE_TAG("acknowledgements",Acknowledgements)

};

//----------------------------------------------------------------------------
//--- DEFINITIONS
//----------------------------------------------------------------------------


void ctkCmdLineApplication::setFlag(const std::string& flag,const std::string& section,const std::string& key)
{
 commandLineFlags[flag]=std::pair<std::string,std::string>(section,key);
}

//----------------------------------------------------------------------------

ctkCLI::ctkParamDataInterface* ctkCmdLineApplication::getParam(const std::string& section, const std::string& key)
{
 if (param.find(section)!=param.end() && param[section].find(key) != param[section].end())
  return param[section][key];
 else
  return 0x0;
}

//----------------------------------------------------------------------------

void ctkCmdLineApplication::setParam(const std::string& section, const std::string& key, ctkCLI::ctkParamDataInterface *p)
{
 // Add parameter. If another one by the same section/key already exists, preserve value.
 std::string value;
 if (param[section].find(key) != param[section].end())
 {
  // if a different parameter exists already, we have to delete it.
  value=param[section][key]->getString();
  delete param[section][key];
 }
 param[section][key]=p;
 if (!value.empty())
  p->setString(value);
}

//----------------------------------------------------------------------------

void ctkCmdLineApplication::parseCommandLine(int *argc, char ** argv)
{
 int index=0; // current index arguments not marked by '-' and "--"
 for (int i=0;i<*argc;)
 {
  std::string cmd=argv[i];
  // --xml prints the xml description. Overrides anything else.
  if (cmd=="--xml")
  {
   std::cout << getXMLDescription();
   argv[i++]=0x0; // mark as handled
   continue;
  }
  // help text
  if (cmd=="--help" || cmd=="-h")
  {
   std::cout << getSynopsis();
   argv[i++]=0x0; // mark as handled
   continue;
  }
  // save/load an ini-file
  if (cmd=="--ctk-save-ini" || cmd=="--ctk-load-ini")
  {
   if (i==*argc-1)
   {
    std::cerr << "Expected value but found end of argument list.\n";
    std::cerr << "Ignored command line argument " << cmd << std::endl;
    break;
   }
   argv[i++]=0x0; // mark as handled
   if (cmd[6]=='s')
    save(argv[i++]);
   else
    load(argv[i++]);
   continue;
  }
  // Command Line arguments start with '-' or "--".
  if (cmd[0]=='-')
  {
   // after the flag we expect a value
   if (i==*argc-1)
   {
    std::cerr << "Expected value but found end of argument list.\n";
    std::cerr << "Ignored command line argument " << cmd << std::endl;
    break;
   }
   argv[i]=0x0; // mark as handled
  }
  else
  {
   // Since this argument does not start with '-' we assign an index.
   cmd=ctkCLI::toString(index++);
  }
  if (commandLineFlags.find(cmd)!=commandLineFlags.end())
  {
   ctkCLI::ctkParamDataInterface *p=
    param[commandLineFlags[cmd].first][commandLineFlags[cmd].second];
   if (p->getType()==ctkCLI::getTypeName<bool>())
    p->setString(ctkCLI::toString(!ctkCLI::stringTo<bool>(p->getString())));
   else
    p->setString(argv[i]);
  }
  else if (cmd[0]=='-') // warn only for unknown flags and leave additional unhandled arguments alone 
   std::cerr << "Ignored command line argument " << cmd << std::endl;
  i++; // we handled the current argument and now use the next as our data
 }
 // Finally, remove handled arguments from argv
 int handled=0;
 for (int i=0;i<*argc;i++)
 {
  if (argv[i]==0x0)
  {
   handled++;
   argv[i]=argv[i+handled];
   i--;
  }
 }
 argc-=handled;
}

//----------------------------------------------------------------------------

void ctkCmdLineApplication::parse(const std::string& ini)
{
 std::string list="Global";
 std::istringstream strstr(ini);
 for (int lineNumber=0; !strstr.eof(); lineNumber++)
 {
  std::string line;
  getline(strstr,line,'\n');
  // ignore comments and empty lines
  if (line.length()<2 || line[0]=='#')
   continue;
  if (line[0]=='[')
  {
   list=line.substr(1,line.length()-2);
   continue;
  }
  std::istringstream linestr(line);
  std::string key,value;
  getline(linestr,key,'=');
  getline(linestr,value,'\0');
  ctkCLI::trim(key);
  ctkCLI::trim(value);
  param[list][key]->setString(value);
 } // for lines
}

//----------------------------------------------------------------------------

bool ctkCmdLineApplication::load(const std::string& iniFile)
{
 // read complete contents of file into string
 std::ifstream file(iniFile.c_str());
 if (!file.is_open() || !file.good())
 {
  file.clear();
  return false;
 }
 std::string all;
 getline(file,all,'\0');
 file.close();
 // and parse it
 parse(all);
 return true;
}

//----------------------------------------------------------------------------

void ctkCmdLineApplication::save(const std::string& iniFile) const
{
 std::ofstream file(iniFile.c_str());
 for (MapIteratorStrStrParam it=param.begin();it!=param.end();++it)
  {
   file << "\n[" << it->first << "]\n\n";
   for (MapIteratorStrParam pit=it->second.begin();pit!=it->second.end();++pit)
    file << pit->first << " = " << pit->second->getString() << "\n";
   file << "\n\n";
  }
 file.close();
}

//----------------------------------------------------------------------------

std::string ctkCmdLineApplication::getXMLDescription() const
{
 // order of slicer tags in an XML file (whatever the reason that they require this ordering)
 const int num_app_tag=8;
 const char *app_tags[]={
   "category",
   "title",
   "description",
   "version",
   "documentation-url",
   "license",
   "contributor",
   "acknowledgements"
  };
 std::ostringstream xml;
 // Name (titel) and category of the app
 xml << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
 xml << "<executable>\n";
 // All other tags ("license", "documentation-url" etc.)
 for (int i=0;i<num_app_tag;i++)
  if (tags.find(app_tags[i])!=tags.end())
   xml << "  <" << app_tags[i] << ">" << tags.find(app_tags[i])->second << "</" << app_tags[i] << ">\n";
 // The "parameters"
 for (MapIteratorStrStrParam sit=param.begin();sit!=param.end();++sit)
 {
  xml << "  <parameters>\n"; // advanced="true|false" missing
  xml << "    <label>" << sit->first << "</label>\n";
  xml << "    <description>" << sit->first << " - Section" << "</description>\n";
  for (MapIteratorStrParam kit=sit->second.begin();kit!=sit->second.end();++kit)
  {
   ctkCLI::ctkParamDataInterface &p=*(kit->second);
   // define type and attributes, such as "fileExtensions" etc.
   xml << "    <" << p.getType();
   for (MapIteratorStrStr ait=p.attribs.begin();ait!=p.attribs.end();++ait)
    if (!ait->second.empty())
     xml << " " << ait->first << "=\"" << ait->second << "\"";
   xml<< ">\n";
   xml << "      <name>" << kit->first << "</name>\n";
   // Define "defult" value (note: uses the current value of the parameter)
   xml << "      <default>" << p.getString() << "</default>\n";
   // Define additional tag such as "description", "flag" etc.
   for (MapIteratorStrStr it=p.tags.begin();it!=p.tags.end();++it)
   {
    if (it->second.empty()) continue;
    // special case: "enumeration" tag has child-nodes for items
    if (it->first=="enumeration")
    {
     std::vector<std::string> enumeration=ctkCLI::stringToVector<std::string>(it->second,',');
     if (!enumeration.empty())
     {
      xml << "      <enumeration>\n";
      for (std::vector<std::string>::const_iterator
       cit=enumeration.begin();cit!=enumeration.end();++cit)
        xml << "        <element>" << *cit << "</element>\n";
      xml << "      </enumeration>\n";
     }
    }
    else
     xml << "      <" << it->first << ">" << it->second << "</" << it->first << ">\n"; 
   }
   // finally, "constraints" tag is also a special case with child-nodes
   if (!p.constraints.empty())
   {
    xml << "      <constraints>\n";
    for (MapIteratorStrStr cit=p.constraints.begin();cit!=p.constraints.end();++cit)
     xml << "        <" << cit->first << ">" << cit->second << "</" << cit->first << ">\n";
    xml << "      </constraints>\n";
   }
   xml << "    </" << p.getType() << ">\n";
  }
  xml << "  </parameters>\n";
 }
 xml << "</executable>\n";
 return xml.str();
}

//----------------------------------------------------------------------------

// local utility that formats the verbose help text for an argument
void printOptionVerbose(std::ostringstream &str, ctkCLI::ctkParamDataInterface& p)
{
 if (p.tags["flag"].empty())
 {
  if (p.tags["longflag"].empty())
   return;
  else
   str << " [--" << p.tags["longflag"] << " <" << p.getType() << ">]\n";
 }
 else
 {
  if (p.tags["longflag"].empty())
   str << " [-" << p.tags["flag"] << " <" << p.getType() << ">]\n";
  else 
   str << " [-" << p.tags["flag"] << "|--" << p.tags["longflag"] << " <" << p.getType() << ">]\n";
 }
 if (!p.tags["description"].empty())
  str << "    " << p.tags["description"] << "\n\n";
}

//----------------------------------------------------------------------------

std::string ctkCmdLineApplication::getSynopsis()
{

 std::ostringstream str;
 std::string indent="      ";
 for (int i=0;i<(int)tags["titel"].length();i++) indent.push_back(' ');
 // Print short summary of cmd line args (std-args by ctk first)
 str << "USAGE:\n\n";
 str << "   " << "./" << tags["titel"] << " [-h] [--xml]\n";
 str << indent << "[--ctk-save-ini <file>] [--ctk-load-ini <file>]\n"; // 2do
 // All other cmd line args
 std::map<std::string, ctkCLI::ctkParamDataInterface*> indexed;
 for (MapIteratorStrStrParam sit=param.begin();sit!=param.end();++sit)
  for (MapIteratorStrParam kit=sit->second.begin();kit!=sit->second.end();++kit)
  {
   ctkCLI::ctkParamDataInterface& p(*(kit->second));
   if (p.tags["flag"].empty() && p.tags["longflag"].empty())
   {
    if (p.tags["index"]!="")
     indexed[p.tags["index"]]=&p;
   }
   else
   {
    if (p.tags["flag"].empty())
     str << indent << "[--" << p.tags["longflag"] << " <" << p.getType() << ">]\n";
    else
     str << indent << "[-" << p.tags["flag"] << " <" << p.getType() << ">]\n";
   }
  }
 // finally print the index args
 for (MapIteratorStrParam it=indexed.begin();it!=indexed.end();++it)
   str << indent << "<" << it->second->getType() << ">";
 // Go through the parameters again, by section, and print a verbose description
 for (MapIteratorStrStrParam sit=param.begin();sit!=param.end();++sit)
 {
  str << "\n\n" << sit->first << ":\n\n";
  for (MapIteratorStrParam kit=sit->second.begin();kit!=sit->second.end();++kit)
    printOptionVerbose(str,*(kit->second));
 }
 // Also for teh indexed args
 for (MapIteratorStrParam it=indexed.begin();it!=indexed.end();++it)
 {
  str << "\n\n" << it->second->getType() << "(" << it->second->tags["index"] << "):\n";
  str << "    " << it->second->tags["description"] << "\n";
 }
 // Finally print the description, contributors and acknowledgements for the tool
 if (!tags["description"].empty())
  str << "\n\n" << tags["description"] << "\n\n";
 if (!tags["contributor"].empty())
  str << "\n\nAuthor: " << tags["contributor"] << "\n\n";
 if (!tags["acknowledgements"].empty())
  str << "\n\nAcknowledgements: " << tags["acknowledgements"] << "\n\n";
 return str.str();
}

#endif // __ctkCmdLineApplication_h
