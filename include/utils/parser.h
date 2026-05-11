#ifndef PARSER_H
#define PARSER_H

/*
  PARSER ADT - Data Parsing Module
 
  This module is responsible for converting raw input data
  (typically from stdin or files) into structured system objects.

  It acts as a bridge between external representation (strings)
  and internal ADT structures.
 
  It is strictly separated from business logic to ensure modularity
  and information hiding.
 */

#include "../models/report.h"

/*
 parse_report -> Converts a formatted string into a Report ADT instance.
  Used when loading data from files or structured input
  
  PRECONDITIONS:
  raw_input must be a valid formatted string representing a report
  format must match expected system structure
  POSTCONDITIONS:
  returns a fully initialized Report object
  returned object contains parsed and validated fields
 */
Report parse_report(const char* raw_input);

/*
 parse_int -> Converts string to integer with basic validation
 PRECONDITIONS:
 raw_input must be a valid formatted string representing a report
 format must match expected system structure
 
 POSTCONDITIONS:
 returns a fully initialized Report object
 returned object contains parsed and validated fields
 */
int parse_int(const char* str);

#endif