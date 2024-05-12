

struct fxNBT {
	int key{}, init_offset{}, key_offset{}, val_offset{}, offset{}, parent{}, id{}, length{}, init_length{}, key_length{}, val_length{}, entries{}, depth{};
	std::string tagID{}, type{}, group{}, tagname{}, data_type{};

	std::string str_val{};
	int byte_val{};
	int16_t short_val{};
	int32_t int_val{};
	int64_t long_val{};
	float float_val{};
	std::map<int, fxNBT> map{};

	struct actorData {
		int offset{}, offsetX{}, offsetY{}, offsetZ{}, length{}, lengthX{}, lengthY{}, lengthZ{};
		std::string data_type{}, value{};
		std::string X{}, Y{}, Z{};
	};

	struct tileData {
		int offset{}, offsetX{}, offsetY{}, offsetZ{}, offsetPairX{}, offsetPairZ{}, length{}, lengthX{}, lengthY{}, lengthZ{}, lengthPairX{}, lengthPairZ{};
		std::string data_type{}, value{};
		std::string X{}, Y{}, Z{}, PairX{}, PairZ{};
		std::map<int, tileData> map{};
	};

	struct portalData {
		int entries{}, init_offset{}, key_offset{}, val_offset{}, dim_offset{}, x_offset{}, z_offset{}, init_length{}, key_length{}, val_length{}, dim_length{}, x_length{}, z_length{}, list_end{};
		std::string data_type{}, value{};
		std::string dim{}, x{}, z{};
		std::map<int, portalData> map{};
	};

	std::string get_val() {

		stringstream value;
		if (data_type == "String") value << str_val;
		if (data_type == "Byte") value << byte_val;
		if (data_type == "Short") value << short_val;
		if (data_type == "Int") value << int_val;
		if (data_type == "Long") value << long_val;
		if (data_type == "Float") value << float_val;
		if (data_type == "ByteArray") value << str_val;
		if (data_type == "Other") value << "N/A";
		return value.str();

	}

	int sort_map(std::map<int, std::map<std::string, fxNBT>>& data, int fisrtKey, int lastKey, int depth) {

		std::map<int, fxNBT> upper;
		std::map<int, fxNBT> lower;
		int nextKey = 0;
		int lastI = fisrtKey;
		for (int i = fisrtKey; i < lastKey; i++) {

			// Skip keys that have been processed by reitteration
			if (i < nextKey) continue;

			std::map<std::string, fxNBT> temp;
			fxNBT mapKey = map[i];
			mapKey.depth = depth;

			if (mapKey.key > 0) {
				if (mapKey.type == "value") {
					// Put at end (these are root values)
					lower.insert(std::make_pair(lower.size(), mapKey));
				}
				else {

					// Put compound / list back into system
					temp.insert(std::make_pair(mapKey.group, mapKey));
					data.insert(std::make_pair(data.size(), temp));

					// Reitteration
					// If map is a child compound or value then iterate from offset
					// Return the offset of the next parent. i.e if the parent key is 
					// greater than the last parent key, we're out of the child loop
					// Find length
					int nextSubKey = mapKey.key + 1;
					int lastSubKey = lastKey;
					for (int c = nextSubKey; c < lastSubKey; c++) {

						if (map[c].parent < mapKey.key) {
							lastSubKey = map[c].key;
							break;
						}
					}
					int nextDepth = depth + 1;
					nextKey = sort_map(data, nextSubKey, lastSubKey, nextDepth);
				}
			}
		}

		// Add lower data
		for (int i = 0; i < lower.size(); i++) {
			std::map<std::string, fxNBT> temp;
			temp.insert(std::make_pair("root", lower[i]));
			data.insert(std::make_pair(data.size(), temp));
		}
		return lastKey;

	}
	
	std::map<int, std::map<std::string, fxNBT>> sort() {
		std::map<int, std::map<std::string, fxNBT>> data;
		sort_map(data, 0, map.size(), 0);
		return data;
	}

