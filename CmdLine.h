//: CmdLine.h - header for a class that parses the command line
//              (uses classes StrQueue and StrTable)
// @author Matt Stallmann, 16 Dec 1998
// <br> $Revision: 1.3 $, $Author: mfms $, $Date: 2005/05/05 15:00:48 $

// HISTORY:
//   1 Apr 1999 - changed it so that a flag without an = sign is
//                automatically set to 0 (so its presence can be tested) 
//  29 Jun 1999 - added getFile( int ) method
//   3 Nov 2004 - updated this to use STL instead of Jo Perry's utilities.

// ASSUMPTIONS:

//   1. Any command line argument beginning with '-' (except for '-' by
//   itself) is a flag; the - is followed by a legal C identifier, an '=',
//   and the value of the flag (no spaces); the value may be interpreted as
//   a string, an integer, a double, or a Boolean (0 or 1). A '-' followed
//   by an identifier and no '=' is entered with value 0.

//   2. Anything other than a flag is interpreted as a file name.

//   3. A '-' by itself represents standard input (is reported as the file
//   name "").

#ifndef CMDLINE_H
#define CMDLINE_H

#include<string>
#include<vector>
#include<map>
#include<set>

typedef std::map< std::string, std::string, std::less< std::string > > string_table;
typedef std::vector< std::string > string_list;
typedef std::set< std::string > string_set;

class CmdLine {
public:
  CmdLine( int argc, char * * argv );
  // PRE: argc = number of command line arguments and argv = an array
  //      containing the arguments
  // POST: values of all flags have been stored and the file names,
  //       beginning with the command name, are queued up in the queue
  //       returned by getFiles() - an empty file name indicates standard
  //       input (or can be interpreted as desired by the client) 


  ~CmdLine();                   // clean up allocated memory

  bool flagsAreLegal( const string_set legal_flags ) const;
  // returns true iff all the flags match those on the legal_flags list

  unsigned numberOfFiles() const;
  // number of files on the command line (not including argv[0]!)

  string_list getFiles() const;
  // POST: retval == a copy of the queue of file names

  const std::string getFile( unsigned i );
  // PRE: i <= numberOfFiles()
  // POST: retval == the i-th file name (i = 0 means the command name); if
  //       there are no flags, this is equivalent to argv[i]

  bool flagPresent( const std::string flag ) const;
  // POST: retval == true iff flag is present in command line

  const std::string stringFlag( const std::string flag ) const;
  // POST: retval == the value of the given flag

  int intFlag( const std::string flag ) const;
  // PRE: the value of flag is a legal integer
  // POST: retval == value of flag interpreted as an integer

  double doubleFlag( const std::string flag ) const;
  // PRE: the value of flag is a legal floating point number
  // POST: retval == value of flag interpreted as an (double precision)
  //                 floating point number

  bool boolFlag( const std::string flag ) const;
  // PRE: the value of flag is 0 or 1
  // POST: retval == value of flag interpreted as a Boolean

private:
  string_list my_files;
  string_table my_flags;
};

#endif

//  [Last modified: 2019 05 02 at 16:47:13 GMT]
