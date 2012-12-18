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

#ifndef __ctkParam_h
#define __ctkParam_h

#include "ctkCmdLineApplication.hxx"

#include <algorithm>

/**
* \ingroup Command Line Module
*
* This class is designed to define parameters of your application.
* Parameters are referenced by a section/key pair and grouped by their section.
* 
* This class serves only as a proxy to the ctkParamData*classes.
* Objects of this class are intended to be temporary. Do not use new.
* Using the same section/key pair with several ctkParam<>s is fine.
*
* ctkParam<T> overloads assignment and cast to T, so the parameter
* behaves like a c++ variable of type T.
*
* Syntax example:<br>
*   ctkParam<std::string>("Name","First Name")="Bruce";<br>
*   int n=ctkParam<int>("Algorithm", "Max Iteration");<br>
*
* You an assign values via command line in your ctkCmdLineApplication<br>
*   ./myapp --name-first-name "Johnny" --algorithm-max-iterations 127<br>
* 
* 
* Supported types are:
* 
* - bool 
* - int
* - float
* - double
* - string
* - std::vector<int>
* - std::vector<double>
* - std::vector<std::string>
*
* 
* There are some special ctkParam* classes for (exotic) types. Most
* of these provide some extra functionality, such as setting possible values,
* defining lodable file extensions etc.
*
* - ctkParamEnumInt
* - ctkParamEnumFloat
* - ctkParamEnumDouble
* - ctkParamEnumString 
* - ctkParamFile
* - ctkParamDirectory
* - ctkParamImage
* - ctkParamGeometry
* - ctkParamPoint
* - ctkParamRegion
*
**/
template <typename BasicType>
class ctkParam {
protected:
 std::string section;
 std::string key;
 ctkCmdLineApplication& app;

 /// c-tor called by sub-classes: declare type will not be used, because it is not an exact type match
 ctkParam() : app(ctkApp) {}

 std::string getNormName() const
 {
  std::string name=section+"-"+key;
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  std::replace(name.begin(), name.end(),' ','-');
  return name;
 }

public:
 /// Define a parameter with this constructor. Do not use new. This class is a temporary proxy and does not store the value.
 ctkParam(const std::string& s, const std::string k)
  : app(ctkApp) , section(s) , key(k)
 {
  if (!app.getParam(section,key))
   declareType();
  app.getParam(section,key)->tags["name"]=getNormName();
 }
 
 /// Provide a (verbose) description of what this parameter is good for.
 ctkParam& setDescription(const std::string& str) {
  app.getParam(section,key)->tags["description"]=str;
  return *this;
 }

 /// Provide a label to this parameter
 ctkParam& setLabel(const std::string& str) {
  app.getParam(section,key)->tags["label"]=str;
  return *this;
 }

 ctkParam& setChannel(bool input)
 {
  app.getParam(section,key)->tags["channel"]=( input ? "input" : "output" );
  return *this;
 }

 /// Declare a flag for this parameter (shortflag should be single character string or empty)
 ctkParam& declare(const std::string& description, const std::string& shortflag="")
 {
  std::string name=getNormName();
  app.getParam(section,key)->tags["longflag"]=name;
  app.setFlag(std::string("--")+name,section,key);
  app.getParam(section,key)->tags["description"]=description;
  if (!shortflag.empty())
  {
   app.setFlag(std::string("-")+shortflag,section,key);
   app.getParam(section,key)->tags["flag"]=shortflag;
  }
  return *this;
 }

 // Declare Indexed Command Line Argument
 ctkParam& declare(const std::string& description, int idx)
 {
  app.getParam(section,key)->tags["flag"]=app.getParam(section,key)->tags["longflag"];
  app.getParam(section,key)->tags["index"]=ctkCLI::toString(idx);
  app.setFlag(ctkCLI::toString(idx),section,key);
  app.getParam(section,key)->tags["description"]=description;
  return *this;
 }

 virtual std::string getString() const { return app.getParam(section,key)->getString(); }
 virtual void setString(const std::string& value) { return app.getParam(section,key)->setString(value); }

 /// Force the ctkParamData to convert its type. (a ctkParam of type double can be used to get/set int data, no conversion will happen)
 void declareType() {
  app.setParam(section,key,new ctkCLI::ctkParamData<BasicType>());
  declare("",""); // makes the long flag known
 }

 /// Assignment by template type. Makes the ctkParam behave almost like a c++ variable of the template type
 inline ctkParam& operator=(const BasicType& v) { return setValue(v); }
 /// Cast to template type. Makes the ctkParam behave almost like a c++ variable of the template type
 inline operator BasicType() const { return getValue(); }

 /// Access to value (alternative to type-cast operator)
 BasicType getValue() const {
  ctkCLI::ctkParamDataInterface* p=app.getParam(section,key);
  ctkCLI::ctkParamData<BasicType>* sameType=dynamic_cast<ctkCLI::ctkParamData<BasicType>*>(p);
  if (sameType) return sameType->value;
  else return ctkCLI::stringTo<BasicType>(p->getString());
 }

