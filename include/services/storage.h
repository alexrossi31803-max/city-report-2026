#ifndef STORAGE_H
#define STORAGE_H

#include "../adt/list.h"
/*

  STORAGE ADT - Persistence Management Module
 
  This module handles all interactions with persistent storage files.
  It is responsible for loading and saving system data, as well as
  generating derived files used for optimized views.
 */
 /*
 load_users ->Load users from persistent storage
 Preconditions:
 -user data file must exist and be accessible
 Postconditions:
 - user data is loaded into memory structures
 - system user state is initialized for runtime use
 */
void load_users();
/*
 load_reports-> Load reports from persistent storage
 Preconditions: 
 - report master file must exist and be accessible
 - list must be initialized

 Postconditions:
 - all reports are loaded into the ReportList structure
 - in-memory representation is synchronized with file data

 */
void load_reports(ReportList list);
/*
 save_reports->Save reports to persistent storage
 Preconditions:
 -list must contain valid Report nodes
 Postconditions:
 -all reports are written to the master file
 -persistent storage is synchronized with memory state
 */
void save_reports(ReportList list);
/*
 refresh_derived_files->Refresh derived files
 Preconditions:
 - list must contain up-to-date report data
 Postconditions:
 - derived files (filtered/sorted views) are regenerated
 - file views are synchronized with current system state
 */
void refresh_derived_files(ReportList list);

#endif