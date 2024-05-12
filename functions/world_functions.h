#ifndef WORLD_FUNC_H
#define WORLD_FUNC_H

size_t calculate_db_size(leveldb::DB* db) {

	size_t totalKeys = 0;
	static char const spin_chars[] = "/-\\|";

	leveldb::ReadOptions readOptions;
	readOptions.decompress_allocator = new leveldb::DecompressAllocator();

	readOptions.verify_checksums = false;
	leveldb::Iterator* itr = db->NewIterator(readOptions);

	for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
		// Update progress
		if (totalKeys % 50000 == 0) {
			std::cout << " " << spin_chars[int(totalKeys / 50000) & 3] << " ";
			std::cout << "\r" << flush;
		}
		// End Update Progress

		totalKeys++;
	}
	delete itr;
	return totalKeys;
}

void parse_world(std::string type, leveldb::DB* db, world_settings srcWorldSettings, world_settings dstWorldSettings, size_t totalKeys, std::vector<std::vector<char>>& dbKeys, std::vector<std::vector<char>>& portalData, std::ofstream& logFile) {

	leveldb::ReadOptions readOptions;
	readOptions.decompress_allocator = new leveldb::DecompressAllocator();

	readOptions.verify_checksums = false;
	leveldb::Iterator* itr = db->NewIterator(readOptions);
	size_t currentKey = 0;
	float lastProgress = 0;
	static char const spin_chars[] = "/-\\|";
	std::map<i_act, std::vector<char>> actorData;
	std::map<i_act, std::vector<char>> digestData;

	i_dim thisDimension;
	i_chnk thisX1;
	i_chnk thisX2;
	i_chnk thisZ1;
	i_chnk thisZ2;
	size_t keysDeleted = 0;
	size_t portalsDeleted = 0;

	for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {

		// Update progress
		float progress = ceil(((float)currentKey / (float)totalKeys) * 100);
		if (progress > lastProgress) {
			if (type == "source") std::cout << " |- Finding Source World data: ";
			if (type == "destination") std::cout << " |- Removing Destination World data: ";
			std::cout << int(progress) << "% [" << currentKey << "/" << totalKeys << "]";
			std::cout << "\r";
			std::cout.flush();
			lastProgress = progress;
		}
		currentKey++;
		// End Update Progress

		if (type == "source") {
			thisDimension = srcWorldSettings.dimension;
			thisX1 = srcWorldSettings.x1;
			thisX2 = srcWorldSettings.x2;
			thisZ1 = srcWorldSettings.z1;
			thisZ2 = srcWorldSettings.z2;
		}
		else {
			thisDimension = dstWorldSettings.dimension;
			thisX1 = dstWorldSettings.x1;
			thisX2 = dstWorldSettings.x2;
			thisZ1 = dstWorldSettings.z1;
			thisZ2 = dstWorldSettings.z2;
		}

		// Get Keys
		leveldb::Slice k = itr->key();
		std::string dbkey = k.ToString();
		if (
			k.starts_with("AutonomousEntities")
			|| k.starts_with("BiomeData")
			|| k.starts_with("Nether")
			|| k.starts_with("Overworld")
			|| k.starts_with("TheEnd")
			|| k.starts_with("scoreboard")
			|| k.starts_with("mobevents")
			|| k.starts_with("schedulerWT")
			|| k.starts_with("LevelChunkMetaDataDictionary")
			|| k.starts_with("VILLAGE_")
			|| k.starts_with("map_")
			|| k.starts_with("~local_player")
			|| k.starts_with("player_")
			|| k.starts_with("tickingarea_")
			|| k.starts_with("portals")
			|| k.starts_with("game_flatworldlayers")
			|| k.starts_with("structuretemplate_")
			) {
			//std::cout << "Ignore Key." << std::endl;
			continue;
		}
		else if (k.starts_with("actorprefix")) {
			// Actor
			i_act actorID = bytestring_to_int(k, 11);
			auto thisKey = std::vector<char>(k.data(), k.data() + k.size());
			actorData[actorID] = thisKey;
		}
		else if (k.starts_with("digp")) {
			// Digest
			// Get Chunk Info
			i_chnk chunkX = get_intval(k, 4);
			i_chnk chunkZ = get_intval(k, 8);
			// Get Dimension
			i_dim dimension = 0;
			if (k.size() > 12) dimension = get_intval(k, 12);

			if (dimension == thisDimension && chunkX >= thisX1 && chunkX <= thisX2 && chunkZ >= thisZ1 && chunkZ <= thisZ2) {
				auto thisKey = std::vector<char>(k.data(), k.data() + k.size());
				dbKeys.push_back(thisKey);
				// Get actor data so we can find actor keys later
				auto v = itr->value();
				auto value = v.ToString();
				int bytes = 8;
				for (int i = 0; i < value.size(); i += bytes) {
					i_act actorID = bytestring_to_int(value, i);
					digestData[actorID] = thisKey;
				}
			}
			continue;
		}
		else if (k.size() == 9 || k.size() == 10 || k.size() == 13 || k.size() == 14) {
			auto thisKey = std::vector<char>(k.data(), k.data() + k.size());
			// Chunk Data
			// Get Chunk Info
			i_chnk chunkX = get_intval(k, 0);
			i_chnk chunkZ = get_intval(k, 4);
			// Get Dimension
			i_dim dimension;
			if (k.size() == 9 || k.size() == 10) dimension = 0;
			else dimension = get_intval(k, 8);

			if (dimension == thisDimension && chunkX >= thisX1 && chunkX <= thisX2 && chunkZ >= thisZ1 && chunkZ <= thisZ2) {
				dbKeys.push_back(thisKey);
			}
			continue;
		}
	}

	int i = 0;
	int actorDataSize = actorData.size();
	float lProgress = 0;
	int matches = 0;

	std::map<i_act, std::vector<char>>::iterator atr;
	for (atr = begin(actorData); atr != end(actorData); ++atr) {

		// Update progress
		float progress = ceil(((float)i / (float)actorDataSize) * 100);
		if (progress > lProgress) {
			if (type == "source") std::cout << " |- Finding entity data: ";
			if (type == "destination") std::cout << " |- Removing entity data: ";
			std::cout << int(progress) << "% [" << i << "/" << actorDataSize << "]";
			std::cout << "\r";
			std::cout.flush();
			lProgress = progress;
		}
		// End Update Progress

		i_act actorID = atr->first;

		std::map<i_act, std::vector<char>>::iterator dtr;
		dtr = digestData.find(actorID);
		if (dtr != digestData.end()) {
			// Match
			auto thisKey = std::vector<char>(atr->second.data(), atr->second.data() + atr->second.size());
			dbKeys.push_back(thisKey);
			matches++;
		}
		i++;
	}

	if (type == "source") {
		rLog(" |- Found " + std::to_string(dbKeys.size() - matches) + " database keys to clone from source world.", logFile);
	}
	if (type == "source") rLog(" |- Found " + std::to_string(matches) + " entity keys to clone.", logFile);


	if (type == "destination") {
		leveldb::WriteBatch batch;
		std::vector<std::vector<char>>::iterator it;
		for (int i = 0; i < dbKeys.size(); i++) {
			std::string dbKey(dbKeys[i].begin(), dbKeys[i].end());
			batch.Delete(dbKey);
			keysDeleted++;
		}
		leveldb::WriteOptions write_options;
		write_options.sync = true;
		leveldb::Status status = db->Write(leveldb::WriteOptions(), &batch);
		if (!status.ok()) {
			rLog("Error processing batch: " + status.ToString(), logFile);
		}
		rLog(" |- Deleted " + std::to_string(keysDeleted - matches) + " chunk data keys from the destination world", logFile);
		rLog(" |- Deleted " + std::to_string(matches) + " entity keys from destination world.", logFile);
	}
	delete itr;

	// Read portals key
	// If changing source dimension to destination dimension then don't copy portal data, just delete from destination
	if (srcWorldSettings.dimension == dstWorldSettings.dimension) {
		if (type == "source") rLog(" |- Finding portal data...", logFile);
		if (type == "destination")  rLog(" |- Removing portal data...", logFile);
		std::string portalKey = "portals";
		std::string portalVal;
		leveldb::Status s = db->Get(leveldb::ReadOptions(), portalKey, &portalVal);
		if (s.ok()) {
			// Portal key exists so get data
			// Get list of portals in this dimension by iterating through NBT data
			int poffset = 0;
			fxNBT output;
			while (poffset < portalVal.size()) {
				poffset = fxnt_getNBT(portalVal, poffset, output);
			}
			//output.print_map();

			int recordsToRemove = 0;
			int workingOffset = 0;

			fxNBT::portalData portals = output.find_portal_data();

			for (int i = 0; i < portals.map.size(); i++) {

				fxNBT::portalData portal = portals.map[i];

				i_dim portalDim = std::stoi(portal.dim);
				i_chnk portalChunkX = std::stoi(portal.x) / 16;
				i_chnk portalChunkZ = std::stoi(portal.z) / 16;

				//std::cout << "Portal D: " << portalDim << " X: " << portal.TpX << " Z: " << portal.TpZ << " CX: " << portalChunkX << " CZ: " << portalChunkZ << std::endl;
				if (portalDim == thisDimension && portalChunkX >= thisX1 && portalChunkX <= thisX2 && portalChunkZ >= thisZ1 && portalChunkZ <= thisZ2) {
					if (type == "source") {
						// Add to data if in dimension and in bounds
						//cout << "D: " << thisDimension << " X: " << thisX1 << " Z: " << thisZ1 << endl;
						auto portalRecord = portalVal.substr(portal.init_offset, portal.init_length + 1);
						auto portalRecordChar = std::vector<char>(portalRecord.data(), portalRecord.data() + portalRecord.size());
						portalData.push_back(portalRecordChar);
					}

					if (type == "destination") {

						recordsToRemove++;
						portalsDeleted++;
						int recordOffset = portal.init_offset - workingOffset;
						int recordLength = portal.init_length + 1;

						portalVal.erase(recordOffset, recordLength);
						// Move the offset to account for change in length
						workingOffset += recordLength;
					}
				}
			}
			if (type == "destination" && recordsToRemove > 0) {

				// Update list
				fxNBT::portalData list = output.find_portal_list();
				int listValue = list.entries;
				int newVal = listValue - recordsToRemove;
				auto newValStr = int_to_bytestring(newVal);
				portalVal.replace(list.val_offset, list.val_length, newValStr);
				/*
				// Check
				int loffset = 0;
				fxNBT loutput;
				while (loffset < portalVal.size()) {
					loffset = fxnt_getNBT(portalVal, loffset, loutput);
				}
				loutput.print_map();
				system("pause");
				*/
				if (portalsDeleted > 0) {
					leveldb::WriteOptions write_options;
					write_options.sync = true;
					leveldb::Status p = db->Put(leveldb::WriteOptions(), portalKey, portalVal);
					if (!p.ok()) {
						rLog("Error processing portal batch: " + p.ToString(), logFile);
					}
					rLog(" |- " + std::to_string(portalsDeleted) + " portal records deleted from destination world.", logFile);
				}
				else {
					rLog(" |- No portal data to remove.", logFile);
				}
			}
			if (type == "source") {
				if (portalData.size() > 0) {
					rLog(" |- Found " + std::to_string(portalData.size()) + " portal records to clone.", logFile);
				}
				else {
					rLog(" |- No portal data to clone.", logFile);
				}
			}
		}
	}

}


