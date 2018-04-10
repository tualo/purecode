#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/chrono.hpp>
#include <ctime>

#include <sys/time.h>
#include <stdio.h>
#include <cstdlib>
#include <fstream>

#include <time.h>

#include <unistd.h>

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <pthread.h>
#include <vector>
#include <string>

#include "args.hxx"

#include "Image.cpp"


#ifdef HAVE_OPENCL
#include <opencv2/core/ocl.hpp>
#endif



boost::format isotimeformat("%04d-%02d-%02d %02d:%02d:%02d");
boost::format insertfile("insert into sv_images_extended (id,createdate,server,filename) values ('%s','%s','%s','%s') on duplicate key update id=values(id), createdate=values(createdate), server=values(server), filename=values(filename) ");
boost::format insertcode("insert into sv_images_extended_codes (id,code) values ('%s','%s') on duplicate key update code=values(code) ");

std::string version="1.0.001";


double debug_start_time = (double)cv::getTickCount();
double debug_last_time = (double)cv::getTickCount();
double debug_window_offset = 0;

bool bDebugTime=false;
void debugTime(std::string str){
  if (bDebugTime){
    double time_since_start = ((double)cv::getTickCount() - debug_start_time)/cv::getTickFrequency();
    double time_since_last = ((double)cv::getTickCount() - debug_last_time)/cv::getTickFrequency();
    std::cout << str << ": " << time_since_last << "s " << "(total: " << time_since_start  << "s)" << std::endl;
  }
  debug_last_time = (double)cv::getTickCount();
}