 /// Set the value (alternative to the overloaded assignment operator)
 ctkParam& setValue(const BasicType& in) {
  ctkCLI::ctkParamDataInterface* p=app.getParam(section,key);
  ctkCLI::ctkParamData<BasicType>* sameType=dynamic_cast<ctkCLI::ctkParamData<BasicType>*>(p);
  if (sameType) sameType->value=in;
  else p->setString(ctkCLI::toString(in));
  return *this;
 }

};

// Some ugly preprocessor code, which makes the definitions in this file a lot shorter.
#define CTK_PARAM_DEFINE_ATTRIB(ATTRIB,FUNCNAME) ThisType& set##FUNCNAME(const std::string& str) { app.getParam(section,key)->attribs[ATTRIB]=str; return *this;}
#define CTK_PARAM_DEFINE_TAG(ATTRIB,FUNCNAME) ThisType& set##FUNCNAME(const std::string& str) { app.getParam(section,key)->tags[ATTRIB]=str; return *this;}

#define DEFINE_TYPE_SPECIALIZATION(TYPE,BASE,TYPESTR,SPECIAL)                  \
 namespace ctkCLI {                                                         \
  class ctkParamData##TYPE : public ctkParamDataInterface {              \
  public:                                                                \
   BASE value;                                                        \
   virtual std::string getType() const { return TYPESTR; }            \
   virtual std::string getString() const { return toString(value); }  \
   virtual void        setString(const std::string& new_value) {      \
    value=stringTo<BASE>(new_value);                               \
   }                                                                  \
  };                                                                     \
 }                                                                          \
class ctkParam##TYPE : public ctkParam<BASE> {                                 \
 typedef ctkParam##TYPE ThisType;                                           \
 public:                                                                    \
  ctkParam##TYPE(const std::string& s, const std::string& k)             \
  : ctkParam<BASE>() {                                                   \
   section=s; key=k;                                                  \
   if (!app.getParam(section,key))                                    \
    declareType();                                                 \
  }                                                                      \
  ctkParam<BASE>& operator=(const BASE& v) { return setValue(v); }             \
  operator BASE() const { return getValue(); }                           \
  void declareType() {                                                   \
   app.setParam(section,key,new ctkCLI::ctkParamData##TYPE());        \
   declare("","");                                                    \
  }                                                                      \
  SPECIAL                                                                \
 };

//----------------------------------------------------------------------------
// Enumeration types
DEFINE_TYPE_SPECIALIZATION(EnumInt,int,"integer-enumeration",CTK_PARAM_DEFINE_TAG("enumeration",Enumeration) )
DEFINE_TYPE_SPECIALIZATION(EnumFloat,float,"float-enumeration",CTK_PARAM_DEFINE_TAG("enumeration",Enumeration) )
DEFINE_TYPE_SPECIALIZATION(EnumDouble,double,"double-enumeration",CTK_PARAM_DEFINE_TAG("enumeration",Enumeration) )
DEFINE_TYPE_SPECIALIZATION(EnumString,std::string,"string-enumeration",CTK_PARAM_DEFINE_TAG("enumeration",Enumeration) )

//----------------------------------------------------------------------------
// Special
DEFINE_TYPE_SPECIALIZATION(File,std::string,"file",CTK_PARAM_DEFINE_ATTRIB("fileExtensions",FileExtensions))
DEFINE_TYPE_SPECIALIZATION(Directory,std::string,"directory", )
DEFINE_TYPE_SPECIALIZATION(Image,std::string,"image",CTK_PARAM_DEFINE_ATTRIB("type",Type) CTK_PARAM_DEFINE_ATTRIB("fileExtensions",FileExtensions) )
DEFINE_TYPE_SPECIALIZATION(Geometry,std::string,"geometry",CTK_PARAM_DEFINE_ATTRIB("type",Type) CTK_PARAM_DEFINE_ATTRIB("fileExtensions",FileExtensions) )
DEFINE_TYPE_SPECIALIZATION(Point,std::vector<std::string>,"point",CTK_PARAM_DEFINE_ATTRIB("multiple",Multiple) CTK_PARAM_DEFINE_ATTRIB("coordinateSystem",CoordinateSystem) )
DEFINE_TYPE_SPECIALIZATION(Region,std::vector<std::string>,"region",CTK_PARAM_DEFINE_ATTRIB("multiple",Multiple) CTK_PARAM_DEFINE_ATTRIB("coordinateSystem",CoordinateSystem) )

//----------------------------------------------------------------------------
// For double: Slider range (should be a template specialization of ctkParam<double> really)
DEFINE_TYPE_SPECIALIZATION(Double,std::string,"double", 
 ctkParamDouble& setRange(double minv, double maxv, double step=0.01) {
   app.getParam(section,key)->constraints["minimum"]=ctkCLI::toString(minv);
   app.getParam(section,key)->constraints["maximum"]=ctkCLI::toString(maxv);
   app.getParam(section,key)->constraints["step"]=ctkCLI::toString(step);
   return *this;
  }
 );

#endif // __ctkParam_h
