# IndoorNavi Firmware for hardware version STM32L4V0.5
## Instalation
1. Generate project files from CubeMX
1. Import project in Eclipse
1. In folder ./Middlewares add New->Folder FolderName:UWB ->Advance LinkedFolder, Browse:"../UWB_core/src"
1. Project->Properties->C/C++ Builde->Settings->Tool Settings->MCU GCC Compiler->Includes Add "${workspace_loc:/${ProjName}/Middlewares/UWB}"
1. Project->Properties->C/C++ Builde->Settings->Build Steps->Pre build Command->"python ../updateHash.py"
1. Project->Properties->C/C++ Builder->Behavior->Enable parallel build
1. Move platform/ folder to the platform specific location
1. Check if project compile with success
1. Implement platform/ folder function