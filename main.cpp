#include "ctkParam.hxx"

CTK_INSTANTIATE_CMD_LINE_APP("The Big Test", "Does absolutely nothing.");

int main(int argc, char ** argv)
{
 ctkApp.setCategory("Toys");
 ctkApp.setVersion("1.0");
 ctkApp.setContributor("Santa");


 //// Basic Types (this one might still be different from the slicer syntax)
 ctkParam<bool>("Basic Types","Bool Param").declare("Just a test","b")=true;
 ///// Enum Types
 ctkParamEnumDouble("EnumTypes","Double Enum").setEnumeration("0.1,0.2,0.3,0.4");
 ctkParam<double>("EnumTypes","Double Enum")=0.3;
 //// Vector types
 ctkParam<std::vector<double> >("Vector Types","Double Vec").setString("1,2,3,4");

 //// Special
 ctkParamFile("Special","File").setFileExtensions("bli,bla,blbub").declare("Input File",0).setChannel(true);

 ctkParamDouble("Special","Slider").setRange(0,1);
 ctkParam<double>("Special","Slider")=0.333;

 //// A simple parser of arguments in "--section-key value" format
 ctkApp.parseCommandLine(&argc,argv);

 ctkApp.save("test.ini");
 return 0;
}
