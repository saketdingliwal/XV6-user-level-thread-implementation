#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
	toggle();	// toggle the system trace on or off
	ps();		// This is part of assignment, which prints the pid and the name of the process
	exit();
}
