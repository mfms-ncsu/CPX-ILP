/// @file cplex_ilp.cpp reads an ILP in .mps or .lp format and solves it
///                  using the CPLEX solver (adapted from ilomipex2.cpp in
///                  the examples directory for cplex75 by Matt Stallmannn).

/// @todo add more reporting of statistics such as number of cuts

// 2004/10/20 - making changes so that it'll work with cplex90 and concert20
// 2005/05/05 - additional changes for backward compatibility with cplex75
// 2019/05/17 - added a different timer mechanism: CPLEX 12.9 internal timer
// reports much longer than actual times

// --------------------------------------------------------------------------
//  Copyright (C) 1999-2001 by ILOG.
//  All Rights Reserved.
//  Permission is expressly granted to use this example in the
//  course of developing applications that use ILOG products.
// --------------------------------------------------------------------------

#define VERSION "1.5"
#define RELEASE_DATE "2019/7/20"

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <ilcplex/ilocplex.h>
#include "CmdLine.h"
#include "ClockTimer.h"
// #include "callback_test.h"

ILOSTLBEGIN

static void usage( const char *progname );

#if 0 // couldn't figure out how to make callbacks work
class CountFractionalCuts: public FractionalCutCallbackI {
};
#endif

char * current_time() {
  const size_t TIME_BUFFER_SIZE = 512;
  time_t raw_time;
  struct tm * universal_time_info;
  static char time_message_buffer[TIME_BUFFER_SIZE];

  time(&raw_time);
  universal_time_info = gmtime(&raw_time);
  strftime(time_message_buffer, TIME_BUFFER_SIZE,
           "%a %Y/%m/%d %T GMT",
           universal_time_info);
  return time_message_buffer;
}

