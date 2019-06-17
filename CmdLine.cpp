//: CmdLine.cpp - implementation of command-line parser
// @author Matt Stallmann, 16 Dec 1998
//   1 Apr 1999 - changed it so that a flag without an = sign is
//                automatically set to 0 (so its presence can be tested) 
//  29 Jun 1999 - added getFile( int ) method
//   3 Nov 2004 - modified it to use STL instead of Jo Perry's utilities

#include"CmdLine.h"
#include<stdlib.h>              // abort()
#include<iostream>
#include<sstream>
#include<assert.h>

// private data:
//  string_list my_files;
//  string_table my_flags;

static void process_flag( const std::string str, string_table & flag_table )
{
  std::string flag_name = "";
  std::string flag_value = "";
  const size_t eq_pos = str.find( "=" );
  if ( eq_pos == std::string::npos ) {
    flag_name = str;
    flag_value = "0";
  }
  else {
    flag_name = str.substr( 0, eq_pos );
    flag_value = str.substr( eq_pos + 1, str.length() - eq_pos - 1 ); 
  }
  flag_table[ flag_name ] = flag_value; // latest value of a flag counts if
                                        // there are multiple occurrences
}

CmdLine::CmdLine( int argc, char * * argv )
{
  while ( argc ) {
    if ( '-' == (*argv)[0] ) {
      if ( (*argv)[1] ) process_flag( *argv + 1, my_flags );
      else my_files.push_back( "" );
    }
    else my_files.push_back( *argv );
    --argc;
    ++argv;
  }
}

CmdLine::~CmdLine()
{
  // no action required - all dynamic data is buried in the contained objects
}

bool
CmdLine::flagsAreLegal( const string_set legal_flags ) const
{
  bool retval = true;
  // check each flag
  for( string_table::const_iterator current = my_flags.begin();
       current != my_flags.end(); ++current ) {
    if( legal_flags.find( current->first ) == legal_flags.end() ) {
      retval = false;
      std::cerr << " Unknown option: " << current->first << std::endl;
    }
  }
  return retval;
}

unsigned
CmdLine::numberOfFiles() const
{
  return (my_files.size() - 1); // exclude argv[0]
}

string_list 
CmdLine::getFiles() const
{
  return my_files;
}

const std::string
CmdLine::getFile( unsigned i )
{
  assert( i <= my_files.size() );
  return my_files[ i ];
}

bool
CmdLine::flagPresent( const std::string flag ) const
{
  if ( my_flags.find( flag ) != my_flags.end() ) {
    return true;
  }
  else {
    return false;
  }
}

const std::string 
CmdLine::stringFlag( const std::string flag ) const
{
  string_table::const_iterator position = my_flags.find( flag );
  if ( position == my_flags.end() ) {
    std::cerr << "Command-line flag: -" << flag << " was not present" << std::endl;
    abort();
  }
  return position->second;
}

int
CmdLine::intFlag( const std::string flag ) const
{
  const std::string str_val = stringFlag( flag );
  std::istringstream conversion_stream( str_val );
  int retval = 0;
  if( ! (conversion_stream >> retval) ) {
    std::cerr << "Flag value is expected to be integer, flag = -" << flag
         << ", value = " << str_val << std::endl;
    abort();
  }
  std::string left_overs = "";
  if( conversion_stream >> left_overs ) {
    std::cerr << "Garbage at end of integer flag value, flag = -" << flag
         << ", value = " << retval << ", garbage is " << left_overs <<
      std::endl;
    abort();
  }
  return retval;
}

double
CmdLine::doubleFlag( const std::string flag ) const
{
  const std::string str_val = stringFlag( flag );
  std::istringstream conversion_stream( str_val );
  double retval = 0;
  if( ! (conversion_stream >> retval) ) {
    std::cerr << "Flag value is expected to be double, flag = -" << flag
         << ", value = " << str_val << std::endl;
    abort();
  }
  std::string left_overs = "";
  if( conversion_stream >> left_overs ) {
    std::cerr << "Garbage at end of integer flag value, flag = -" << flag
         << ", value = " << retval << ", garbage is " << left_overs <<
      std::endl;
    abort();
  }
  return retval;
}

bool
CmdLine::boolFlag( const std::string flag ) const
{
  const std::string str_val = stringFlag( flag );
  bool retval = false;
  if ( str_val != "0" && str_val != "false"
       && str_val != "no" ) { // not false
    if ( str_val != "1" && str_val != "true"
         && str_val != "yes" ) { // not true
      std::cerr << "Invalid boolean flag value, flag = " << flag << ", value = "
           << str_val << std::endl;
      abort();
    }
    else retval = true;         // == "1"
  }
  return retval;
}


//  [Last modified: 2019 05 02 at 17:06:15 GMT]