	void print_map() {

		// Sort Map Data
		std::map<int, std::map<std::string, fxNBT>> data;
		int mapSize = map.size();
		sort_map(data, 0, mapSize, 0);
		int dataSize = data.size();

		for (int i = 0; i < dataSize; i++) {

			std::map<std::string, fxNBT> mapData = data[i];
			std::map<std::string, fxNBT>::iterator dt;

			for (dt = mapData.begin(); dt != mapData.end(); ++dt) {

				std::string group = dt->first;
				fxNBT map = dt->second;

				std::string indent = "";
				for (int d = 0; d < map.depth; d++) {
					indent += "  ";
				}

				std::string value = map.get_val();
				std::cout << indent;
				if (map.type == "Compound") {
					std::cout << "[";
					if (map.tagname != "") std::cout << map.tagname << ": ";
					std::cout << map.entries << " entries";
					std::cout << "]";
					std::cout << " {" << map.init_offset << "," << map.init_length << "}";
					std::cout << " {" << map.key_offset << "," << map.key_length << "}";
				}
				else if (map.type == "List") {
					std::cout << "List: ";
					std::cout << map.tagname << ": " << map.entries << " entries";
					std::cout << " {" << map.init_offset << "," << map.init_length << "}";
					std::cout << " {" << map.key_offset << "," << map.key_length << "}";
					std::cout << " {" << map.val_offset << "," << map.val_length << "}";
				}
				else std::cout << " -> ";
				if (map.type == "value") {
					if (map.tagname != "") std::cout << map.tagname;
					std::cout << " = " << value;
					if (map.type != "") std::cout << " (" << map.data_type << ")";
					std::cout << " {" << map.init_offset << "," << map.init_length << "}";
					//std::cout << " {" << map.key_offset << "," << map.key_length << "}";
					std::cout << " {" << map.val_offset << "," << map.val_length << "}";
				}
				std::cout << std::endl;
			}
		}
	}

	actorData find_val(std::string key) {

		actorData output;
		int mapSize = map.size();
		for (int i = 0; i < mapSize; i++) {

			fxNBT mapKey = map[i];
			if (mapKey.type == "value" && mapKey.tagname == key) {
				output.offset = mapKey.val_offset;
				output.length = mapKey.length;
				output.value = mapKey.get_val();
			}
		}
		return output;

	}

	actorData find_pos() {

		actorData output;
		int mapSize = map.size();
		for (int i = 0; i < mapSize; i++) {

			fxNBT mapKey = map[i];
			if (mapKey.type == "List" && mapKey.tagname == "Pos") {

				// Get children of list
				// Iterate through map again, if parent = this key then is child
				int v = 0;
				for (int x = 0; x < mapSize; x++) {
					fxNBT subKey = map[x];
					if (subKey.type == "value" && subKey.parent == mapKey.key) {
						if (v == 0) {
							output.offsetX = subKey.val_offset;
							output.lengthX = subKey.length;
							output.X = subKey.get_val();
						}
						if (v == 1) {
							output.offsetY = subKey.val_offset;
							output.lengthY = subKey.length;
							output.Y = subKey.get_val();
						}
						if (v == 2) {
							output.offsetZ = subKey.val_offset;
							output.lengthZ = subKey.length;
							output.Z = subKey.get_val();
						}
						v++;
					}
				}
			}
		}
		return output;
	}

	portalData find_portal_list() {
		portalData output;
		int mapSize = map.size();
		int p = 0;
		for (int i = 0; i < mapSize; i++) {

			fxNBT mapKey = map[i];
			if (mapKey.type == "List" && mapKey.tagname == "PortalRecords") {
				output.val_offset = mapKey.val_offset;
				output.val_length = mapKey.val_length;
				output.entries = mapKey.entries;
				output.list_end = mapKey.init_offset + mapKey.init_length;
				//std::cout << "L: " << output.entries << " O: " << output.val_offset << " L: " << output.val_length << " LE: " << output.list_end << std::endl;
			}
		}
		return output;
	}

	portalData find_portal_data() {

		portalData output;
		int mapSize = map.size();
		int p = 0;
		for (int i = 0; i < mapSize; i++) {

			fxNBT mapKey = map[i];
			if (mapKey.type == "Compound" && mapKey.tagname == "PortalRecords") {

				bool valueFound = false;

				// Get children of list
				// Iterate through map again, if parent = this key then is child
				for (int x = 0; x < mapSize; x++) {
					fxNBT subKey = map[x];
					if (subKey.parent == mapKey.key && subKey.type == "value" && subKey.tagname == "DimId") {
						output.map[p].dim_offset = subKey.val_offset;
						output.map[p].dim_length = subKey.length;
						output.map[p].dim = subKey.get_val();
						valueFound = true;
					} 
					else if (subKey.parent == mapKey.key && subKey.type == "value" && subKey.tagname == "TpX") {
						output.map[p].x_offset = subKey.val_offset;
						output.map[p].x_length = subKey.length;
						output.map[p].x = subKey.get_val();
						valueFound = true;
					}
					else if (subKey.parent == mapKey.key && subKey.type == "value" && subKey.tagname == "TpZ") {
						output.map[p].z_offset = subKey.val_offset;
						output.map[p].z_length = subKey.length;
						output.map[p].z = subKey.get_val();
						valueFound = true;
					}
				}

				if (valueFound == true) {
					output.map[p].init_offset = mapKey.init_offset;
					output.map[p].init_length = mapKey.init_length;
					output.map[p].key_offset = mapKey.key_offset;
					output.map[p].key_length = mapKey.key_length;
					output.map[p].val_offset = mapKey.offset;
					output.map[p].val_length = mapKey.length;
					p++;
				}
			}
		}
		return output;
	}


	fxNBT() {};

};