int
main (int argc, char **argv)
{
   IloEnv env;

   env.out() << "+++ cplex_ilp, release " << VERSION << ", " << RELEASE_DATE << " +++" << endl;
   env.out() << "\t" << env.getVersion() << endl;
   env.out() << "CurrentTime\t" << current_time() << endl;

   // process the command line
   CmdLine command_line( argc, argv );

   // correct number of file arguments?
   if( command_line.numberOfFiles() != 1 ) {
     cerr << "Should have exactly one file name on the command line." << endl;
     usage( argv[ 0 ] );
     exit( 101 );
   }

   // all flags legal?
   string_set expected_flags;
   expected_flags.insert( "lp_only" ); // indicates that this should solved
                                       // as a linear program instead of an
                                       // ILP
   expected_flags.insert( "cost" );
   expected_flags.insert( "UB" );
   expected_flags.insert( "time" );
   expected_flags.insert( "nodes" );
   expected_flags.insert( "sols" );
   expected_flags.insert( "dep" );
   expected_flags.insert( "feasible" );
   expected_flags.insert( "vsel" );
   expected_flags.insert( "nsel" );
   expected_flags.insert( "heur" );
   expected_flags.insert( "rins" );
   expected_flags.insert( "lp" );
   expected_flags.insert( "verify" );
   expected_flags.insert( "solution" );
   expected_flags.insert( "trace" );
   expected_flags.insert( "t_freq" );
   expected_flags.insert( "frac_cuts" );
   expected_flags.insert( "covers" );
   expected_flags.insert( "probing" );
   if( ! command_line.flagsAreLegal( expected_flags ) ) {
     usage( argv[ 0 ] );
     exit( 102 );
   }
   
   IloModel model(env);
   IloCplex cplex(env);
#if 0 // No callbacks for now 
   MyHeuristicCallBackI myHCI;
   cplex.use(*myHCI.createInstance());
#endif

   // send all trace output to stderr
   cplex.setOut(cerr);

   IloObjective   obj;
   IloNumVarArray var(env);
   IloRangeArray  rng(env);
   bool solve_as_lp = false; // true if only the LP solution is desired
   string input_file_string( command_line.getFile( 1 ) );
   istringstream input_file_stream( input_file_string );
   char * input_file_name = new char[ input_file_string.length() + 1 ];
   input_file_stream >> input_file_name;
   cout << "InputFile\t" << input_file_name << endl;
   
   // make sure file can be opened for input
   ifstream input_stream(input_file_name, ios::in);
   if( ! input_stream ) {
     cerr << "Unable to open file " << input_file_name
          << " for input." << endl;
     return EXIT_FAILURE;
   }

   try {
     cplex.importModel(model, input_file_name, obj, var, rng);
   }
   catch ( IloException & e ) {
     cout << "*** Error while reading file ***" << endl;
     cout << e.getMessage();
     e.end();
     return EXIT_FAILURE;
   }
   if( command_line.flagPresent( "lp_only" ) ) {
     solve_as_lp = true;
     model.add(IloConversion(env, var, ILOFLOAT));
   }

   delete [] input_file_name;
      
   // set timeout or limit number of nodes, if desired
   if( command_line.flagPresent( "time" ) ) {
     int timeout = command_line.intFlag( "time" );
     if( timeout <= 0 ) {
       cerr << "Bad timeout value "
            << command_line.stringFlag( "time" )
            << " -- should be int > 0." << endl;
       exit( 110 );
     }
     cplex.setParam( IloCplex::TiLim, timeout );
   }
   else { // default time limit is one hour
     cplex.setParam( IloCplex::TiLim, 3600 );
   }

   if( command_line.flagPresent( "nodes" ) ) {
     int nodelimit = command_line.intFlag( "nodes" );
     if( nodelimit <= 0 ) {
       cerr << "Bad node limit value "
            << command_line.stringFlag( "nodes" )
            << " -- should be int > 0." << endl;
       exit( 120 );
     }
     cplex.setParam( IloCplex::NodeLim, nodelimit );
   }

   // set stopping criterion based on objective function if desired
   // (assume a minimization problem)
   int target_cost = INT_MIN;
   if( command_line.flagPresent( "cost" ) ) {
     target_cost = command_line.intFlag( "cost" );
     model.add( obj >= target_cost );
   }

   // set an assumed initial upper bound
   int initial_upper_bound = INT_MAX;
   if( command_line.flagPresent( "UB" ) ) {
     initial_upper_bound = command_line.intFlag( "UB" );
     cplex.setParam(IloCplex::CutUp, initial_upper_bound - 1); 
     //     model.add( obj <= initial_upper_bound - 1 );
   }

   // stop after a certain number of solutions have been found
   if( command_line.flagPresent( "sols" ) ) {
     int solutionLimit = command_line.intFlag( "sols" );
     if( solutionLimit <= 0 ) {
       cerr << "Bad solution limit value "
            << command_line.stringFlag( "sols" )
            << " -- should be int > 0." << endl;
       exit( 130 );
     }
     cplex.setParam( IloCplex::IntSolLim, solutionLimit );
   }

   // set preference for dependency checking (of rows)
   if( command_line.flagPresent( "dep" ) ) {
     int dependence_indicator = command_line.intFlag( "dep" );
     if( dependence_indicator < -1 || dependence_indicator > 3 ) {
       cerr << "Bad dependence indicator "
            << command_line.stringFlag( "dep" )
            << " -- should be -1, 0, 1, 2, or 3." << endl;
       exit( 133 );
     }
     cplex.setParam( IloCplex::DepInd, dependence_indicator );
   }

   // set preference for feasibility versus optimality
   if( command_line.flagPresent( "feasible" ) ) {
     int emphasis = command_line.intFlag( "feasible" );
     if( emphasis < 0 || emphasis > 4 ) {
       cerr << "Bad feasibility indicator "
            << command_line.stringFlag( "feasible" )
            << " -- should be 0, 1, 2, 3, or 4." << endl;
       exit( 135 );
     }
     cplex.setParam( IloCplex::MIPEmphasis, emphasis );
   }

   // node and variable selection, respectively
   if( command_line.flagPresent( "nsel" ) ) {
     char selection_type = command_line.stringFlag( "nsel" )[ 0 ];
     switch( selection_type ) {
     case 'd': case 'D': // depth-first
       cplex.setParam( IloCplex::NodeSel, 0 ); break;
     case 'b': case 'B': // best bound (default)
       cplex.setParam( IloCplex::NodeSel, 1 ); break;
     case 'e': case 'E': // best estimate
       cplex.setParam( IloCplex::NodeSel, 2 ); break;
     case 'f': case 'F': // alternate best estimate
       cplex.setParam( IloCplex::NodeSel, 3 ); break;
     default:
       cerr << "Warning: Bad node selection indicator "
            << command_line.stringFlag( "nsel" ) << " -- using default."
            << endl;
     }
   }

   if( command_line.flagPresent( "vsel" ) ) {
     char selection_type = command_line.stringFlag( "vsel" )[ 0 ];
     switch( selection_type ) {
     case 'n': case 'N': // minimum infeasibility
       cplex.setParam( IloCplex::VarSel, -1 ); break;
     case 'a': case 'A': // automatic (default)
       cplex.setParam( IloCplex::VarSel, 0 ); break;
     case 'x': case 'X': // maximum infeasibility
       cplex.setParam( IloCplex::VarSel, 1 ); break;
     case 'p': case 'P': // pseudo costs
       cplex.setParam( IloCplex::VarSel, 2 ); break;
     case 's': case 'S': // "strong" branching
       cplex.setParam( IloCplex::VarSel, 3 ); break;
     case 'r': case 'R': // pseudo reduced costs
       cplex.setParam( IloCplex::VarSel, 4 ); break;
     default:
       cerr << "Warning: Bad variable selection indicator "
            << command_line.stringFlag( "vsel" ) << " -- using default."
            << endl;
     }
   }

   // heuristic frequency
   if( command_line.flagPresent( "heur" ) ) {
     // run heuristic this often
     int frequency = command_line.intFlag( "heur" );
     if( frequency < -1 ) {
       cerr << "Warning: Bad heuristic frequency "
            << command_line.stringFlag( "heur" )
            << " -- should be int >= -1 (-1 = none, 0 = automatic)." << endl;
       cerr << "Setting to automatic." << endl;
       frequency = 0;
     }
     cplex.setParam( IloCplex::HeurFreq, frequency );
   }

   // relaxation-induced neighborhood search heuristic frequency
   if( command_line.flagPresent( "rins" ) ) {
     // run heuristic this often
     int frequency = command_line.intFlag( "rins" );
     if( frequency < -1 ) {
       cerr << "Warning: Bad RINS heuristic frequency "
            << command_line.stringFlag( "rins" )
            << " -- should be int >= -1 (-1 = none, 0 = automatic)." << endl;
       cerr << "Setting to automatic." << endl;
       frequency = 0;
     }
     cplex.setParam( IloCplex::RINSHeur, frequency );
   }

   // LP algorithm used
   if( command_line.flagPresent( "lp" ) ) {
     char selection_type = command_line.stringFlag( "lp" )[ 0 ];
     switch( selection_type ) {
     case 'a': case 'A': // automatic (default)
       cplex.setParam( IloCplex::NodeAlg, 0 ); break;
     case 'p': case 'P': // primal simplex
       cplex.setParam( IloCplex::NodeAlg, 1 ); break;
     case 'd': case 'D': // dual simplex
       cplex.setParam( IloCplex::NodeAlg, 2 ); break;
     case 'b': case 'B': // barrier
       cplex.setParam( IloCplex::NodeAlg, 4 ); break;
     case 'f': case 'F': // sifting
       cplex.setParam( IloCplex::NodeAlg, 5 ); break;
       break;
     default:
       cerr << "Warning: Bad LP algorithm indicator "
            << command_line.stringFlag( "lp" ) << " -- using default."
            << endl;
     }
   }

   // pursue Gomory cuts aggressively.
   if( command_line.flagPresent( "frac_cuts" ) ) {
     cplex.setParam( IloCplex::FracCuts, 2 );
   }

   // pursue cover cuts aggressively
   if( command_line.flagPresent( "covers" ) ) {
     cplex.setParam( IloCplex::Covers, 2 );
   }

   // set level of probing, 0 is default, larger is supposed to work well
   // when there are lots of 0/1 variables
   if ( command_line.flagPresent( "probing" ) ) {
     int probe_level = command_line.intFlag( "probing" );
     if ( probe_level >= -1 && probe_level <= 3 ) {
       cplex.setParam( IloCplex::Probe, probe_level );
     }
     else {
       cerr << "Warning: Bad probe level " << probe_level << endl
            << "-1 to 3 expected -- using default value of 0."
            << endl;
     }
   }

   // Trace level
   if( command_line.flagPresent( "trace" ) ) {
     int trace_level = command_line.intFlag( "trace" );
     if( trace_level < 0 || trace_level > 5 ) {
       cerr << "Warning: Bad trace level "
            << command_line.stringFlag( "trace" ) << endl
            << " 0-5 expected -- using our default value of 1."
            << endl;
       trace_level = 0;
     }
     cplex.setParam( IloCplex::MIPDisplay, trace_level );
   }
   else {
     // make 0 the default trace level
     cplex.setParam( IloCplex::MIPDisplay, 0 );
   }

   // Trace frequency (only relevant for level 2 and higher)
   if( command_line.flagPresent( "t_freq" ) ) {
     int frequency = command_line.intFlag( "t_freq" );
     cplex.setParam( IloCplex::MIPInterval, frequency );
   }

   /// !!!
   /// The parameters below don't necessarily have the desired effect;
   /// even if both are set, CPLEX may report 'StatusCode Optimal' and an
   /// objective that is far from optimal. The safer bet is to leave well
   /// enough alone and let CPLEX report 'StatusCode OptimalTol' with
   /// 'ProvedOptimal' false. In any case, it's important to check the value
   /// reported against the optimum, if known, or the best known value. 
   /// !!!

   // cplex.setParam(IloCplex::Param::Emphasis::Numerical, true);
   // cplex.setParam(IloCplex::Param::MIP::Tolerances::MIPGap, 0);
   
   cout << "Parameter values for current run --" << endl;
   cout << "Timeout\t" << cplex.getParam( IloCplex::TiLim ) << endl;
   cout << "Node_limit\t" << cplex.getParam( IloCplex::NodeLim )
             << endl;
   cout << "Target_cost\t" << target_cost << endl;
   cout << "Initial_UB\t" << initial_upper_bound << endl;
   cout << "Dependence indicator:\t"
             << cplex.getParam( IloCplex::DepInd ) << endl;
   cout << "Feasibility indicator:\t"
             << cplex.getParam( IloCplex::MIPEmphasis ) << endl;
   cout << "Node_selection\t"
             << cplex.getParam( IloCplex::NodeSel ) << endl;
   cout << "Variable_selection\t"
             << cplex.getParam( IloCplex::VarSel ) << endl;
   cout << "Heuristic_Frequency\t"
             << cplex.getParam( IloCplex::HeurFreq ) << endl;
   cout << "RINS_Heur_Frequency\t"
             << cplex.getParam( IloCplex::RINSHeur ) << endl;
   cout << "LP_Algorithm\t"
             << cplex.getParam( IloCplex::NodeAlg ) << endl;
   cout << "FracCuts\t"
             << cplex.getParam( IloCplex::FracCuts ) << endl;
   cout << "Covers\t"
             << cplex.getParam( IloCplex::Covers ) << endl;
   cout << "-- end of run parameters." << endl << endl;

   cplex.extract( model );

   // print dimensions of the matrix
   cout << "Variables\t" << cplex.getNcols() << endl;
   cout << "Constraints\t" << cplex.getNrows() << endl;
   cout << "NonZeros\t" << cplex.getNNZs() << endl;

   IloBool solution_found = false;
   ClockTimer runtime_timer = ClockTimer();
   runtime_timer.start();
   try {
     solution_found = cplex.solve();
   }
   catch ( IloException & e ) {
     cout << "*** Error during solving ***" << endl;
     cout << e.getMessage();
     runtime_timer.stop();
     cout << "*** elapsed time = " << runtime_timer.getTotalTime() << endl;
     e.end();
     return EXIT_FAILURE;
   }
   runtime_timer.stop();

   IloCplex::CplexStatus solution_status = cplex.getCplexStatus();

   IloBool timed_out
     = (solution_status == IloCplex::AbortTimeLim) ? IloTrue : IloFalse;
   IloBool proved_optimal
     = (solution_status == IloCplex::Optimal
        //        || solution_status == IloCplex::Infeasible
        || solution_status == CPXMIP_INFEASIBLE
        )
    ? IloTrue : IloFalse;

   cout << "runtime \t" << runtime_timer.getTotalTime() << endl;
   cout << "CPXtime \t" << cplex.getTime() << endl;
   cout << "TimedOut\t" << timed_out << endl;
   cout << "SolutionFound\t" << solution_found << endl;
   cout << "ProvedOptimal\t" << proved_optimal << endl;
   cout << "StatusCode\t" << solution_status << endl;
   cout << "num_branches\t" << cplex.getNnodes() << endl;
   if( solution_found ) {
     cout << "value     \t" << cplex.getObjValue() << endl;
   }
   cout << "iterations\t" << cplex.getNiterations() << endl;
   cout << "frac_cuts  \t" << cplex.getNcuts(IloCplex::CutFrac) << endl;
   cout << "clique_cuts\t" << cplex.getNcuts(IloCplex::CutClique) << endl;
   cout << "cover_cuts\t" << cplex.getNcuts(IloCplex::CutCover) << endl;

   if( command_line.flagPresent( "verify" ) && solution_found ) {
     if( solve_as_lp ) { // linear program
       IloNumArray vals( env );
       cplex.getValues( vals, var );
       cout << "Solution" << endl;
       for( int i = 0; i < vals.getSize(); ++i ) {
         cout << "x" << setw( 5 ) << setfill( '0' ) << i
                   << setfill( ' ' ) << "\t" << vals[ i ] << endl;
       }
     }
     else { // integer program
       IloNumArray vals( env );
       cplex.getValues( vals, var );
       cout << "Solution\t";
       for( int i = 0; i < vals.getSize(); ++i ) {
         cout << static_cast< int >( vals[ i ] + 0.5 );
       }
       cout << endl;
     } // end, integer program
   } // end, verify

   if( command_line.flagPresent( "solution" ) && solution_found ) {
     cout << "BeginSolution" << endl;
     for (IloModel::Iterator it(model); it.ok(); ++it) {
       IloExtractable e = *it;
       if ( e.isVariable() ) {
         IloNumVar v = e.asVariable();
         if ( v.getName() ) {
           env.out() << v.getName() << "\t";
           // apparently, CPLEX will output non-integer values for integer
           // variables
           if ( v.getType() == ILOBOOL || v.getType() == ILOINT ) {
             int value = cplex.getValue(v);
             if ( value > 0 ) 
               env.out() << static_cast<int>(cplex.getValue(v) + 0.5);
             else // need to round down for negative values
               env.out() << static_cast<int>(cplex.getValue(v) - 0.5);
           }
           else {
             env.out() << cplex.getValue(v);
           }
           env.out() << endl;
         }
       }
     }
     cout << "EndSolution" << endl;
   }  

   env.end();
   return 0;
}  // END main


