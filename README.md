# aruco-database-converter

Converts the aruco database include files from OpenCV into a single flexible include file.

To use the resulting database.h file, you can define a ARUCO_DB preprocessor macro that has one of:
- ARUCO_DB_ORIGINAL
- ARUCO_DB_4X4_1000
- ARUCO_DB_5X5_1000
- ARUCO_DB_6X6_1000
- ARUCO_DB_7X7_1000
- ARUCO_DB_ARUCO_MIP_36h12
- ARUCO_DB_APRILTAG_16h5
- ARUCO_DB_APRILTAG_25h9
- ARUCO_DB_APRILTAG_36h10
- ARUCO_DB_APRILTAG_36h11

You can also define a ARUCO_DB_SIZE preprocessor macro to restrict the database
to a subset of the total arucos available on the selected database.

For instance, with this code:

```
#define ARUCO_DB        ARUCO_DB_5X5_1000
#define ARUCO_DB_SIZE   25
#include "database.h"
```

the resulting aruco database will be the opencv 5x5 aruco database, but just the first 25 arucos. This may be useful on embedded projects with limited memory and CPU time to match arucos to the database.

It may also help avoid spurious matches with arucos that the project is not really using and shouldn't appear on the image.

# running
Just run "make" to compile, then run with "./aruco_converter > database.h" 

The database.h file is part of the repository, although in theory it shouldn't need to be. This is just a convenience for users that just need the include file and can then just download it and avoid downloading the entire repository, compiling and running the code.
