#ifndef LEVELDB_H
#define LEVELDB_H

//#define _CRT_SECURE_NO_WARNINGS

#define LEVELDB_PLATFORM_WINDOWS
#pragma comment(lib,"leveldb.lib")
#define DLLX __declspec( dllimport )

#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <regex>
#include <map>
#include <sstream>
#include <set>
#include <string>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <variant>
#include <vector>
#include <optional>
#include <cassert>
#include <thread>
#include <future>
#include <stdint.h>
#include <string.h>
#include <stdio.h>


#include "leveldb/db.h"
#include "leveldb/zlib_compressor.h"
#include "leveldb/env.h"
#include "leveldb/filter_policy.h"
#include "leveldb/cache.h"
#include "leveldb/decompress_allocator.h"
#include "leveldb/write_batch.h"

typedef int64_t i_act;
typedef int32_t i_chnk;
typedef int32_t i_dim;
typedef int16_t i_tag;
typedef int16_t i_sci;
int max8 = 999;

#include "functions/functions.h"
#include "NBTParser/parse_nbt.h"
#include "FXNTNBT/fxnt_nbt_types.h"
#include "FXNTNBT/fxnt_get_nbt.h"
#include "functions/world_init.h"
#include "functions/world_functions.h"

std::string version = "1.2";

// inDev = true (development) false (production)
bool inDev = false;

#endif