static void usage ( const char *progname )
{
   cerr << "Usage: " << progname << " [flags] inputfile" << endl;
   cerr << "   where inputfile is a file in mps or lp format." << endl;
   cerr << "   Flags are 0 or more of the following, in any order:" << endl;
   cerr << "     -cost=<int>        stop when solution has this cost" << endl;
   cerr << "     -UB=<int>          assume an upper bound with this value exists" << endl;
   cerr << "     -time=<int>        time out (number of seconds)" << endl;
   cerr << "     -nodes=<int>       stop after this number of nodes" << endl;
   cerr << "     -sols=<int>        stop after this number of solutions"
        << endl;
   cerr << "     -dep=<int>         extent of checking for row dependence"
        << endl;
   cerr << "        -1 = automatic: let CPLEX choose when to use dependency checking"
        << endl;
   cerr << "         0 = Off : do not use dependency checker (default)"
        << endl;
   cerr << "         1 = turn on only at the beginning of preprocessing"
        << endl;
   cerr << "         2 = turn on only at the end of preprocessing"
        << endl;
   cerr << "         3 = turn on at the beginning and at the end of preprocessing"
        << endl;
   cerr << "     -feasible=<int>    extent to which to emphasize feasibility/optimality"
        << endl;
   cerr << "         0 = Balance optimality and feasibility (default)"
        << endl;
   cerr << "         1 = Emphasize feasibility over optimality"
        << endl;
   cerr << "         2 = Emphasize optimality over feasibility"
        << endl;
   cerr << "         3 = Emphasize moving best bound (work harder on optimality)"
        << endl;
   cerr << "         4 = Emphasize hidden feasibility (work harder on feasibility)"
        << endl;
   cerr << "     -nsel=b/d/e/f      node selection --" 
        << endl;
   cerr << "         b = best bound (default)"
        << endl;
   cerr << "         d = depth-first search"
        << endl;
   cerr << "         e = best estimate"
        << endl;
   cerr << "         f = alternate best estimate"
        << endl;
   cerr << "     -vsel=a/x/n/p/r/s  variable selection --"
        << endl;
   cerr << "         a = automatic (default)"
        << endl;
   cerr << "         x = max infeasibility (cut off lots of branches early)"
        << endl;
   cerr << "         n = min infeasibility (look for feasible solution)"
        << endl;
   cerr << "         p = pseudo costs"
        << endl;
   cerr << "         r = pseudo reduced costs"
        << endl;
   cerr << "         s = `strong' branching"
        << endl;
   cerr << "     -heur=<int>        heuristic frequency (number of nodes between"
        << endl;
   cerr << "                         applications of the heuristic)" << endl;
   cerr << "        -1 = never, 0 = automatic (CPLEX chooses/default)"
        << endl;
   cerr << "     -rins=<int>        frequency of relaxation-induced neighborhood search"
        << endl;
   cerr << "     -lp=a/p/d/b/f      LP algorithm --" 
        << endl;
   cerr << "         a = automatic (default)"
        << endl;
   cerr << "         p = primal simplex"
        << endl;
   cerr << "         d = dual simplex"
        << endl;
   cerr << "         b = barrier method"
        << endl;
   cerr << "         f = sifting"
        << endl;
   cerr << "     -verify            print solution as a string of 0's and 1's (for verification)"
        << endl;
   cerr << "     -solution          print solution with one variable/value pair per line" << endl
        << "                         between lines labeled BeginSolution and EndSolution"
        << endl;
   cerr << "     -frac_cuts         pursue Gomory cuts aggressively" 
        << endl;
   cerr << "     -covers            pursue cover cuts aggressively" 
        << endl;
   cerr << "     -probing=[(-1)-3]  probing level, from none to very aggressive (default 0)"
        << endl;
   cerr << "     -trace=[0-5]       trace level (default = 0, see below)," 
        << endl;
   cerr << "                        each level adds more --" << endl;
   cerr << "         0 = none"
        << endl;
   cerr << "         1 = show integer feasible solutions"
        << endl;
   cerr << "         2 = give a report every 100 nodes"
        << endl;
   cerr << "         3 = show node cuts"
        << endl;
   cerr << "         4 = show LP subproblem information at the root"
        << endl;
   cerr << "         5 = show LP subproblem information at every node"
        << endl;
   cerr << "     -t_freq=<int>       trace frequency (nodes between trace output)"
        << endl;
} // END usage

//  [Last modified: 2019 08 23 at 19:58:51 GMT]
