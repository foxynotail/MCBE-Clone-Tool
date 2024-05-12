
#ifndef WORLD_SETUP_H
#define WORLD_SETUP_H

std::string get_world_directory(std::string type, std::ofstream& logFile) {

	std::string Type = type;
	Type[0] = toupper(Type[0]);

	std::string worldDir = "worlds/" + type + "_world";		
	std::string worldDBDir = worldDir + "/db";

	// LOAD LEVEL.DAT AND GET WORLD NAME
	std::string levelFile = worldDir + "/levelname.txt";
	bool worldExists = file_exists(levelFile);
	
	rLog("-> Checking " + type + " world files...", logFile);

	if (worldExists == 1) {

		rLog(" |- " + Type + " World Exists.", logFile);
		std::ifstream infile(levelFile);
		std::string worldName;

		if (getline(infile, worldName)) {
			rLog(" |- World Name: " + worldName, logFile);
		}
		infile.close();

		return worldDBDir;
	}
	else {
		rLog(" |- ERROR: " + Type + " World Data Not Found!", logFile);
		system("pause");
		return 0;
	}
}

struct world_settings {
	int status, lenX, lenZ, offsetX, offsetZ;
	i_dim dimension;
	i_chnk x1, z1, x2, z2;
};

world_settings source_world_settings(std::ofstream& logFile) {

	// GET WORLD DIMENSION
	int status;
	i_dim dimension;
	i_chnk x1, x2, z1, z2, chunkX1, chunkX2, chunkZ1, chunkZ2, newX1, newX2, newZ1, newZ2;
	bool error = false;
	std::cout << "\n#####################################################################################" << std::endl;
	std::cout << "Please enter the dimension you wish to clone from the source world..." << std::endl;
	std::cout << "-> 0 = The Overworld, 1 = The Nether, 2 = The End" << std::endl;
	std::cout << "Source Dimension: ";
	std::cin >> dimension;

	if (!std::cin || dimension < 0 || dimension > 2) error = true;

	// GET WORLD COORDS
	if (error == false) {
		std::cout << "\n#####################################################################################" << std::endl;
		std::cout << "Please enter the coodinates you wish to clone from source world..." << std::endl;
		std::cout << "Source World From X: ";
		cin >> x1;
		if (!std::cin) error = true;
	}
	if (error == false) {
		std::cout << "Source World To X: ";
		std::cin >> x2;
		if (!std::cin) error = true;
	}
	if (error == false) {
		std::cout << "Source World From Z: ";
		std::cin >> z1;
		if (!std::cin) error = true;
	}
	if (error == false) {
		std::cout << "Source World To Z: ";
		std::cin >> z2;
		if (!std::cin) error = true;
	}

	if (error == false) {
		if (x2 < x1) {
			int x3 = x1;
			x1 = x2;
			x2 = x3;
		}
		if (z2 < z1) {
			int z3 = z1;
			z1 = z2;
			z2 = z3;
		}
		// Need to work out what these are in chunks then get the closest coords
		rLog("Clone coordinates [Blocks]: " + std::to_string(x1) + "," + std::to_string(z1) + " to " + std::to_string(x2) + "," + std::to_string(z2), logFile);
		rLog("  |- Calculating chunk coordinates...", logFile);

		chunkX1 = ceil(x1 / 16);
		chunkX2 = ceil(x2 / 16);
		chunkZ1 = ceil(z1 / 16);
		chunkZ2 = ceil(z2 / 16);

		rLog("Chunk coordinates [Chunks]: " + std::to_string(chunkX1) + "," + std::to_string(chunkZ1) + " to " + std::to_string(chunkX2) + "," + std::to_string(chunkZ2), logFile);

		newX1 = chunkX1 * 16;
		newX2 = chunkX2 * 16;
		newZ1 = chunkZ1 * 16;
		newZ2 = chunkZ2 * 16;

		rLog("Cloning from [Blocks]: " + std::to_string(newX1) + "," + std::to_string(newZ1) + " to " + std::to_string(newX2) + "," + std::to_string(newZ2), logFile);

		switch (dimension) {
		case 0: {
			rLog("The Overworld Dimension Selected", logFile);
			break;
		}
		case 1: {
			rLog("The Nether Dimension Selected", logFile);
			break;
		}
		case 2: {
			rLog("The End Dimension Selected", logFile);
			break;
		}
		}
		status = 1;
	}
	if (error == true) {
		rLog("Error: Invalid Choice.", logFile);
		status = 0; dimension = 0; x1 = 0; x2 = 0; z1 = 0; z2 = 0;
	}
	world_settings retval;
	retval.status = status;
	retval.dimension = dimension;
	retval.x1 = chunkX1;
	retval.x2 = chunkX2;
	retval.z1 = chunkZ1;
	retval.z2 = chunkZ2;
	retval.lenX = chunkX2 - chunkX1;
	retval.lenZ = chunkZ2 - chunkZ1;
	return retval;
}
world_settings destination_world_settings(world_settings source, std::ofstream& logFile) {

	// GET WORLD DIMENSION
	int status;
	i_dim dimension;
	i_chnk x1, x2, z1, z2, chunkX1, chunkX2, chunkZ1, chunkZ2, newX1, newX2, newZ1, newZ2;
	bool error = false;
	std::cout << "\n#####################################################################################" << std::endl;
	std::cout << "Please enter the dimension you wish to clone from the destination world..." << std::endl;
	std::cout << "-> 0 = The Overworld, 1 = The Nether, 2 = The End" << std::endl;
	std::cout << "Destination Dimension: ";
	std::cin >> dimension;

	if (!std::cin || dimension < 0 || dimension > 2) error = true;

	// GET WORLD COORDS
	if (error == false) {
		std::cout << "\n#####################################################################################" << std::endl;
		std::cout << "Please enter the starting coodinates you wish to clone from destination to..." << std::endl;
		std::cout << "The end coordinates will be determined by the size of the clone" << std::endl;
		std::cout << "Destination World From X: ";
		std::cin >> x1;
		if (!std::cin) error = true;
	}
	if (error == false) {
		std::cout << "Destination World From Z: ";
		std::cin >> z1;
		if (!std::cin) error = true;
	}
	
	if (error == false) {
		// Need to work out what these are in chunks then get the closest coords

		rLog("Destination Start Coordinates [Blocks]: " + std::to_string(x1) + "," + std::to_string(z1), logFile);
		rLog("  |- Calculating chunk coordinates...", logFile);

		chunkX1 = ceil(x1 / 16);
		chunkZ1 = ceil(z1 / 16);
		chunkX2 = ceil(chunkX1 + source.lenX);
		chunkZ2 = ceil(chunkZ1 + source.lenZ);

		rLog("Cloning to [Chunks]: " + std::to_string(chunkX1) + "," + std::to_string(chunkZ1) + " to " + std::to_string(chunkX2) + "," + std::to_string(chunkZ2), logFile);

		newX1 = chunkX1 * 16;
		newX2 = chunkX2 * 16;
		newZ1 = chunkZ1 * 16;
		newZ2 = chunkZ2 * 16;

		std::string direction;
		rLog("Cloning to [Blocks]: " + std::to_string(newX1) + "," + std::to_string(newZ1) + " to " + std::to_string(newX2) + "," + std::to_string(newZ2), logFile);

		switch (dimension) {
		case 0: {
			rLog("The Overworld Dimension Selected", logFile);
			break;
		}
		case 1: {
			rLog("The Nether Dimension Selected", logFile);
			break;
		}
		case 2: {
			rLog("The End Dimension Selected", logFile);
			break;
		}
		}
		status = 1;
	}
	if (error == true) {
		rLog("Error: Invalid Choice.", logFile);
		status = 0; dimension = 0; x1 = 0; x2 = 0; z1 = 0; z2 = 0;
	}
	world_settings retval;
	retval.status = status;
	retval.dimension = dimension;
	retval.x1 = chunkX1;
	retval.x2 = chunkX2;
	retval.z1 = chunkZ1;
	retval.z2 = chunkZ2;
	retval.offsetX = chunkX1 - source.x1;
	retval.offsetZ = chunkZ1 - source.z1;
	//std::cout << "D: " << retval.dimension << " -> " << retval.x1 << "," << retval.z1 << " - " << retval.x2 << "," << retval.z2 << std::endl;
	return retval;
}
leveldb::Options world_options(std::ofstream& logFile) {

	class NullLogger : public leveldb::Logger {
	public:
		void Logv(const char*, va_list) override {
		}
	};
	/* BEGIN DATABASE OPTIONS */

	rLog("-> Setting LevelDB options...", logFile);

	leveldb::Options options;

	//don't create new database if can't find world
	options.create_if_missing = false;

	//create a bloom filter to quickly tell if a key is in the database or not
	options.filter_policy = leveldb::NewBloomFilterPolicy(10);

	//create a 40 mb cache (we use this on ~1gb devices)
	options.block_cache = leveldb::NewLRUCache(40 * 1024 * 1024);

	//create a 4mb write buffer, to improve compression and touch the disk less
	options.write_buffer_size = 4 * 1024 * 1024;

	//disable internal logging. The default logger will still print out things to a file
	options.info_log = new NullLogger();

	//use the new raw-zip compressor to write (and read)
	options.compressors[0] = new leveldb::ZlibCompressorRaw(-1);

	//also setup the old, slower compressor for backwards compatibility. This will only be used to read old compressed blocks.
	options.compressors[1] = new leveldb::ZlibCompressor();

	//create a reusable memory space for decompression so it allocates less
	return options;
}
#endif