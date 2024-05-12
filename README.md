MCPE Clone Tool v1.2 x64 (64Bit)
=========================================
Clone chunks between Minecraft Bedrock Edition Worlds


What it does:
=============
This script clones chunks from one world to another.
They can be cloned to a different location in the destination world and even a different dimension.

This is useful if you want to roll back areas of your world from an old backup, or if you want to move builds from one world to another.

Requirements:
=============
This tool is for Minecraft Bedrock Edition and has been tested on 1.18.31 - 1.19 worlds.
Other versions may not work so make a backup first.
This tool is written in C++ and runs on Windows 64bit.


Instructions:
=============
1. Unzip the download to a new folder on your PC. e.g C:\mcbe-clone-tool

Source World:
2. Find the world you want to copy from, from your Minecraft world folders
3. Copy the contents of that world folder to the worlds/source_world. e.g. C:\mcbe-clone-tool\worlds\source_world
-> Make sure you copy the entire contents of the world folder, not just the db folder

Destination World:
4. Find the world you want to copy to, from your Minecraft world folders.
-> If you want to copy to the same world as the source world, then just copy that world into the destination folder.
-> If you don't have a destination world already and want to copy to a blank world, then you need to create a new world using Minecraft first.
5. Copy the contents of that world folder to the worlds/destination_world. e.g. C:\mcbe-clone-tool\worlds\destination_world
-> Make sure you copy the entire contents of the world folder, not just the db folder
BACKUP!!! Make a backup of this folder before you run the clone tool incase there are any issues!

Clone an Area:
6. For each area you wish to clone, run the Clone.exe file and fill in the required data.
-> Source World Dimension: Which dimension is the area in that you want to clone? 0 = Overworld, 1 = Nether, 2 = The End
-> Source World Coordinates: From X, To X, From Z, To Z.
-> Y (height) values are ignored as this will clone the entire chunk including sub chunks.
-> Enter the coordinates as close to chunk boundaries as possible. The system will calculate to the closest chunk, inside the area you've selected.
-> Destination World Dimension: Which dimension is the area in that you want to clone to? 0 = Overworld, 1 = Nether, 2 = The End
-> Destination World Coordinates: From X, From Z
-> Enter the coordinates as close to chunk boundaries as possible. The system will calculate to the closest chunk, inside the area you've selected.
7. Confirm that you want to continue.
-> Y = Yes, N = No, S = Scan Only
-> Scan Only will simulate the clone and give infomation about how much information would be cloned if you had selected Yes.

Do steps 6 & 7 for each area you wish to clone.

Finished:
8. Once you've cloned all of the areas you needed to, copy the destination_world folder back to the Minecraft Worlds folder. (Don't forget to make a backup of the original destination world as this will overwrite your original world data)

