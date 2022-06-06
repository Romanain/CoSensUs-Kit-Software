# CoSensUs IMU sensor
For DFRobot Firebeetle ESP32 version 2

# Known errors
Sometimes the platformio build throws errors when trying to compile the arm subfolder of the EloquentTinyML library (.pio\libdeps\firebeetle32\EloquentTinyML\src\eloquent_tinyml\tensorflow\arm). Since the board is not an Arm but ESP based board you can safely delete this folder and compile without errors. 