void update_coordinates(int offsetX, int offsetZ, std::string& value) {

	int offset = 0;
	fxNBT output;
	while (offset < value.size()) {
		offset = fxnt_getNBT(value, offset, output);
	}
	//output.print_map();

	int mapSize = output.map.size();

	for (int i = 0; i < mapSize; i++) {

		fxNBT item = output.map[i];

		bool found = false;
		int ioffset = item.val_offset;
		int ilength = item.length;
		int newVal;
		std::string ivalue = item.get_val();

		if (item.type == "value" && item.tagname == "pairx") {
			found = true;
			newVal = std::stoi(ivalue) + (offsetX * 16);
			//std::cout << "PairX: " << ivalue << std::endl;
		}
		else if (item.type == "value" && item.tagname == "pairz") {
			found = true;
			newVal = std::stoi(ivalue) + (offsetZ * 16);
			//std::cout << "PairZ: " << ivalue << std::endl;
		}
		else if (item.type == "value" && item.tagname == "x") {
			found = true;
			newVal = std::stoi(ivalue) + (offsetX * 16);
			//std::cout << "X: " << ivalue << std::endl;
		}
		else if (item.type == "value" && item.tagname == "z") {
			found = true;
			newVal = std::stoi(ivalue) + (offsetZ * 16);
			//std::cout << "Z: " << ivalue << std::endl;
		}
		if (found == true) {
			//std::cout << "Len: " << ilength << std::endl;
			//std::cout << "NewVal: " << newVal << std::endl;
			auto newValStr = int_to_bytestring(newVal);
			//std::cout << "NBTVal: " << newValStr << std::endl;
			//int checkX = get_intval(newValStr, 0);
			//std::cout << "CheckVal: " << checkX << std::endl;
			//std::cout << "CheckLen: " << newValStr.size() << std::endl;
			value.replace(ioffset, ilength, newValStr);
		}
	}
	/*
	int checkOffset = 0;
	fxNBT checkOutput;
	while (checkOffset < value.size()) {
		checkOffset = fxnt_getNBT(value, checkOffset, checkOutput);
	}
	checkOutput.print_map();
	system("pause");
	*/

}

#endif