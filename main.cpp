#include "main.h"

using namespace std;

int main(int argc, char* argv[]) {

	std::ofstream logFile = create_log();

	std::cout << "#################################################################" << std::endl;
	std::cout << "# Clone v" << version << std::endl;
	std::cout << "# Clone parts of Source World to Destination World." << std::endl;
	std::cout << "# For Minecraft Bedrock Edition 1.18.31+" << std::endl;
	std::cout << "# " << std::endl;
	std::cout << "# Instructions & Info in the README.txt file" << std::endl;
	std::cout << "# " << std::endl;
	std::cout << "# Script by FoxyNoTail" << std::endl;
	std::cout << "# foxynotail.com" << std::endl;
	std::cout << "# @foxynotail" << std::endl;
	std::cout << "# (C) 2022, FoxyNoTail" << std::endl;
	std::cout << "############################################################" << std::endl;
	std::cout << std::endl;
	rLog("Clone Program Started.", logFile);

	std::string srcWorld = get_world_directory("source", logFile);
	std::string dstWorld = get_world_directory("destination", logFile);

	world_settings srcSettings = source_world_settings(logFile);
	int srcStatus = srcSettings.status;
	if (srcStatus == 0) {
		system("pause");
		return 1;
	}

	world_settings dstSettings = destination_world_settings(srcSettings, logFile);
	int dstStatus = dstSettings.status;
	if (dstStatus == 0) {
		system("pause");
		return 1;
	}
	i_dim dstDimension = dstSettings.dimension;
	//std::cout << "CHECK SRC: " << srcX1 << " - " << srcX2 << " - " << srcZ1 << " - " << srcZ2 << " - " << srcDimension << endl;
	//std::cout << "CHECK DST: " << dstX1 << " - " << dstX2 << " - " << dstZ1 << " - " << dstZ2 << " - " << dstDimension << endl;

	// CONFIRM
	std::cout << "\n#####################################################################################" << std::endl;
	std::cout << "Are you sure you want to continue using these settings?" << std::endl;
	std::cout << "-> Y = Yes, N = No, S = Scan Only" << std::endl;
	std::cout << "#####################################################################################" << std::endl;
	std::cout << "Confirm: ";
	std::string confirm;
	std::cin >> confirm;

	bool scanOnly = false;

	if (confirm == "Y" || confirm == "y") {
		rLog("Cloning Confirmed...", logFile);
		scanOnly = false;
	}
	else if (confirm == "S" || confirm == "s") {
		rLog("Scan Only  Confirmed...", logFile);
		scanOnly = true;
	}
	else {
		rLog("Process Aborted.", logFile);
		rLog("Done.", logFile);
		system("pause");
		return 1;
	}

	rLog("-> Starting Process...", logFile);

	// Start timer to calculate process time
	clock_t start_s = clock();

	// Setup standard world options
	leveldb::Options options = world_options(logFile);

	// Open Source World DB
	rLog("-> Loading " + srcWorld + "...", logFile);
	leveldb::Status srcDBStatus;
	leveldb::DB* srcDb;

	srcDBStatus = leveldb::DB::Open(options, srcWorld, &srcDb);
	
	if (!srcDBStatus.ok()) {
		rLog("Opening Source World : " + srcWorld + " failed.", logFile);

		system("pause");
		return 1;
	}
	else {
		rLog("-> Source World database opened successfully", logFile);
	}

	// Open Destination World DB
	rLog("-> Loading " + dstWorld + "...", logFile);
	leveldb::Status dstDBStatus;
	leveldb::DB* dstDb;

	dstDBStatus = leveldb::DB::Open(options, dstWorld, &dstDb);

	if (!dstDBStatus.ok()) {
		rLog("Opening Destination World : " + dstWorld + " failed.", logFile);
		system("pause");
		return 1;
	}
	else {
		rLog("-> Destination World database opened successfully", logFile);
	}

	// Calculate database sizes (threaded)
	rLog("-> Calculating Database Sizes.", logFile);
	std::cout << " |- Please wait..." << std::endl;
	size_t srcTotalKeys, dstTotalKeys;
	if (inDev == false) {
		std::future<size_t> srcSize = std::async(&calculate_db_size, srcDb);
		std::future<size_t> dstSize = std::async(&calculate_db_size, dstDb);

		srcTotalKeys = srcSize.get();
		rLog(" |- Source Database has " + std::to_string(srcTotalKeys) + " keys to process.", logFile);
		dstTotalKeys = dstSize.get();
		rLog(" |- Destination Database has " + std::to_string(dstTotalKeys) + " keys to process.", logFile);
	}
	else {
		srcTotalKeys = 5163306;
		dstTotalKeys = 5163306;
	}

	// For the next part:
	// 1. Iterate through source keys looking for data to copy
	// 2. Iterate through destination keys deleting the area where it will go
	std::cout << std::endl;
	rLog("-> Creating list of keys to clone from source world...", logFile);
	std::cout << " |- Please wait..." << std::endl;
	std::vector<std::vector<char>> srcDBKeys;
	std::vector<std::vector<char>> portalData;

	//bool getSrcData = get_src_data(srcDb, srcSettings, dstSettings, srcTotalKeys, srcDBKeys, portalData);
	parse_world("source", srcDb, srcSettings, dstSettings, srcTotalKeys, srcDBKeys, portalData, logFile);
	
	if (scanOnly == false) {

		std::cout << std::endl;
		rLog("-> Removing existing keys from destination world...", logFile);
		std::cout << " |- Please wait..." << std::endl;
		std::vector<std::vector<char>> dstNull;

		//bool getSrcData = erase_dst_data(dstDb, dstSettings, dstTotalKeys, dstKeysDeleted, dstPortalsDeleted);
		parse_world("destination", dstDb, srcSettings, dstSettings, dstTotalKeys, dstNull, dstNull, logFile);
	}

	//system("pause");
	//return 0;
	// Copy DB 1 keys to DB 2
	if (scanOnly == false) {
		std::cout << std::endl;
		rLog("-> Cloning data from source world to destination world...", logFile);
		std::cout << " |- Please wait..." << std::endl;

		// In this section we need to shift the key data to the new coordinates for each key we're moving. 
		// So read the keys, grab the coord/dimension data and rewrite the keys

		leveldb::ReadOptions readOptions;
		readOptions.decompress_allocator = new leveldb::DecompressAllocator();
		readOptions.verify_checksums = false;

		leveldb::WriteOptions write_options;
		write_options.sync = true;
		int found = 0;
		int notfound = 0;
		int currentKey = 0;
		int totalKeys = srcDBKeys.size();
		float lastProgress = 0;

		for (int i = 0; i < totalKeys; i++) {

			// Update progress
			float progress = ceil(((float)currentKey / (float)totalKeys) * 100);
			if (progress > lastProgress) {
				std::cout << " |- Cloning Data: " << int(progress) << "% [" << currentKey << "/" << totalKeys << "]";
				std::cout << "\r";
				std::cout.flush();
				lastProgress = progress;
			}
			currentKey++;
			// End Update Progress

			std::string dbKey(srcDBKeys[i].begin(), srcDBKeys[i].end());
			leveldb::Slice srcDbKey = dbKey;

			std::string keyValue;
			leveldb::Status readKey = srcDb->Get(leveldb::ReadOptions(), srcDbKey, &keyValue);
			if (readKey.ok()) {

				std::string newKey = "";
				leveldb::Slice newSlice;
				std::string prefix = "";

				i_chnk chunkX = 0;
				i_chnk chunkZ = 0;
				i_dim dimension = 0;
				i_tag tagByte = max8;
				i_sci subChunkIndex = max8;
				i_chnk newChunkX = 0;
				i_chnk newChunkZ = 0;
				i_dim newDimension = 0;
				i_tag newTagByte = max8;
				i_sci newSubChunkIndex = max8;
				std::string chunkXStr;
				std::string chunkZStr;
				std::string dimStr;
				std::string start;
				std::string end;

				// Create new key at correct coordinates
				if (srcDbKey.starts_with("actorprefix")) {
					// Don't change actor key as doesn't contain chunk data
					newKey = dbKey;
					//cout << "Actor Key" << std::endl;
					// Need to read actor values and move the entities to the new location including their dimension data
					// Entities have:
					// Pos: x, y, z
					// LastDimensionId: dimid
					int offset = 0;
					fxNBT output;
					while (offset < keyValue.size()) {
						offset = fxnt_getNBT(keyValue, offset, output);
					}					
					//output.print_map();
					
					fxNBT::actorData pos = output.find_pos();					
					//std::cout << "Coords X: " << pos.X << " Y: " << pos.Y << " Z: " << pos.Z << std::endl;
					// Pos is float so calculate new position, convert to float and write data to keyValue
					float newX = std::stof(pos.X) + ((float)dstSettings.offsetX*16);
					auto newXStr = float_to_bytestring(newX);
					//float checkX = get_floatval(newXStr, 0);
					keyValue.replace(pos.offsetX, pos.lengthX, newXStr);

					float newZ = std::stof(pos.Z) + ((float)dstSettings.offsetZ*16);
					auto newZStr = float_to_bytestring(newZ);
					//float checkZ = get_floatval(newZStr, 0);
					keyValue.replace(pos.offsetZ, pos.lengthZ, newZStr);

					// Get dimension
					fxNBT::actorData dim = output.find_val("LastDimensionId");
					//std::cout << "Last Dim: " << dim.value << " {" << dim.offset << "," << dim.length << "}" << std::endl;
					auto newDimStr = int_to_bytestring(dstSettings.dimension);
					//int checkD = get_intval(newDimStr, 0);
					keyValue.replace(dim.offset, dim.length, newDimStr);
					// ALL Working!
					/*
					int checkOffset = 0;
					fxNBT checkOutput;
					while (checkOffset < keyValue.size()) {
						checkOffset = fxnt_getNBT(keyValue, checkOffset, checkOutput);
					}
					checkOutput.print_map();
					fxNBT::actorData cpos = checkOutput.find_pos();
					std::cout << "CCoords X: " << cpos.X << " Y: " << cpos.Y << " Z: " << cpos.Z << std::endl;
					fxNBT::actorData cdim = checkOutput.find_val("LastDimensionId");
					std::cout << "Last Dim: " << cdim.value << " {" << cdim.offset << "," << cdim.length << "}" << std::endl;
					system("pause");
					return 0;
					*/
				}
				else {
					if (srcDbKey.starts_with("digp")) {
						chunkX = get_intval(srcDbKey, 4);
						chunkZ = get_intval(srcDbKey, 8);
						if (srcDbKey.size() > 12) {
							dimension = get_intval(srcDbKey, 12);
						}
						prefix += "digp";
						//cout << "Digest Key" << std::endl;
					}
					else {

						//cout << "Chunk Key" << std::endl;
						chunkX = get_intval(srcDbKey, 0);
						chunkZ = get_intval(srcDbKey, 4);
						if (srcDbKey.size() <= 11) {
							dimension = 0;
							tagByte = srcDbKey[8];
							//tagByte = get_strval(srcDbKey, 8, 1);
							if (srcDbKey.size() == 10) {
								subChunkIndex = srcDbKey[9];
								//subChunkIndex = get_strval(srcDbKey, 9, 1);
							}
						}
						else if (srcDbKey.size() >= 13) {
							dimension = get_intval(srcDbKey, 8);
							tagByte = srcDbKey[12];
							//tagByte = get_strval(srcDbKey, 12, 1);
							if (srcDbKey.size() == 14) {
								subChunkIndex = srcDbKey[13];
								//subChunkIndex = get_strval(srcDbKey, 13, 1);
							}
						}

						// Need to read chunks for tile entities as they contain values for their position and storage
						// Double chests have
						// pairx: x coord
						// pairz: z coord 
						// All chests have
						// x, y, z coords
						// Block entities are in key 49
						if (tagByte == 49) {

							//std::cout << "BlockEntity" << std::endl;
							update_coordinates((int)dstSettings.offsetX, (int)dstSettings.offsetZ, keyValue);
							// Multiple containers are listed within the data
							// In order to get each one, we can't simply find a value as there are multiple
							// So we need to look for a List with x, y or z values inside.
							// The pairx and pairz values are optional and might not exist
						}

						
						// Check other keys for x/Z data
						if (tagByte == 50) {
							// Entity Data Ticks
							//std::cout << "Entity Data" << std::endl;
							update_coordinates((int)dstSettings.offsetX, (int)dstSettings.offsetZ, keyValue);
						}

						if (tagByte == 51) {
							// Pending Ticks
							// Pending ticks can have x / z for block states like observers so need to update those
							//std::cout << "Pending Ticks" << std::endl;
							update_coordinates((int)dstSettings.offsetX, (int)dstSettings.offsetZ, keyValue);
						}

						if (tagByte == 58) {
							// Random Ticks
							//std::cout << "Random Ticks" << std::endl;
							update_coordinates((int)dstSettings.offsetX, (int)dstSettings.offsetZ, keyValue);
						}
						
					}

					start = "[" + std::to_string(chunkX) + "][" + std::to_string(chunkZ) + "]";
					if (dimension > 0) {
						start += "[" + std::to_string(dimension) + "]";
					}
					if (tagByte != max8) {
						start += "[" + std::to_string(tagByte) + "]";
					}
					if (subChunkIndex != max8) {
						start += "[" + std::to_string(subChunkIndex) + "]";
					}

					newChunkX = chunkX + (dstSettings.x1 - srcSettings.x1);
					newChunkZ = chunkZ + (dstSettings.z1 - srcSettings.z1);
					newDimension = dstSettings.dimension;

					int_to_bytes(newChunkX, chunkXStr);
					int_to_bytes(newChunkZ, chunkZStr);

					newKey = chunkXStr + chunkZStr;

					if (newDimension > 0) {
						int_to_bytes(newDimension, dimStr);
						newKey += dimStr;
					}

					if (tagByte != max8) {
						newKey += tagByte;
					}
					if (subChunkIndex != max8) {
						newKey += subChunkIndex;
					}

					newChunkX = get_intval(newKey, 0);
					newChunkZ = get_intval(newKey, 4);

					end = "[" + std::to_string(newChunkX) + "][" + std::to_string(newChunkZ) + "]";
					if (newDimension > 0) {
						newDimension = get_intval(newKey, 8);
						end += "[" + std::to_string(newDimension) + "]";
						if (tagByte != max8) {
							newTagByte = newKey[12];
							end += "[" + std::to_string(newTagByte) + "]";
						}
						if (subChunkIndex != max8) {
							newSubChunkIndex = newKey[13];
							end += "[" + std::to_string(newSubChunkIndex) + "]";
						}
					}
					else {
						if (tagByte != max8) {
							newTagByte = newKey[8];
							end += "[" + std::to_string(newTagByte) + "]";
						}
						if (subChunkIndex != max8) {
							newSubChunkIndex = newKey[9];
							end += "[" + std::to_string(newSubChunkIndex) + "]";
						}
					}

					// Add prefix					
					newKey = prefix + newKey;
					
					if (inDev == true) {
						/*
						if (srcDbKey.size() != newKey.size()) std::cout << "SIZE ERROR: " << srcDbKey.size() << " - " << newKey.size() << endl;
						std::cout << "Start: " << start << std::endl;
						std::cout << "  End: " << end << std::endl;
						if (tagByte != max8) {
							std::string tb = int_to_hex(tagByte);
							std::string ntb = int_to_hex(newTagByte);
							//std::cout << "TB: [" << tb << "] - [" << ntb << "]" << endl;
						}
						std::cout << "Old Key:[" << srcDbKey.ToString() << "]" << std::endl;
						std::cout << "DST Key:[" << newKey << "]" << std::endl;
						std::cout << std::endl;
						*/
					}
					
				}

				if (inDev == false) {
					//std::cout << "Read new key ok!" << std::endl;
					leveldb::Status writeKey = dstDb->Put(leveldb::WriteOptions(), newKey, keyValue);
					if (!writeKey.ok()) {
						cout << "Error writing key. Status: " << writeKey.ToString() << std::endl;
					}
				}
				/*
				leveldb::Status read = srcDb->Get(leveldb::ReadOptions(), newKey, &readVal);
				if (read.ok()) {
					found++;
				}
				else {
					notfound++;

					if (dbKey.size() != newKey.size()) std::cout << "SIZE ERROR: " << dbKey.size() << " - " << newKey.size() << endl;
					std::cout << "Error reading new key: " << read.ToString() << std::endl;
					std::cout << "Start: " << start << std::endl;
					std::cout << "  End: " << end << std::endl;
					if (tagByte != max8) {
						std::string tb = int_to_hex(tagByte);
						std::string ntb = int_to_hex(newTagByte);
						//std::cout << "TB: [" << tb << "] - [" << ntb << "]" << endl;
					}
					std::cout << "Old Key:[" << srcDbKey.ToString() << "]" << std::endl;
					std::cout << "DST Key:[" << newKey << "]" << std::endl;
					std::cout << std::endl;
				}
				*/
			}
			else {
				std::cout << "Error reading key: " << srcDbKey.ToString() << " Status: " << readKey.ToString() << std::endl;
			}
		}
		if (inDev == true) {
			//std::cout << "Found: " << found << std::endl;
			//std::cout << "Not Found: " << notfound << std::endl;
		}
		if (portalData.size() > 0) {
			rLog(" |- Cloning portal data...                                                      ", logFile);
			// Read destination portal data
			leveldb::Slice portalKey = "portals";
			std::string dstPortalData;
			leveldb::Status readPKey = dstDb->Get(leveldb::ReadOptions(), portalKey, &dstPortalData);
			if (readPKey.ok()) {

				int offset = 0;
				fxNBT output;
				while (offset < dstPortalData.size()) {
					offset = fxnt_getNBT(dstPortalData, offset, output);
				}
				//output.print_map();
				// Update portalrecord list entry value to match new number of records
				fxNBT::portalData list = output.find_portal_list();

				// Get position of end of current portal list to append new records
				int listEnd = list.list_end;

				std::string portalRecordsStr = dstPortalData.substr(0, list.list_end);
				for (int i = 0; i < portalData.size(); i++) {
					std::string portalRecord(portalData[i].begin(), portalData[i].end());
					// Combine portal data
					portalRecordsStr += portalRecord;
				}
				portalRecordsStr += dstPortalData.substr(list.list_end);
				dstPortalData = portalRecordsStr;

				int listValue = list.entries;
				int newVal = listValue + portalData.size();
				auto newValStr = int_to_bytestring(newVal);

				std::string newPortalData = dstPortalData.substr(0, list.val_offset);
				newPortalData += newValStr;
				newPortalData += dstPortalData.substr(list.val_offset + list.val_length);
				leveldb::Slice newPortalSlice = newPortalData;

				std::string checkStr = newPortalData.substr(list.val_offset, list.val_length);
				int checkint = get_intval(checkStr, 0);

				// Check
				/*
				int loffset = 0;
				fxNBT loutput;
				while (loffset < newPortalSlice.size()) {
					loffset = fxnt_getNBT(newPortalSlice, loffset, loutput);
				}
				loutput.print_map();
				*/
				leveldb::Status writeKey = dstDb->Put(leveldb::WriteOptions(), "portals", newPortalSlice);
				if (!writeKey.ok()) {
					std::cout << "Error writing portal data. Status: " << writeKey.ToString() << std::endl;
				}
			}
			else {
				// What if destination doesn't have a portal key? Can we make one?
				std::cout << "Destination world does not have a portal key!" << std::endl;
			}
		}
	}

	rLog(" |- Cloning Complete.                                                      ", logFile);


	// ######################################################
	// End Script & Close Database

	rLog("-> Closing Database...", logFile);
	delete srcDb;
	delete dstDb;
	rLog("-> Done.", logFile);

	// TIMER
	// the code you wish to time goes here
	clock_t stop_s = clock();
	float clocktime = (stop_s - start_s) / 1000;

	if (clocktime > 60) {
		clocktime = clocktime / 60;
		std::stringstream ss;
		ss << dec << setprecision(2) << clocktime;
		std::string time = ss.str();
		rLog("\nProcess took " + time + " minutes to complete", logFile);
	}
	else {
		std::stringstream ss;
		ss << dec << setprecision(2) << clocktime;
		std::string time = ss.str();
		rLog("\nProcess took " + time + " seconds to complete", logFile);
	}

	system("pause");	
	return 0;

}