#pragma once

/*
* Outputs the message in red
*
* Example:
* print_error("failed to open file")
*
* ERROR: failed to open file
*/
void print_error(const char *message);

/*
* Outputs the message in red
*
* Example:
* print_fatal("failed to open file")
*
* FATAL: failed to open file
*/
void print_fatal(const char *message);

/*
* Outputs the message in orange
*
* Example:
* print_warning("failed to open file")
*
* _WARNING: failed to open file
*/
void print_warning(const char *message);

/*
* Outputs the message in blue
*
* Example:
* print_verbose("Opened file")
*
* Opened file
*/
void print_verbose(const char *message);
