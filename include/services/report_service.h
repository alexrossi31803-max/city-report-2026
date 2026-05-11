#ifndef REPORT_SERVICE_H
#define REPORT_SERVICE_H

#include "../adt/list.h"
/*
create_new_report -> Create a new report
Preconditions:
-list must be initialized and valid
-user input must be available for report data entry
Postconditions: 
-a new Report is created and inserted into the list
-system state is updated with the new report
*/
void create_new_report(ReportList list);
/*
update_status-> Update report status
Preconditions: 
 - list must be initialized and contain valid reports
 - id must correspond to an existing report
Postconditions: 
 - if report is found, its status is updated
 - otherwise, no changes are applied to the system state
*/
void update_status(ReportList list, int id);
/*
delete_report_service->Delete report service
Preconditions:
 - list must be initialized and contain valid reports
 - id must correspond to an existing report
Postconditions: 
 - if report exists, it is removed from the system and memory is freed
 - if not found, system state remains unchanged
*/
void delete_report_service(ReportList list, int id);

#endif