int main( int argc, char** argv ){


  args::ArgumentParser parser("Ocrs reconize barcodes and text.", "Take care depending on speed Pixel per CM Y can vary");
  args::HelpFlag help(parser, "help", "Display this help menu", { "help"});
  args::Flag debug(parser, "debug", "Show debug messages", {'d', "debug"});
  args::Flag debugwindow(parser, "debugwindow", "Show debug window", {'w', "debugwindow"});
  args::Flag debugtime(parser, "debugtime", "Show times", {'t', "debugtime"});
  args::Flag disableopencl(parser, "disableopencl", "disable opencl", {"disableopencl"});
  args::ValueFlag<std::string> filename(parser, "filename", "The filename", {'f',"file"});


  args::ValueFlag<std::string> db_host(parser, "host", "The database server host", {'h',"host"});
  args::ValueFlag<std::string> db_name(parser, "name", "The database name", {'n',"name"});
  args::ValueFlag<std::string> db_user(parser, "user", "The database server username", {'u',"user"});
  args::ValueFlag<std::string> db_pass(parser, "password", "The database server password", {'x',"password"});
  args::ValueFlag<std::string> db_encoding(parser, "encoding", "The database server encoding", {'e',"encoding"});


  args::Flag savedb(parser, "savedb", "store results in db", {"savedb"});
  args::ValueFlag<std::string> argresultpath(parser, "resultpath", "The resultpath", {"result"});
  args::ValueFlag<std::string> argserverid(parser, "serverid", "The serverid", {"serverid"});
  args::Flag removeorignal(parser, "removeorignal", "remove orignal file", {"removeorignal"});


  try
  {
      parser.ParseCLI(argc, argv);
      
      
      if (db_name==0){
        std::cout << parser;
        return 0;
      }
      if (db_user==0){
        std::cout << parser;
        return 0;
      }
      
      if (filename==0){
        std::cout << parser;
        return 0;
      }

      if (argserverid==0){
        std::cout << parser;
        return 0;
      }
  }
  catch (args::Help)
  {

      std::cout << parser;
      return 0;
  }
  catch (args::ParseError e)
  {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }
  if (debug){
    std::cout << "processing image: " << args::get(filename) << std::endl;
  }


  bDebugTime = (debugtime==1);
  debugTime("Start");

  if (disableopencl==1){
    #ifdef HAVE_OPENCL
    cv::ocl::setUseOpenCL(false);
    #endif
  }



  const char* str_db_host = "localhost";
  const char* str_db_user = "root";
  const char* str_db_name = "sorter";
  const char* str_db_password = "";
  const char* str_db_encoding = "utf8";
  const char* str_server_id = "none";
  if (db_encoding){ str_db_encoding= (args::get(db_encoding)).c_str(); }
  if (db_host){ str_db_host= (args::get(db_host)).c_str(); }
  if (db_user){ str_db_user= (args::get(db_user)).c_str(); }
  if (db_name){ str_db_name= (args::get(db_name)).c_str(); }
  if (db_pass){ str_db_password= (args::get(db_pass)).c_str(); }
  if (argserverid){ str_server_id= (args::get(argserverid)).c_str(); }

  MYSQL *con = mysql_init(NULL);

  mysql_options(con, MYSQL_SET_CHARSET_NAME, "utf8");
  mysql_options(con, MYSQL_INIT_COMMAND, "SET NAMES utf8");

  if (con == NULL){
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  if (mysql_real_connect(con, str_db_host,str_db_user, str_db_password,   str_db_name, 0, NULL, 0) == NULL){
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);
  }

  Image* im=new Image();
  im->setDebug(debug);
  im->setDebugTime(bDebugTime);
  im->setDebugWindow(debugwindow==1);
  im->open((args::get(filename)).c_str());
  im->barcode();


  std::time_t t = boost::filesystem::last_write_time( args::get(filename) ) ;
  std::string isotime = "2000-01-01 01:01:01";
  std::string isodate = "2000-01-01";
  char mbstr[100];
  if (std::strftime(mbstr, sizeof(mbstr), "%F %T", std::localtime(&t))) {
      isotime = std::string(mbstr);
  }
  if (std::strftime(mbstr, sizeof(mbstr), "%F", std::localtime(&t))) {
      isodate = std::string(mbstr);
  }


  std::vector<std::string> codes = im->getCodes();
  std::string resultpath = "";


  if (argresultpath){ resultpath=args::get(argresultpath); }
  std::string id = boost::filesystem::basename(args::get(filename).c_str());
  

  
  boost::replace_all(resultpath, "#DATE", isodate);

  const char* path = resultpath.c_str();
  boost::filesystem::path dir(path);
  if( !boost::filesystem::is_directory(dir) ) 
  { 
    if(boost::filesystem::create_directory(dir))
    {
      std::cout<< "Directory Created: "<<resultpath<<std::endl;
    }
  }
  std::string fname = boost::str( boost::format("%s%s.jpg") % resultpath % id );


  std::cout << "ALL CODES: " << std::endl;
  std::string uuid=fname;


  std::string sql = "select uuid() useid ";
  if (mysql_query(con, sql.c_str())){
    std::cout << "EE " << sql << std::endl;
    fprintf(stderr, "%s\n", mysql_error(con));
  }else{
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int num_fields;

    result = mysql_use_result(con);
    num_fields = mysql_num_fields(result);
    while ((row = mysql_fetch_row(result))){
        //unsigned long *lengths;
        uuid = std::string(row[0]);
    }
    mysql_free_result(result);
  }
  


  if (savedb==1){
    std::vector<int> params;
    params.push_back(CV_IMWRITE_JPEG_QUALITY);
    params.push_back(100);
    im->save(fname,params);
  }

  if (savedb==1){
    std::string insertfile_sql = boost::str(insertfile % uuid % isotime % str_server_id % fname );
    if (mysql_query(con, insertfile_sql.c_str())){
      fprintf(stderr, "%s\n", mysql_error(con));
    }
  }
  for(std::string code : codes) {
    std::cout << "" << code << std::endl;
    if (savedb==1){
      std::string insertcode_sql = boost::str(insertcode % uuid % code );
      if (mysql_query(con, insertcode_sql.c_str())){
        fprintf(stderr, "%s\n", mysql_error(con));
      }
    }
  }


  
  

  if (removeorignal==1){
    std::string oname=args::get(filename);
    if ( remove( oname.c_str() ) != 0 ) {
      perror( "Error deleting file" );
      exit(1);
    }
  }
  
  mysql_close(con);
  debugTime("Stop");

  return -1;
}
